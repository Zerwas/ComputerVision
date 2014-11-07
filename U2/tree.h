#ifndef TREE_H
#define TREE_H

#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using namespace cv;
using namespace std;

struct Node{
    int extrema;
    int parent,leftChild,rightChild;
};

class Tree
{
public:
    Tree(int width);
    Vec3b getMedian();
    //left
    Vec3b insertL(Vec3b newEle);
    //right
    Vec3b insertR(Vec3b newEle);
    //botton
    Vec3b insertB(Vec3b newEle);
    void printFilter();
private:
    void insert(Vec3b newEle);
    bool compare(Vec3b a,Vec3b b);
    void balance(int position,Node* Tree,int node,bool max,bool all=false);
    int width;
    int size;
    //actual colors
    Vec3b* elements;
    int* parents;
    //position of oldest element
    int pos;
    //arrays of nodes of the two trees
    Node* maxTree;
    Node* minTree;
    //number of nodes in maxTree
    int numberOfNodes;
    //number of nodes that don not have children which are leafs in maxTree
    int numberOfFullNodes;
};

#endif // TREE_H
