#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <tree.h>

using namespace cv;
using namespace std;


const int trackbarmax=80;
int filterSize = 1;
Mat image,median;
//here calculated images are saved so changing the trackbar is smoother
//from 0 to trackbarmax
Mat images[trackbarmax+1];

// define a trackbar callback
static void onTrackbar(int, void*)
{
    if (images[filterSize].empty()){
        clock_t start,finish;
        start=clock();
        //Mat median;
        image.copyTo(median);
        //_______________________________________________tree with rnd border__________________________________________
        for (int y = 0; y < image.rows; ++y) {
            Tree* tree=new Tree(2*filterSize+1);
            //fill filter at letmost point in row
            for (int x = -filterSize; x <= filterSize; ++x) {
                for (int y2 = -filterSize; y2 <=filterSize; ++y2) {
                    //if reqested pixel is outside of the image add another random pixel that is inside the filter and the picture
                    tree->insert(image.at<Vec3b>(Point(x<0?rand()%filterSize:x,
                                                        (y-y2)<0?(rand()%(y+filterSize)):
                                                                 (y-y2)>=image.rows?image.rows-1-rand()%(image.rows-1-y+filterSize):(y-y2))));
                }
            }
            //set lefttmost pixel in row
            median.at<Vec3b>(Point(0,y))=tree->getMedian();
            //move filter right
            for (int x = 1; x < image.cols; ++x) {
                for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                    //same here if pixel not in picture take a random one
                    tree->insert(image.at<Vec3b>(Point((x+filterSize)>=image.cols?image.cols-1-rand()%(image.cols-1-x+filterSize):x+filterSize,
                                                        (y-y2)<0?(rand()%(y+filterSize)):
                                                                 (y-y2)>=image.rows?image.rows-1-rand()%(image.rows-1-y+filterSize):(y-y2))));
                }
                //set pixel
                median.at<Vec3b>(Point(x,y))=tree->getMedian();
            }
            //free space
            delete tree;
        }
        finish=clock();
        printf("%d:%ldms\n",filterSize,(finish-start)*1000/CLOCKS_PER_SEC);
        //save calculated image
        median.copyTo(images[filterSize]);
    }
    //show image
    imshow("Target", images[filterSize]);
}


static void help()
{
    printf("\nThis sample demonstrates Canny edge detection\n"
           "Call:\n"
           "    /.edge [image_name -- Default is cat.jpg]\n\n");
}

const char* keys =
{
    "{1| |cat.jpg|input image name}"
};

int main( int argc, const char** argv )
{
    CommandLineParser parser(argc, argv, keys);
    string filename = parser.get<string>("1");

    image = imread(filename, 1);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", filename.c_str());
        help();
        return -1;
    }
    //show original of filterwidth = 1
    image.copyTo(images[0]);

    // Create a window
    namedWindow("Filter", 1);

    // create a toolbar
    createTrackbar("Filter Size", "Filter", &filterSize, trackbarmax, onTrackbar);

    //initialize
    onTrackbar(0,0);
    //show original
    imshow("Filter",image);
    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);
    return 0;

}
