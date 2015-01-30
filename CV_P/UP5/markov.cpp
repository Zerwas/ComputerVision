#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

const int sizeMax=15,treshMax=100,medianMax=15,maxDisp=15,alphaMax=40,funcTypeMax=5;
Mat imageL,imageR,graph,solution,solved,target;
Mat diffs[sizeMax+1],targets[sizeMax+1];
int size=2,tresh=70,median=0,spinePos,alpha=7,funcType=0;
vector<Point> leafs;
vector<pair<Point,int> > roots;

inline bool isLeaf(int x,int y){
    uchar nodeValue=graph.at<uchar>(y,x);
    //check whether all children are solved
    if ((nodeValue>>3&1)&&(!solved.at<bool>(y-1,x))){
        return false;
    }
    if ((nodeValue>>2&1)&&(!solved.at<bool>(y,x-1))){
        return false;
    }
    if ((nodeValue>>1&1)&&(!solved.at<bool>(y+1,x))){
        return false;
    }
    if ((nodeValue&1)&&(!solved.at<bool>(y,x+1))){
        return false;
    }
    return true;
}
inline vector<Point> getChildren(uchar nodeValue,int x,int y){
    vector<Point> children;
    if (nodeValue>>3&1){
        children.push_back(Point(x,y-1));
    }
    if (nodeValue>>2&1){
        children.push_back(Point(x-1,y));
    }
    if (nodeValue>>1&1){
        children.push_back(Point(x,y+1));
    }
    if (nodeValue&1){
        children.push_back(Point(x+1,y));
    }
    return children;
}
inline double g(int k,int kPrime){
    switch (funcType) {
    case 0:
        return ((kPrime==k)?0:1);
    case 1:
        return abs(k-kPrime)/15.;
    case 2:
        return abs(k-kPrime)>3?1:abs(k-kPrime)/4.;
    case 3:
        return (k-kPrime)*(k-kPrime)/225.;
    case 4:
        return abs(k-kPrime)>3?1:(k-kPrime)*(k-kPrime)/16.;
    default:
        return abs(k-kPrime)>0?(abs(k-kPrime)<7?1:0.5):0;
    }

}

inline void solveNode(int x,int y){
    uchar nodeValue=graph.at<uchar>(y,x);
    //set node to solved
    solved.at<bool>(y,x)=true;
    //________________precalc vector of children________________________
    vector<Point> children=getChildren(nodeValue,x,y);
    //number of possible configurations for children
    int total=1;
    for (int i = 0; i < children.size(); ++i) {
        total*=16;
    }
    //___________________the magic happens here__________________________
    double best,h;
    int kPrime;
    short kI,i;
    for (short k = 0; k < 16; ++k) {
        best=INFINITY;
        for (kPrime = 0; kPrime < total; ++kPrime) {
            h=0;
            for (i = 0; i < children.size(); ++i) {
                //g(k,k'_i)+solution(k'_i);
                kI=(kPrime>>(4*i))&15;
                h+=(alpha*0.01)*g(k,kI)+diffs[kI].at<double>(children[i]);
            }
            if (h<best){
                best=h;
                solution.at<int>(y,x)=kPrime;
            }
        }
        //(+=) add g(k)
        diffs[k].at<double>(y,x)+=best;
    }
    //___________________add new leafs to queue_________________________
    //find parents that become new leafs and put them in queue
    if ((nodeValue>>7&1)&&isLeaf(x,y-1)){
        leafs.push_back(Point(x,y-1));
    }
    if ((nodeValue>>6&1)&&isLeaf(x-1,y)){
        leafs.push_back(Point(x-1,y));
    }
    if ((nodeValue>>5&1)&&isLeaf(x,y+1)){
        leafs.push_back(Point(x,y+1));
    }
    if ((nodeValue>>4&1)&&isLeaf(x+1,y)){
        leafs.push_back(Point(x+1,y));
    }
}

inline void readOff(int x,int y,int k){
    //draw solution
    target.at<uchar>(y,x)=k*254/maxDisp+1;
    uchar nodeValue=graph.at<uchar>(y,x);
    vector<Point> children=getChildren(nodeValue,x,y);
    for (int i = 0; i < children.size(); ++i) {
        if (target.at<uchar>(children[i])==0)
            roots.push_back(pair<Point,int>(children[i],(solution.at<int>(y,x)>>(4*i))&15));
    }
}

static void afterEffects(int, void*){
    Mat h;
    medianBlur(target,h,median*2+1);
    imshow("Depth",h);
}

/**
 * @brief solveGraph find global optimum for Graph and display resulting depth picture
 */
