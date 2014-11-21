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
    ~Tree();
    Vec3b getMedian();
    //right
    void insert(Vec3b newEle);
    void printFilter();
private:
    bool compare(Vec3b a,Vec3b b);
    void balance(int position,Node* Tree, int node, bool max);
    int width;
    int size;
    int l,r,h;
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
