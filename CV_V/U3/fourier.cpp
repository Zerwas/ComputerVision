#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace cv;
using namespace std;


const int dMax=200,coeffMax=200,numberOfPoints=1024;
int d = 94,maxd,tresh,coeff=5;
Vec3b wantedColor;
Mat image;
vector<Point> contour;
vector < pair<double, double> > xCoefficients,yCoefficients;

/**
 * @brief distance between a and b
 * @param a
 * @param b
 * @return
 */
static double dst(Point a, Point b){
    return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

/**
 * @brief fourierFunction
 * @param x
 * @param coefficients
 * @param perimeter
 * @return f(x;coefficients,perimeter)
 */
static double fourierFunction(double x,vector< pair<double, double> > coefficients,double perimeter){
    double result=coefficients[0].first/2;
    for (int n = 1; n <= coeff; ++n) {
        result+=coefficients[n].first*cos((n*M_PI*x*2)/perimeter)+coefficients[n].second*sin((n*M_PI*x*2)/perimeter);
    }
    return result;
}

/**
 * @brief drawLines draws lines between points given in vector
 * @param img
 * @param points
 * @param color
 */
static void drawLines(Mat img,vector<Point> points,uchar color){
    vector<Point>::iterator it;
    for (it=points.begin(); it<points.end()-1; it++) {
        line(img,*it,*(it+1),color);
    }
}

static void fourierTransform(int,void*){
    Mat target;
    image.copyTo(target);
    double perimeter=0;
    //calculate perimeter of contour,last point in contour is equal to the first point in contour
    for (uint i = 0; i < contour.size()-1; ++i) {
        perimeter+=dst(contour[i],contour[i+1]);
    }
    //_______________________calculate points (x(s),y(s))______________________________
    vector < pair<double, double> > fContour;
    double s=0,s_prev,index=0,s_i;
    for (int i = 0; i < numberOfPoints; ++i) {
        //move along contour
        s_i=i*perimeter/numberOfPoints;
        while (s_i>=s){
            s_prev=s;
            s+=dst(contour[index],contour[index+1]);
            index++;
        }
        //s_i is now between index-th and index+1-th point (may be equal to index-th point but not index+1)
        fContour.push_back(pair<double, double> (contour[index].x+(contour[index-1].x-contour[index].x)*(s-s_i)/(s-s_prev),contour[index].y+(contour[index-1].y-contour[index].y)*(s-s_i)/(s-s_prev)));
        //target.at<Vec3b>(Point((int)(contour[index].x+(contour[index-1].x-contour[index].x)*(s-s_i)/(s-s_prev)),(int)fContour[i].second))={255,255,0};
    }
    //imshow("Points",target);
    //__________________________________________calculate fourier coefficients______________________________________________
    //xCoefficients : fourier function for s->x
    //yCoefficients : fourier function for s->y
    double a,b;
    for (int n = xCoefficients.size(); n <= coeff; ++n) {
        //calculate x coefficients
        a=0;b=0;
        //approximate integral
        for (int x = 0; x < numberOfPoints; ++x) {
            s_i=x*perimeter/numberOfPoints;
            a+=fContour[x].second*cos((n*M_PI*s_i*2.)/perimeter);
            b+=fContour[x].second*sin((n*M_PI*s_i*2.)/perimeter);
        }
        a=(a*2)/numberOfPoints;
        b=(b*2)/numberOfPoints;
        xCoefficients.push_back(pair<double, double> (a,b));

        //calculate y coefficients
        a=0;b=0;
        //approximate integral
        for (int x = 0; x < numberOfPoints; ++x) {
            s_i=x*perimeter/numberOfPoints;
            a+=fContour[x].first*cos((n*M_PI*s_i*2.)/perimeter);
            b+=fContour[x].first*sin((n*M_PI*s_i*2.)/perimeter);
        }
        a=(a*2)/numberOfPoints;
        b=(b*2)/numberOfPoints;
        yCoefficients.push_back(pair<double, double> (a,b));
    }

    /*for (int i = 0; i < xCoefficients.size(); ++i) {
        cout << xCoefficients[i].first << "," << xCoefficients[i].second << "\n";
    }*/
    //__________________________________________________draw function_____________________________________________
    image.copyTo(target);
    vector<Point> points;
    //draw fourier shape
    points.clear();
    for (int x = 0; x < perimeter; ++x) {
        points.push_back(Point((int)fourierFunction(x,yCoefficients,perimeter),(int)fourierFunction(x,xCoefficients,perimeter)));
    }
    drawLines(target,points,255);
    imshow("Fourier",target);
}

/**
 * @brief onTrackbar find contour
 */
static void onTrackbar(int, void*)
{
    Mat marked;
    image.copyTo(marked);
    //*d/trackbarmax für exponentiel scaling, not linear
    tresh=((((d*maxd/dMax)*d)/dMax)*d)/dMax;
    tresh=tresh-wantedColor[0]*wantedColor[0]-wantedColor[1]*wantedColor[1]-wantedColor[2]*wantedColor[2];
    Vec3b b,w,p;
    //black
    b[0]=0;
    b[1]=0;
    b[2]=0;
    //white
    w[0]=255;
    w[1]=255;
    w[2]=255;
    int j;
    //iterate over every pixel
    for (int i = 0; i < image.cols; ++i) {
        for (j = 0; j < image.rows; ++j) {
            p=image.at<Vec3b>(Point(i,j));
            //check whether given color is in acceptable distance to wanted color
            //paint pixel black or white
            marked.at<Vec3b>(Point(i,j))=((-2*wantedColor[0]*p[0])+(p[0]*p[0])+(-2*wantedColor[1]*p[1])+(p[1]*p[1])+(-2*wantedColor[2]*p[2])+(p[2]*p[2])<=tresh)?w:b;
        }
    }
    Mat contourImg;
    cvtColor(marked, contourImg, CV_BGR2GRAY);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    // Find contours
    findContours(contourImg,contours,hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    int maxSize=0,maxIndex=-1,area,areaDepth;
    //find largest white region
    for (uint i = 0; i < contours.size(); ++i) {
        //calculate area of the contour
        area=contourArea(contours[i])+contours[i].size();
        j=hierarchy[i][2];
        //subtract area of child contours
        while(j>=0){
                area-=contourArea(contours[j])-contours[j].size();
                j=hierarchy[j][0];

        }
        //calculate depth of contour so no black regions are painted red
        areaDepth=0;
        j=hierarchy[i][3];
        while(j>=0){
                areaDepth++;
                j=hierarchy[j][3];

        }
        //if (area>=0) printf("%d,%d,this=%d,next=%d,child=%d\n",area,areaDepth,i,hierarchy[i][1],hierarchy[i][2]);
        //check whether currently investigated area is bigger than the largest so far
        if (area>=maxSize&&areaDepth%2==0){//&&marked.at<Vec3b>(contours[i][0])==w){
            maxSize=area;
            maxIndex=i;
        }
    }

    //red
    Scalar color(0,0,255);
    //fill largest white region
    if (maxIndex>=0)
        drawContours(marked,contours,maxIndex, color,
                 CV_FILLED, 8);

    //set contour
    contour=contours[maxIndex];
    contour.insert(contour.end(),contour[0]);
    //clear fourier function
    xCoefficients.clear();
    yCoefficients.clear();
    //save calculated image
    //call fT
    fourierTransform(0,0);
    //show image
    imshow("Target", marked);
}

static void setWantedColor(int event, int x, int y, int , void* ){
    if  ( event == EVENT_LBUTTONDOWN ){

        //set wanted color
        wantedColor=image.at<Vec3b>(Point(x,y));
        maxd=0;
        int h;
        //find out maximal distance to wanted color
        for (int i = 0; i < image.cols; ++i) {
            for (int j = 0; j < image.rows; ++j) {
                Vec3b p=image.at<Vec3b>(Point(i,j));
                //calculate distance to pixel p
                h=(wantedColor[0]-p[0])*(wantedColor[0]-p[0])+(wantedColor[1]-p[1])*(wantedColor[1]-p[1])+(wantedColor[2]-p[2])*(wantedColor[2]-p[2]);
                maxd=maxd<h?h:maxd;
             }
        }
        //show image with new wanted color
        onTrackbar(0, 0);
    }

}


int main()
{

    image = imread("Wirbel.png", 1);

    // Create a window
    namedWindow("Fourier", 1);

    // create a toolbar
    createTrackbar("Distance", "Fourier", &d, dMax, onTrackbar);
    createTrackbar("Coefficients", "Fourier", &coeff, coeffMax, fourierTransform);
    //mouse click event
    setMouseCallback("Fourier", setWantedColor, NULL);

    //show original
    imshow("Fourier",image);

    //initialise
    setWantedColor(EVENT_LBUTTONDOWN,195,311,0,NULL);
    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);

    return 0;

}
