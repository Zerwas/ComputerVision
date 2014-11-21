#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <tree.h>
#include <math.h>

using namespace cv;
using namespace std;

Tree::~Tree(){
    delete[] maxTree;
    delete[] minTree;
    delete[] parents;
    delete[] elements;
}

Tree::Tree(int width):width(width),size(width*width),pos(0)
{
    //printf("%d\n",size);
    //size-1 is allways devisible by 4 (4n²+4n)!!!
    elements=new Vec3b[size];
    parents=new int[size];
    numberOfNodes=(size-1)/2-1;
    numberOfFullNodes=(numberOfNodes-1)/2;
    //initialize trees
    maxTree=new Node[numberOfNodes];
    minTree=new Node[numberOfNodes+1];

    maxTree[0].leftChild=1;
    maxTree[0].rightChild=2;
    maxTree[0].parent=-1;
    //maxTree[0].extrema=0;
    maxTree[1].parent=0;
    maxTree[2].parent=0;

    minTree[0].leftChild=1;
    minTree[0].rightChild=2;
    minTree[0].parent=-1;
    //minTree[0].extrema=size-1;
    minTree[1].parent=0;
    minTree[2].parent=0;
    for(int ebene=1;ebene<size/2;ebene=ebene*2+1){
        for(int i=0;i<=ebene&&i+ebene<numberOfFullNodes;i++){
            //maxTree[ebene+i].extrema=((numberOfNodes+1)/(ebene+1))*i;
            //printf("%d,%d,%d\n",ebene+i,((numberOfNodes+1)/(ebene+1))*i,numberOfNodes+((numberOfNodes+1)/(ebene+1))*i);
            maxTree[ebene+i].leftChild=ebene*2+2*i+1;
            maxTree[ebene*2+2*i+1].parent=ebene+i;
            maxTree[ebene+i].rightChild=ebene*2+2*i+2;
            maxTree[ebene*2+2*i+2].parent=ebene+i;

            //minTree[ebene+i].extrema=numberOfNodes+((numberOfNodes+1)/(ebene+1))*i;

            minTree[ebene+i].leftChild=ebene*2+2*i+1;
            minTree[ebene*2+2*i+1].parent=ebene+i;
            minTree[ebene+i].rightChild=ebene*2+2*i+2;
            minTree[ebene*2+2*i+2].parent=ebene+i;
        }
    }
    //minTree has odd # of leafs
    //minTree[numberOfFullNodes].extrema=numberOfNodes;//nedded?
    minTree[numberOfFullNodes].leftChild=numberOfNodes;
    minTree[numberOfNodes].parent=numberOfFullNodes;

    //fill parent references
    for(int i=0;i<(size-1)/2;i++){
        //printf("%d\n",i);
        parents[i]=numberOfFullNodes+i/2;
        if(i%2==0) maxTree[numberOfFullNodes+i/2].leftChild=i;
        else maxTree[numberOfFullNodes+i/2].rightChild=i;
    }
    for(int i=1;i<(size+1)/2+1;i++){
        //printf("%d\n",(size-1)/2+i-1);
        parents[(size-1)/2+i-1]=numberOfNodes+numberOfFullNodes+i/2;
        if(i%2==0) minTree[numberOfFullNodes+i/2].leftChild=(size-1)/2+i-1;
        else minTree[numberOfFullNodes+i/2].rightChild=(size-1)/2+i-1;
    }

    //fill extrema
    for(int i=numberOfFullNodes;i<numberOfNodes;i++){
        //printf("%d,%d:%d\n",i,(i-numberOfFullNodes)*2,numberOfNodes+1+(i-numberOfFullNodes)*2);
        maxTree[i].extrema=maxTree[i].leftChild;
        Node node=maxTree[i];
        //push extrema to top of tree to create a consistent state
        while (node.parent!=-1) {
            maxTree[node.parent].extrema=maxTree[i].extrema;
            node=maxTree[node.parent];
        }
        minTree[i+1].extrema=minTree[i+1].rightChild;
        node=minTree[i+1];
        while (node.parent!=-1) {
            minTree[node.parent].extrema=minTree[i+1].extrema;
            node=minTree[node.parent];
        }
    }

}

Vec3b Tree::getMedian(){
    //first element is root
    //minTree has one element more then maxTree so minTree minimum is Median
    return elements[minTree[0].extrema];
}

/**
 * @brief Tree::compare the given vectors
 * @param a
 * @param b
 * @return true if a>b
 */
bool Tree::compare(Vec3b a,Vec3b b){
    return a[0]+a[1]+a[2]>b[0]+b[1]+b[2];
}

/**
 * @brief Tree::balance
 * @param position of element being balanced
 * @param Tree in which the element is
 * @param node parent node of the element
 * @param max true if maxTree false if minTree
 */
