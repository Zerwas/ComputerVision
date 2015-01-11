#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

const int sizeMax=15,treshMax=100,medianMax=15,maxDisp=15;
Mat imageL,imageR,graph;
Mat diffs[sizeMax+1],targets[sizeMax+1];
int size=1,tresh=70,median=3;
vector<Point> leafs;

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

static bool isLeaf(int x,int y){
    uchar nodeValue=graph.at<uchar>(y,x);
    //check whether all children are solved
    if ((nodeValue>>3&1)&&graph.at<uchar>(y-1,x)>0){
        return false;
    }
    if ((nodeValue>>2&1)&&graph.at<uchar>(y,x-1)>0){
        return false;
    }
    if ((nodeValue>>1&1)&&graph.at<uchar>(y+1,x)>0){
        return false;
    }
    if ((nodeValue&1)&&graph.at<uchar>(y,x+1)>0){
        return false;
    }
    return true;
}

static void solveNode(int x,int y){
    uchar nodeValue=graph.at<uchar>(y,x);
    //set node to solved
    graph.at<uchar>(y,x)=0;
    //____________________magic happens here____________________________

    //__________________________________________________________________
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

/**
 * @brief solveGraph find global optimum for Graph and display resulting depth picture
 */
static void solveGraph(int, void*){
    cout << ((66>>2&1))<<"\n";
    Mat target=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    //calc global optimum
    int n=0;
    while(!leafs.empty()){
        //cout << leafs[0]<<"\n";
        //calc solution for first leaf
        //TODO only returns parents that actually are new leafs
        solveNode(leafs[0].x,leafs[0].y);
        target.at<uchar>(leafs[0])=(n++)*255/(imageL.rows*imageL.cols);
        leafs.erase(leafs.begin());
    }
    //read off solution
    imshow("Depth",target);
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
    //TODO start from 0?,save root?
    /*graph.at<uchar>(size,graph.cols/2)=4+2+1;
    for (int y = size+1; y < imageL.rows-size; ++y) {
        //spine
        graph.at<uchar>(y,graph.cols/2)=128+4+2+1;
        //left leafs
        leafs.push_back(Point(size,y));
        graph.at<uchar>(y,size)=16;
        //right leafs
        leafs.push_back(Point(graph.cols-size-1,y));
        graph.at<uchar>(y,graph.cols-size-1)=64;
    }
    //left arms
    for (int x = graph.cols/2-1; x >size; --x) {
        for (int y = size; y < imageL.rows-size; ++y) {
            graph.at<uchar>(y,x)=16+4;
        }
    }
    //right arms
    for (int x = graph.cols/2+1; x < graph.cols-size; ++x) {
        for (int y = size; y < imageL.rows-size; ++y) {
            graph.at<uchar>(y,x)=64+1;
        }
    }//*/
    //just a test
    //graph.at<uchar>(graph.rows/2,graph.cols/3)+=2;
    //graph.at<uchar>(graph.rows/2+1,graph.cols/3)+=128;

    //___________________________mygraph fully connected but acyclic__________________________________
    /*||||||||||
     *----------
     *||||||||||
     *----------
     */
    //root
    graph.at<uchar>(size,graph.cols/2)=4+2+1;
    //left top
    graph.at<uchar>(size,size)=16+2;
    //right top
    graph.at<uchar>(size,graph.cols-size-1)=64+2;
    //left right borders
    for (int y = size+1; y < graph.rows-size-1; ++y) {
        //spine
        graph.at<uchar>(y,graph.cols/2)=128+4+2+1;
        //left
        graph.at<uchar>(y,size)=128+16+2;
        //right
        graph.at<uchar>(y,graph.cols-size-1)=128+64+2;
    }
    //left side
    for (int x = size+1; x < graph.cols/2; ++x) {
        //top
        graph.at<uchar>(size,x)=16+4+2;
        //body
        for (int y = size+1; y < graph.rows-size-1; ++y) {
            graph.at<uchar>(y,x)=128+16+4+2;
        }
        //bottom
        graph.at<uchar>(graph.rows-size-1,x)=128+16+4;
    }
    //right side
    for (int x = graph.cols/2+1; x < graph.cols-size-1; ++x) {
        //top
        graph.at<uchar>(size,x)=64+2+1;
        //body
        for (int y = size+1; y < graph.rows-size-1; ++y) {
            graph.at<uchar>(y,x)=128+64+2+1;
        }
        //bottom
        graph.at<uchar>(graph.rows-size-1,x)=128+64+1;
    }
    //leaf leafs
    leafs.push_back(Point(size,graph.rows-size-1));
    graph.at<uchar>(graph.rows-size-1,size)=128+16;
    //right leafs
    leafs.push_back(Point(graph.cols-size-1,graph.rows-size-1));
    graph.at<uchar>(graph.rows-size-1,graph.cols-size-1)=128+64;
    //*/
    imshow("Graph",graph);
    solveGraph(0,0);
}

/**
 * @brief preCalc calculate disparities and differenses for all box sizes
 */
static void preCalc(int, void*){
    //calculate differences for each disparity
    for (int i = 0; i <= maxDisp; ++i) {
        Mat diffImg=Mat::zeros(imageL.rows,imageL.cols,DataType<double>::type);
        diffImg.copyTo(diffs[i]);
    }
    for (int y = size; y < imageL.rows-size; ++y) {
        for (int x = size; x < imageL.cols-size; ++x) {
            for (int x2 = 0; x2 >= -maxDisp; --x2) {
                //save difference
                diffs[-x2].at<double>(y,x)=diff(imageL,imageR,x,y,x+x2,y);
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
    createTrackbar("diff", "Depth", &size, sizeMax, preCalc);
    createTrackbar("Median", "Depth", &median, medianMax, solveGraph);
    preCalc(0,0);
    waitKey(0);
    return 0;

}
