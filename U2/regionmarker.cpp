#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <tree.h>

#include <set>
#include <thread>

using namespace cv;
using namespace std;


const int trackbarmax=80;
int filterSize = 20;
Mat image,ims,median;
//here calculated images get saved so changed the trackbar is smoother
//from 0 to trackbarmax
Mat images[trackbarmax+1];
bool compL(Vec3b a,Vec3b b){
    return a[0]+a[1]+a[2]>b[0]+b[1]+b[2];
}

void calcMedianInRow(int lower,int upper){
    for (int y = lower; y < upper; ++y) {
        Tree* tree=new Tree(2*filterSize+1);
        //fill filter
        for (int x = -filterSize; x <= filterSize; ++x) {
            for (int y2 = -filterSize; y2 <=filterSize; ++y2) {
                tree->insertR(image.at<Vec3b>(Point(x<0?rand()%filterSize:x,
                                                    (y-y2)<0?(rand()%(y+filterSize)):
                                                             (y-y2)>=image.rows?image.rows-1-rand()%(image.rows-1-y+filterSize):(y-y2))));
            }
        }
        median.at<Vec3b>(Point(0,y))=tree->getMedian();
        //move right
        for (int x = 1; x < image.cols; ++x) {
            for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                tree->insertR(image.at<Vec3b>(Point((x+filterSize)>=image.cols?image.cols-1-rand()%(image.cols-1-x+filterSize):x+filterSize,
                                                    (y-y2)<0?(rand()%(y+filterSize)):
                                                             (y-y2)>=image.rows?image.rows-1-rand()%(image.rows-1-y+filterSize):(y-y2))));
            }
            median.at<Vec3b>(Point(x,y))=tree->getMedian();
        }
        delete tree;
    }
}
// define a trackbar callback
static void onTrackbar(int, void*)
{
    if (images[filterSize].empty()){
        clock_t start,finish;
        start=clock();
        //Mat median;
        image.copyTo(median);
        //go filter image
        int rightBorder=image.cols-filterSize-1;
        int bottonBorder=image.rows-filterSize-1;

        //______________________________________________multiset version naiv________________________________________
/*
        bool(*compare)(Vec3b,Vec3b)=compL;
        multiset<Vec3b,bool(*)(Vec3b,Vec3b)> left (compare),right (compare);
        multiset<Vec3b,bool(*)(Vec3b,Vec3b)>::iterator it;
        for (int y = filterSize; y <= bottonBorder; ++y) {
            left.clear();
            //fill filter
            for (int x = 0; x < 2*filterSize+1; ++x) {
                for (int y2 = -filterSize; y2 <=filterSize; ++y2) {
                    left.insert(image.at<Vec3b>(Point(x,y-y2)));
                }
            }
            it=left.begin();
            advance(it,left.size()/2);
            median.at<Vec3b>(Point(filterSize,y))=*it;
            //move right
            for (int x = filterSize+1; x <= rightBorder; ++x) {
                for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                    it=(left.find(image.at<Vec3b>(Point(x-filterSize-1,y+y2))));
                    left.erase(it);
                    left.insert(image.at<Vec3b>(Point(x+filterSize,y-y2)));
                }
                it=left.begin();
                advance(it,left.size()/2);
                median.at<Vec3b>(Point(x,y))=*it;
            }
            imshow("multiset", median);
        }
        //*/
/*
        //_______________________________________slower but working tree version______________________________________
        for (int y = filterSize; y <= bottonBorder; ++y) {
            Tree* tree=new Tree(2*filterSize+1);
            //fill filter
            for (int x = 0; x < 2*filterSize+1; ++x) {
                for (int y2 = -filterSize; y2 <=filterSize; ++y2) {
                    tree->insertR(image.at<Vec3b>(Point(x,y-y2)));
                }
            }
            median.at<Vec3b>(Point(filterSize,y))=tree->getMedian();
            //move right
            for (int x = filterSize+1; x <= rightBorder; ++x) {
                for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                    tree->insertR(image.at<Vec3b>(Point(x+filterSize,y-y2)));
                }
                median.at<Vec3b>(Point(x,y))=tree->getMedian();

            }
        }//*/
/*
        //_______________________________________________tree with rnd border__________________________________________
        for (int y = 0; y < image.rows; ++y) {
            Tree* tree=new Tree(2*filterSize+1);
            //fill filter at letmost point in row
            for (int x = -filterSize; x <= filterSize; ++x) {
                for (int y2 = -filterSize; y2 <=filterSize; ++y2) {
                    //if reqested pixel is outside of the image add another random pixel that is inside the filter and the picture
                    tree->insertR(image.at<Vec3b>(Point(x<0?rand()%filterSize:x,
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
                    tree->insertR(image.at<Vec3b>(Point((x+filterSize)>=image.cols?image.cols-1-rand()%(image.cols-1-x+filterSize):x+filterSize,
                                                        (y-y2)<0?(rand()%(y+filterSize)):
                                                                 (y-y2)>=image.rows?image.rows-1-rand()%(image.rows-1-y+filterSize):(y-y2))));
                }
                //set pixel
                median.at<Vec3b>(Point(x,y))=tree->getMedian();
            }
            //free space
            delete tree;
        }
        //imshow("rnd med", median);
        //*/

        //_____________________________________________tread tree with rnd border______________________________________
        thread t1(calcMedianInRow,0,image.rows/8);
        thread t2(calcMedianInRow,image.rows/8,(image.rows/8)*2);
        thread t3(calcMedianInRow,(image.rows/8)*2,(image.rows/8)*3);
        thread t4(calcMedianInRow,(image.rows/8)*3,(image.rows/8)*4);
        thread t5(calcMedianInRow,(image.rows/8)*4,(image.rows/8)*5);
        thread t6(calcMedianInRow,(image.rows/8)*5,(image.rows/8)*6);
        thread t7(calcMedianInRow,(image.rows/8)*6,(image.rows/8)*7);
        thread t8(calcMedianInRow,(image.rows/8)*7,image.rows);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
        t7.join();
        t8.join();
        //*/
        //_______________________________________faster tree version (not correct)_____________________________________
/*

        //filterWidth is allways odd
        Tree* medianTree=new Tree(2*filterSize+1);
        //fill filter
        for (int x = 0; x < 2*filterSize+1; ++x) {
            for (int y = -filterSize; y <= filterSize; ++y) {
                medianTree->insertR(image.at<Vec3b>(Point(x,filterSize-y)));
            }
        }
        median.at<Vec3b>(Point(filterSize,filterSize))=medianTree->getMedian();

        for (int y = filterSize; y <= bottonBorder; ++y) {
            //move right
            for (int x = filterSize+1; x <= rightBorder; ++x) {
                for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                    medianTree->insertR(image.at<Vec3b>(Point(x+filterSize,y-y2)));
                }
                median.at<Vec3b>(Point(x,y))=medianTree->getMedian();
                //if (x==100) medianTree->printFilter();
            }
            //printf("rightborder\n");
            //medianTree->printFilter();
            y++;
            if (y<bottonBorder){
                //check whether at bottom rows
                //move down
                for (int x2 = -filterSize; x2 <= filterSize; ++x2) {
                    medianTree->insertB(image.at<Vec3b>(Point(rightBorder+x2,y+filterSize)));
                }
                median.at<Vec3b>(Point(rightBorder,y))=medianTree->getMedian();
//medianTree->printFilter();
                //move left
                for (int x = rightBorder-1; x >= filterSize; --x) {
                    for (int y2 = -filterSize; y2 <= filterSize; ++y2) {
                        medianTree->insertL(image.at<Vec3b>(Point(x-filterSize,y+y2)));
                    }
                    median.at<Vec3b>(Point(x,y))=medianTree->getMedian();
                     //if (x==100) medianTree->printFilter();
                }

                if (y+1<bottonBorder){
                    //move down
                    for (int x2 = -filterSize; x2 <= filterSize; ++x2) {
                        medianTree->insertB(image.at<Vec3b>(Point(rightBorder+x2,y+filterSize+1)));
                    }
                    median.at<Vec3b>(Point(filterSize,y+1))=medianTree->getMedian();
                }
            }
        }//*/
/*
        //_______________________________borders naiv multiset__________________________________

        bool(*compare)(Vec3b,Vec3b)=compL;
        multiset<Vec3b,bool(*)(Vec3b,Vec3b)> medianSet (compare);
        multiset<Vec3b,bool(*)(Vec3b,Vec3b)>::iterator it;
        for (int y = 0; y < filterSize; ++y) {
            //_______________top border_________________
            medianSet.clear();
            //fill filter
            for (int x = 0; x <= filterSize; ++x) {
                for (int y2 = -y; y2 <=filterSize; ++y2) {
                    medianSet.insert(image.at<Vec3b>(Point(x,y+y2)));
                }
            }
            it=medianSet.begin();
            advance(it,medianSet.size()/2);
            median.at<Vec3b>(Point(0,y))=*it;
            //move right
            for (int x = 1; x < image.cols; ++x) {
                for (int y2 = -y; y2 <= filterSize; ++y2) {
                    if (x>filterSize) {
                        it=(medianSet.find(image.at<Vec3b>(Point(x-filterSize-1,y+y2))));
                        medianSet.erase(it);
                    }
                    if (x<=rightBorder)
                        medianSet.insert(image.at<Vec3b>(Point(x+filterSize,y+y2)));
                }
                it=medianSet.begin();
                advance(it,medianSet.size()/2);
                median.at<Vec3b>(Point(x,y))=*it;
            }
            //_____________botton border________________
            medianSet.clear();
            //fill filter
            for (int x = 0; x <= filterSize; ++x) {
                for (int y2 = -y; y2 <=filterSize; ++y2) {
                    medianSet.insert(image.at<Vec3b>(Point(x,image.rows-1-y-y2)));
                }
            }
            it=medianSet.begin();
            advance(it,medianSet.size()/2);
            median.at<Vec3b>(Point(0,image.rows-1-y))=*it;
            //move right
            for (int x = 1; x < image.cols; ++x) {
                for (int y2 = -y; y2 <= filterSize; ++y2) {
                    if (x>filterSize) {
                        it=(medianSet.find(image.at<Vec3b>(Point(x-filterSize-1,image.rows-1-y-y2))));
                        medianSet.erase(it);
                    }
                    if (x<=rightBorder)
                        medianSet.insert(image.at<Vec3b>(Point(x+filterSize,image.rows-1-y-y2)));
                }
                it=medianSet.begin();
                advance(it,medianSet.size()/2);
                median.at<Vec3b>(Point(x,image.rows-1-y))=*it;
            }
        }
        for (int x = 0; x < filterSize; ++x) {
            //_______________left border_________________
            medianSet.clear();
            //fill filter
            for (int y = 0; y <= 2*filterSize; ++y) {
                for (int x2 = -x; x2 <=filterSize; ++x2) {
                    medianSet.insert(image.at<Vec3b>(Point(x+x2,y)));
                }
            }
            it=medianSet.begin();
            advance(it,medianSet.size()/2);
            median.at<Vec3b>(Point(x,filterSize))=*it;
            //move right
            for (int y = filterSize+1; y < bottonBorder; ++y) {
                for (int x2 = -x; x2 <= filterSize; ++x2) {
                    it=(medianSet.find(image.at<Vec3b>(Point(x+x2,y-filterSize-1))));
                    medianSet.erase(it);
                    medianSet.insert(image.at<Vec3b>(Point(x+x2,y+filterSize)));
                }
                it=medianSet.begin();
                advance(it,medianSet.size()/2);
                median.at<Vec3b>(Point(x,y))=*it;
            }
            //_____________right border________________
            medianSet.clear();
            //fill filter
            for (int y = 0; y <= 2*filterSize; ++y) {
                for (int x2 = -x; x2 <=filterSize; ++x2) {
                    medianSet.insert(image.at<Vec3b>(Point(image.cols-1-x-x2,y)));
                }
            }
            it=medianSet.begin();
            advance(it,medianSet.size()/2);
            median.at<Vec3b>(Point(image.cols-1-x,filterSize))=*it;
            //move right
            for (int y = filterSize+1; y < bottonBorder; ++y) {
                for (int x2 = -x; x2 <= filterSize; ++x2) {
                    it=(medianSet.find(image.at<Vec3b>(Point(image.cols-1-x-x2,y-filterSize-1))));
                    medianSet.erase(it);
                    medianSet.insert(image.at<Vec3b>(Point(image.cols-1-x-x2,y+filterSize)));
                }
                it=medianSet.begin();
                advance(it,medianSet.size()/2);
                median.at<Vec3b>(Point(image.cols-1-x,y))=*it;
            }

        }
        //*/
        finish=clock();
        printf("%d:%ldms\n",filterSize,(finish-start)*1000/CLOCKS_PER_SEC/8);
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
    //put shot noise on picture
    for (int x = 0; x < image.cols; ++x) {
        for (int y = 0; y < image.rows; ++y) {
           // if (rand()%100<3) image.at<Vec3b>(Point(x,y))=Vec3b(rand()%256,rand()%256,rand()%256);
        }
    }
    //show original of filterwidth = 1
    image.copyTo(images[0]);

    // Create a window
    namedWindow("Filter", 1);

    // create a toolbar
    createTrackbar("Filter Size", "Filter", &filterSize, trackbarmax, onTrackbar);

    //initialize
    onTrackbar(0,0);
    /*for (int var = 0; filterSize < 15; ++var) {
        filterSize++;
        onTrackbar(0,0);
    }//*/
    //show original
    imshow("Filter",image);
    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);
    return 0;

}