void Tree::balance(int position, Node *Tree, int node, bool max){
   // printf("balance:%d,(%d,%d:%d)\n",position,Tree[node].leftChild,Tree[node].rightChild,Tree[node].parent);
    r=Tree[node].rightChild;
    if(!max&&node==numberOfFullNodes){
        //former minimum is at node with 1 leaf
        l=Tree[Tree[node].leftChild].extrema;
    }
    else{
        l=Tree[node].leftChild;
    }
    //update parent extrema
    Tree[node].extrema=((max^compare(elements[r],elements[l])))?l:r;
    node=Tree[node].parent;
    while(node!=-1)
    {
        //if(Tree[node].extrema==position)
        h=Tree[node].extrema;
        l=Tree[Tree[node].leftChild].extrema;
        if(!max&&node==numberOfFullNodes){
            //check for node with 1 leaf
            r=Tree[node].rightChild;
        }
        else{
            r=Tree[Tree[node].rightChild].extrema;
        }
        //update extrema
        Tree[node].extrema=((max^compare(elements[r],elements[l])))?l:r;
        //if extrema did not change stop
        if (Tree[node].extrema==h&&Tree[node].extrema!=position)
            break;
        //move up in tree
        node=Tree[node].parent;
    }
}


void Tree::printFilter(){
    Vec3b a=getMedian();
    printf("Filter,Median:%d\n",a[0]+a[1]+a[2]);
    for (int i = width-1; i >=0; --i) {
        for (int j = 0; j < width; ++j) {
            printf("%d",elements[(pos+i+j*width)%size][0]);
            /*a=elements[(pos+i+j*width)%size];
            if (a[0]+a[1]+a[2]<10) printf("%d",0);
            else if (a[0]+a[1]+a[2]<100) printf("%d",1);
            if (a[0]+a[1]+a[2]>=100) printf("%d",2);//*/
        }
        printf(" \n");

    }
}

Vec3b Tree::insertR(Vec3b newEle){
    Vec3b oldEle=elements[pos];
    if (compare(newEle,elements[minTree[0].extrema])){
        //insert new element into minTree

        //printf("1.");
        if(parents[pos]>=numberOfNodes){
            //printf("1\n");

            //old element is in minTree too
            elements[pos]=newEle;
            balance(pos,minTree,parents[pos]-numberOfNodes,false);//care if elem[pos]=extrema
        }
        else{
            //printf("2:%d,%d\n",pos,minTree[0].extrema);

            //old element is in maxTree
            //swap newEle->minTree.extrema->oldEle
            int p=parents[pos];
            h=minTree[0].extrema;
            //bool maxL=!(maxTree[p].rightChild==pos);//change all of these to right child cause off odd #

            //bool minL=!(minTree[parents[minTree[0].extrema]-numberOfNodes].rightChild==minTree[0].extrema);
            //printf("nL:%d\n",maxL);
            if (maxTree[p].rightChild==pos)
                maxTree[p].rightChild=h;
            else
                maxTree[p].leftChild=h;

            if(minTree[parents[h]-numberOfNodes].rightChild==h)
                minTree[parents[h]-numberOfNodes].rightChild=pos;
            else
                minTree[parents[h]-numberOfNodes].leftChild=pos;
            parents[pos]=parents[h];
            parents[h]=p;
            elements[pos]=newEle;
            //balance trees again

            balance(h,maxTree,parents[h],true);
            balance(pos,minTree,parents[pos]-numberOfNodes,false);//all
        }
    }
    else{
        //printf("2.");
        if (compare(newEle,elements[maxTree[0].extrema])){
            //printf("1\n");

            //if new element is between the trees insert it at position of old element
            elements[pos]=newEle;
            if (parents[pos]>=numberOfNodes)
                balance(pos,minTree,parents[pos]-numberOfNodes,false);
            else
                balance(pos,maxTree,parents[pos],true);
        }
        else{
            //printf("2.");
            //insert new element into maxTree
            if(parents[pos]>=numberOfNodes){
                //printf("1\n");
                //old element is in minTree
                //swap newEle->maxTree.extrema->oldEle
                int p=parents[pos]-numberOfNodes;
                h=maxTree[0].extrema;
                //bool maxL=!(minTree[p].rightChild==pos);

                //bool minL=!(maxTree[parents[maxTree[0].extrema]].rightChild==maxTree[0].extrema);

                if (minTree[p].rightChild==pos)
                    minTree[p].rightChild=h;
                else
                    minTree[p].leftChild=h;


                if(maxTree[parents[h]].rightChild==h)
                    maxTree[parents[h]].rightChild=pos;
                else
                    maxTree[parents[h]].leftChild=pos;

                parents[pos]=parents[h];
                parents[h]=p+numberOfNodes;
                elements[pos]=newEle;

                //balance trees again
                balance(h,minTree,parents[h]-numberOfNodes,false);
                balance(pos,maxTree,parents[pos],true);

            }
            else{
                //printf("2\n");
                //old element is in maxTree too
                elements[pos]=newEle;
                balance(pos,maxTree,parents[pos],true);
            }
        }
    }
    pos=(pos+1)%size;
    //printFilter();
    return oldEle;
}