static void solveGraph(int, void*){
    Mat graphView=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    solution=Mat::zeros(imageL.rows,imageL.cols,DataType<int>::type);
    solved=Mat::zeros(graph.rows,graph.cols,DataType<bool>::type);
    for (int x = 0; x < graph.cols; ++x) {
        for (int y = 0; y < graph.rows; ++y) {
            if (graph.at<uchar>(y,x)==0)
                solved.at<bool>(y,x)=true;
        }
    }
    //_________________________________calc global optimum__________________________
    int n=0;
    while(!leafs.empty()){
        //calc solution for first leaf
        solveNode(leafs[0].x,leafs[0].y);
        graphView.at<uchar>(leafs[0])=(n++)*255/(imageL.rows*imageL.cols);
        leafs.erase(leafs.begin());
    }
    //_________________________________read off solution____________________________
    target=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    //calc best solution for each root
    for (int i = 0; i < roots.size(); ++i) {
        int best=INFINITY;
        for (int k = 0; k < 16; ++k) {
            if (diffs[k].at<double>(roots[i].first)<best){
                best=diffs[k].at<double>(roots[i].first);
                roots[i].second=k;
            }
        }
    }
    //save solution in target
    while (!roots.empty()) {
        readOff(roots[0].first.x,roots[0].first.y,roots[0].second);
        roots.erase(roots.begin());
    }
    imshow("graph",graphView);
    afterEffects(0,0);
}

/**
 * @brief createGraph for markov chains
 */
static void createGraph(int, void*){
    //graph code:(parent up,child up....) pu|pl|pb|pr|cu|cl|cb|cr    0 means node is solved
    graph=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    //___________________________________graph from task______________________________________________
    /*    |
     *----------
     *    |
     *----------
     */
    roots.push_back(pair<Point,int>(Point(spinePos,size),0));
    graph.at<uchar>(size,spinePos)=4+2+1;
    for (int y = size+1; y < imageL.rows-size; ++y) {
        //spine
        graph.at<uchar>(y,spinePos)=128+4+2+1;
        //left leafs
        leafs.push_back(Point(size,y));
        graph.at<uchar>(y,size)=16;
        //right leafs
        leafs.push_back(Point(graph.cols-size-1,y));
        graph.at<uchar>(y,graph.cols-size-1)=64;
    }
    //left arms
    for (int x = spinePos-1; x >size; --x) {
        for (int y = size; y < imageL.rows-size; ++y) {
            graph.at<uchar>(y,x)=16+4;
        }
    }
    //right arms
    for (int x = spinePos+1; x < graph.cols-size; ++x) {
        for (int y = size; y < imageL.rows-size; ++y) {
            graph.at<uchar>(y,x)=64+1;
        }
    }
    imshow("Graph",graph);
    solveGraph(0,0);
}

static double diff(Mat img1,Mat img2,int x,int y,int x2,int y2){
    double result=0;
    for (int y3 = -size; y3 <= size; ++y3) {
        for (int x3 = -size; x3 <= size; ++x3) {
            result+=abs(img1.at<uchar>(y+y3,x+x3)-img2.at<uchar>(y2+y3,x2+x3));
        }
    }
    result/=(2*size+1)*(2*size+1);
    return result;
}

/**
 * @brief preCalc calculate disparities and differenses for all box sizes
 */
static void preCalc(int, void*){
    double diffMin = INFINITY,diffMax=-INFINITY,h;
    //calculate differences for each disparity
    for (int i = 0; i <= maxDisp; ++i) {
        Mat diffImg=Mat::zeros(imageL.rows,imageL.cols,DataType<double>::type);
        diffImg.copyTo(diffs[i]);
    }
    for (int y = size; y < imageL.rows-size; ++y) {
        for (int x = size; x < imageL.cols-size; ++x) {
            for (int x2 = 0; x2 >= -maxDisp; --x2) {
                //save difference
                h=diff(imageL,imageR,x,y,x+x2,y);
                diffMin=min(diffMin,h);
                diffMax=max(diffMax,h);
                diffs[-x2].at<double>(y,x)=h;
            }
        }
    }

    //normalize differences
    for (int y = size; y < imageL.rows-size; ++y) {
        for (int x = size; x < imageL.cols-size; ++x) {
            for (int x2 = 0; x2 >= -maxDisp; --x2) {
                diffs[-x2].at<double>(y,x)=(diffs[-x2].at<double>(y,x)-diffMin)/diffMax;
            }
        }
    }

    createGraph(0,0);
}

int main()
{
    imageL = imread("left2.png");
    imageR = imread("right2.png");
    cvtColor(imageL, imageL, CV_RGB2GRAY);
    cvtColor(imageR, imageR, CV_RGB2GRAY);
    namedWindow("Depth",1);
    createTrackbar("Boxsize", "Depth", &size, sizeMax, preCalc);
    createTrackbar("Median", "Depth", &median, medianMax, afterEffects);
    spinePos=imageL.cols/2;
    createTrackbar("Spine Pos", "Depth", &spinePos, imageL.cols-1, preCalc);
    createTrackbar("alpha", "Depth", &alpha, alphaMax, preCalc);
    createTrackbar("weigthfunctionType", "Depth", &funcType, funcTypeMax, preCalc);
    preCalc(0,0);
    waitKey(0);
    return 0;

}
