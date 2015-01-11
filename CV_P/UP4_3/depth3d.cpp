#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

const int sizeMax=10,maxDispMax=50,medianMax=10,depthMax=50,angleMax=100;
Mat imageL,imageR,target,inconsistent;
int size=1,maxDisp=15,median=4,depth=0,angle=angleMax/2,anglex=angleMax/2;

double avg(Mat img,int y,int x){
    int anz=0,sum=0;
    for (int x2 = 0; x2 < 2*size+1; ++x2) {
        for (int y2 = 0; y2 < 2*size+1; ++y2) {
            //if pixel is in picture add it
                anz++;
                sum+=img.at<uchar>(y+y2,x+x2);
        }
    }
    //just return average
    return (double)sum/(double)anz;
}

static double diff(Mat img1,Mat img2,int x,int y,int x2,int y2){
    double result=0;
    for (int y3 = -size; y3 <= size; ++y3) {
        for (int x3 = -size; x3 <= size; ++x3) {
            result+=abs(img1.at<uchar>(y+y3,x+x3)-img2.at<uchar>(y2+y3,x2+x3));
        }
    }
    result/=size*size;
    return result;
}

static void nearestNeighbour(){
    for (int y = 0; y < target.rows; ++y) {
        for (int x = 0; x < target.cols; ++x) {
            if (inconsistent.at<uchar>(y,x)==255){
                int x2=1;
                //look for nearest not inconsistent neighbour on same line
                while (true) {
                    //look right
                    if (x+x2<inconsistent.cols&&inconsistent.at<uchar>(y,x+x2)==0){
                        target.at<uchar>(y,x)=target.at<uchar>(y,x+x2);
                        break;
                    }
                    //look left
                    if (x-x2>=0&&inconsistent.at<uchar>(y,x-x2)==0){
                        target.at<uchar>(y,x)=target.at<uchar>(y,x-x2);
                        break;
                    }
                    x2++;
                }
            }
        }
    }
}

/**
 * @brief apply afterEffects
 * @param img
 */
static void afterEffects(int, void*){
    Mat h,result=Mat::zeros(imageL.rows+300,imageL.cols+300,DataType<Vec3b>::type);
    Mat resdisp=Mat::zeros(imageL.rows+300,imageL.cols+300,DataType<double>::type);
    Mat orig = imread("left2.png");
    //remove inconsistent regions
    nearestNeighbour();
    medianBlur(target,h,2*median+1);
    //h = imread("GT.png");
    //cvtColor(h, h, CV_RGB2GRAY);
    //maxDisp=255;
    for (int y = 0; y < orig.rows; ++y) {
        for (int x = 0; x < orig.cols; ++x) {
            //take root of distance so background is not soo far away
            double disp=pow(h.at<uchar>(y,x),1/(1+(double)depth/depthMax));
            double sqMaxDisp=pow(maxDisp,1/(1+(double)depth/depthMax));
            //put pixels with high disparity in foreground
            Mat p = (Mat_<double>(3,1) <<
                      x-orig.cols/2,
                      y-orig.rows/2,
                      100*(double)sqMaxDisp/(disp+1)-300);
            double a=2*M_PI*(double)(angle-angleMax/2)/(angleMax);
            double b=2*M_PI*(double)(anglex-angleMax/2)/(angleMax);
            Mat rotx = (Mat_<double>(3,3) <<
                      1, 0, 0,
                      0, cos(b), -sin(b),
                      0, sin(b),    cos(b));
            Mat roty = (Mat_<double>(3,3) <<
                      cos(a), 0, -sin(a),
                      0, 1, 0,
                      sin(a), 0,    cos(a));
            p=roty*rotx*p;
            p.at<double>(2,0)=(p.at<double>(2,0)+300)/100;
            int x2=(int)(p.at<double>(0,0)/p.at<double>(2,0))+result.cols/2,y2=(int)(p.at<double>(1,0)/p.at<double>(2,0))+result.rows/2;
            if (x2>=0&&y2>=0&&(x2<result.cols)&&y2<result.rows&&(result.at<Vec3b>(y2,x2)==Vec3b(0,0,0)||resdisp.at<double>(y2,x2)>p.at<double>(2,0))){
                result.at<Vec3b>(y2,x2)=orig.at<Vec3b>(y,x);
                resdisp.at<double>(y2,x2)=p.at<double>(2,0);
            }
        }
    }
    imshow("asd",h);
    imshow("Depth",result);
}

static void calcDepth(int, void*){
    target=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    inconsistent=Mat::zeros(imageL.rows,imageL.cols,DataType<uchar>::type);
    Mat diffImg=Mat::zeros(imageL.rows,imageL.cols,DataType<double>::type);
    double minDiff=INFINITY,maxDiff=0;
    for (int y = size; y < target.rows-size; ++y) {
        for (int x = size; x < target.cols-size; ++x) {
            double difference=INFINITY,h;
            int disparity;
            //calculate disparity
            for (int x2 = 0; x2 > -maxDisp&&x+x2>=size; --x2) {
                h=diff(imageL,imageR,x,y,x+x2,y);
                if (h<difference){//&&(difference-h)>10){
                    difference=h;
                    disparity=x2;
                }
            }
            difference=INFINITY;
            //left right consistency
            int disparity2;
            for (int x2 = disparity; x2 <=disparity+maxDisp&&x+x2<target.cols-size; ++x2) {
                h=diff(imageR,imageL,x+disparity,y,x+x2,y);
                if (h<difference){
                    difference=h;
                    disparity2=x2;
                }
            }
            if (difference<minDiff){
                minDiff=difference;
            }
            if (difference>maxDiff){
                maxDiff=difference;
            }

            diffImg.at<double>(y,x)=difference;
            //check whether the pixels match to each other
            if (disparity2>=-3&&disparity2<=3) target.at<uchar>(y,x)=-disparity;
            else {
                target.at<uchar>(y,x)=0;
                inconsistent.at<uchar>(y,x)=255;
            }
        }
    }
    //normalize diff
    for (int y = size; y < target.rows-size; ++y) {
        for (int x = size; x < target.cols-size; ++x) {
            diffImg.at<double>(y,x)=(diffImg.at<double>(y,x)-minDiff)/(maxDiff-minDiff);
        }
    }
    afterEffects(0,0);
    imshow("diff",diffImg);
    imshow("ic",inconsistent);
}

int main()
{
    imageL = imread("left2.png");
    imageR = imread("right2.png");
    cvtColor(imageL, imageL, CV_RGB2GRAY);
    cvtColor(imageR, imageR, CV_RGB2GRAY);
    namedWindow("Depth",1);
    createTrackbar("Box size", "Depth", &size, sizeMax, calcDepth);
    createTrackbar("dpthscale", "Depth", &depth, depthMax, afterEffects);
    createTrackbar("angleY", "Depth", &angle, angleMax, afterEffects);
    createTrackbar("angleX", "Depth", &anglex, angleMax, afterEffects);
    createTrackbar("Median", "Depth", &median, medianMax, afterEffects);
    calcDepth(0,0);
    waitKey(0);

    return 0;

}