Vec3b Tree::insertL(Vec3b newEle){
    pos=(pos+size-1)%size;//pos--
    Vec3b oldEle=elements[pos];
    insert(newEle);
    //printFilter();
    return oldEle;
}

Vec3b Tree::insertB(Vec3b newEle){//does not work properly
    Vec3b oldEle=elements[pos];
    insert(newEle);
    pos=(pos+width)%size;
    //printFilter();
    return oldEle;
}
/**
 * not in use
 * @brief Tree::insert
 * @param newEle
 */
void Tree::insert(Vec3b newEle){
    //printf("%d\n",minTree[0].extrema);
    if (compare(newEle,elements[minTree[0].extrema])){
        //insert new element into minTree

        //printf("1.");
        if(parents[pos]>=numberOfNodes){
            //printf("1\n");

            //old element is in minTree too
            elements[pos]=newEle;
            balance(pos,minTree,parents[pos]-numberOfNodes,false);//care if elem[pos]=extrema
        }
        else{
            //printf("2:%d,%d\n",pos,minTree[0].extrema);

            //old element is in maxTree
            //swap newEle->minTree.extrema->oldEle
            int p=parents[pos];
            h=minTree[0].extrema;
            //bool maxL=!(maxTree[p].rightChild==pos);//change all of these to right child cause off odd #

            //bool minL=!(minTree[parents[minTree[0].extrema]-numberOfNodes].rightChild==minTree[0].extrema);
            //printf("nL:%d\n",maxL);
            if (maxTree[p].rightChild==pos)
                maxTree[p].rightChild=h;
            else
                maxTree[p].leftChild=h;

            if(minTree[parents[h]-numberOfNodes].rightChild==h)
                minTree[parents[h]-numberOfNodes].rightChild=pos;
            else
                minTree[parents[h]-numberOfNodes].leftChild=pos;
            parents[pos]=parents[h];
            parents[h]=p;
            elements[pos]=newEle;
            //balance trees again

            balance(h,maxTree,parents[h],true);
            balance(pos,minTree,parents[pos]-numberOfNodes,false);//all
        }
    }
    else{
        //printf("2.");
        if (compare(newEle,elements[maxTree[0].extrema])){
            //printf("1\n");

            //if new element is between the trees insert it at position of old element
            elements[pos]=newEle;
            if (parents[pos]>=numberOfNodes)
                balance(pos,minTree,parents[pos]-numberOfNodes,false);
            else
                balance(pos,maxTree,parents[pos],true);
        }
        else{
            //printf("2.");
            //insert new element into maxTree
            if(parents[pos]>=numberOfNodes){
                //printf("1\n");
                //old element is in minTree
                //swap newEle->maxTree.extrema->oldEle
                int p=parents[pos]-numberOfNodes;
                h=maxTree[0].extrema;
                //bool maxL=!(minTree[p].rightChild==pos);

                //bool minL=!(maxTree[parents[maxTree[0].extrema]].rightChild==maxTree[0].extrema);

                if (minTree[p].rightChild==pos)
                    minTree[p].rightChild=h;
                else
                    minTree[p].leftChild=h;


                if(maxTree[parents[h]].rightChild==h)
                    maxTree[parents[h]].rightChild=pos;
                else
                    maxTree[parents[h]].leftChild=pos;

                parents[pos]=parents[h];
                parents[h]=p+numberOfNodes;
                elements[pos]=newEle;

                //balance trees again
                balance(h,minTree,parents[h]-numberOfNodes,false);
                balance(pos,maxTree,parents[pos],true);

            }
            else{
                //printf("2\n");
                //old element is in maxTree too
                elements[pos]=newEle;
                balance(pos,maxTree,parents[pos],true);
            }
        }
    }
    /*printf("minTree: \n");
        for(int ebene=0;ebene<size/2;ebene=ebene*2+1){
            for(int i=0;i<=ebene&&i+ebene<numberOfNodes+1;i++){
                printf("(%d,%d:%d),",minTree[ebene+i].leftChild,minTree[ebene+i].rightChild,minTree[ebene+i].parent);
                printf("%d,",minTree[ebene+i].extrema);
            }
            printf(" \n");
        }
        printf("maxTree \n");
            for(int ebene=0;ebene<size/2;ebene=ebene*2+1){
                for(int i=0;i<=ebene&&i+ebene<numberOfNodes;i++){

                    printf("(%d,%d,%d),",maxTree[ebene+i].leftChild,maxTree[ebene+i].rightChild,maxTree[ebene+i].parent);
                    printf("%d,",maxTree[ebene+i].extrema);
                }
                printf(" \n");
            }
    for(int i=0;i<size;i++)
        printf("%d,",parents[i]);
    printf(" \n");
    for(int i=0;i<size;i++)
        printf("%d,",elements[i][0]);
    printf(":%d",pos);
    printf(" \n");*/
    //pos=(pos+1)%size;
    //return oldEle;
}
