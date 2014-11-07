#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <tree.h>
#include <math.h>

using namespace cv;
using namespace std;


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
    return elements[minTree[0].extrema];
    //return Vec3b(255,255,255);
    return Vec3b(255-elements[minTree[0].extrema][0],255-elements[minTree[0].extrema][1],255-elements[minTree[0].extrema][2]);
}

/**
 * @brief Tree::compare the given vectors
 * @param a
 * @param b
 * @return true if a=>b
 */
bool Tree::compare(Vec3b a,Vec3b b){
    return a[0]+a[1]+a[2]>b[0]+b[1]+b[2];
}

void Tree::balance(int position, Node *Tree, int node, bool max,bool all){
   // printf("balance:%d,(%d,%d:%d)\n",position,Tree[node].leftChild,Tree[node].rightChild,Tree[node].parent);
    if (!all&&position!=maxTree[0].extrema&&position!=minTree[0].extrema){//tree.extrema?
        while(node!=-1)
        {
            if(max^compare(elements[Tree[node].extrema],elements[position])){
                //change minimum in node
                Tree[node].extrema=position;
            }
            else{
                //no change so tree is balanced
                break;
            }
            node=Tree[node].parent;
        }
    }
    else{
        //whole Tree has to be updated because extremum changed
//printf("balElse\n");
        if(!max&&node==numberOfFullNodes){
            //former minimum is at node with 1 leaf
            Tree[node].extrema=(!(max^compare(elements[Tree[Tree[node].leftChild].extrema],elements[Tree[node].rightChild])))?Tree[Tree[node].leftChild].extrema:Tree[node].rightChild;
        }
        else{
            Tree[node].extrema=(!(max^compare(elements[Tree[node].leftChild],elements[Tree[node].rightChild])))?Tree[node].leftChild:Tree[node].rightChild;
        }
        //printf("balElse\n");
        node=Tree[node].parent;
        while(node!=-1)
        {
            if(!max&&node==numberOfFullNodes){
                  Tree[node].extrema=(!(max^compare(elements[Tree[Tree[node].leftChild].extrema],elements[Tree[node].rightChild])))?Tree[Tree[node].leftChild].extrema:Tree[node].rightChild;
            }
            else{
               // printf("Par:%d\n",node);
                if(!(max^compare(elements[Tree[Tree[node].leftChild].extrema],elements[Tree[Tree[node].rightChild].extrema]))){
                    //change minimum in node
                    Tree[node].extrema=Tree[Tree[node].leftChild].extrema;
                }
                else{
                    //no change so tree is balanced
                    Tree[node].extrema=Tree[Tree[node].rightChild].extrema;
                }
            }
            node=Tree[node].parent;
            }

    }
}
void Tree::printFilter(){
    Vec3b a=getMedian();
    printf("Filter,Median:%d\n",a[0]+a[1]+a[2]);
    for (int i = width-1; i >=0; --i) {
        for (int j = 0; j < width; ++j) {
            a=elements[(pos+i+j*width)%size];
            printf("%d,",a[0]+a[1]+a[2]);
        }
        printf(" \n");

    }
}

Vec3b Tree::insertR(Vec3b newEle){
    Vec3b oldEle=elements[pos];
    insert(newEle);
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
            bool maxL=!(maxTree[p].rightChild==pos);//change all of these to right child cause off odd #

            bool minL=!(minTree[parents[minTree[0].extrema]-numberOfNodes].rightChild==minTree[0].extrema);
            //printf("nL:%d\n",maxL);
            if (maxL)
                maxTree[p].leftChild=minTree[0].extrema;
            else
                maxTree[p].rightChild=minTree[0].extrema;

//printf("here\n");
            if(minL)
                minTree[parents[minTree[0].extrema]-numberOfNodes].leftChild=pos;
            else
                minTree[parents[minTree[0].extrema]-numberOfNodes].rightChild=pos;
            //printf("here\n");
            parents[pos]=parents[minTree[0].extrema];
            parents[minTree[0].extrema]=p;
            elements[pos]=newEle;
            //printf("here\n");
            balance(minTree[0].extrema,maxTree,parents[minTree[0].extrema],true);
            balance(pos,minTree,parents[pos]-numberOfNodes,false,true);//all
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
                int p=parents[pos]-numberOfNodes;
                bool maxL=!(minTree[p].rightChild==pos);

                bool minL=!(maxTree[parents[maxTree[0].extrema]].rightChild==maxTree[0].extrema);

                if (maxL)
                    minTree[p].leftChild=maxTree[0].extrema;
                else
                    minTree[p].rightChild=maxTree[0].extrema;


                if(minL)
                    maxTree[parents[maxTree[0].extrema]].leftChild=pos;
                else
                    maxTree[parents[maxTree[0].extrema]].rightChild=pos;

                parents[pos]=parents[maxTree[0].extrema];
                parents[maxTree[0].extrema]=p+numberOfNodes;
                elements[pos]=newEle;

                //int x=parents[pos];
                //parents[pos]=parents[maxTree[0].extrema];
                //parents[maxTree[0].extrema]=x;
                balance(maxTree[0].extrema,minTree,parents[maxTree[0].extrema]-numberOfNodes,false);
                //elements[pos]=newEle;
                balance(pos,maxTree,parents[pos],true,true);

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
