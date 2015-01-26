#include <iostream>
using namespace std;
#include "ps.h"


typedef struct MP{
    //index of other point
    int l;
    int r;
    double v;
    MP(int le,int ri,double value){
        l=le;
        r=ri;
        v=value;
    }
    MP(){
        l=-1;
    }
    MP copy(){
        return MP(l,r,v);
    }
} MATCHPOINT;

/**
 * @brief comp
 * @param a
 * @param b
 * @return a>b?
 */
bool comp(const MATCHPOINT& a, const MATCHPOINT& b) {
    return a.v<b.v;
}

inline double diff(int heightl, int widthl, unsigned char *imgl, int il, int jl,
               int heightr, int widthr, unsigned char *imgr, int ir, int jr,
               int wsize) {
    if (jl<jr) return numeric_limits<double>::max();
    //if (xL-xR>imgL.cols/2) return numeric_limits<double>::max();

    // filter out points that are close to borders
    if( il-wsize<0 ) return numeric_limits<double>::max();
    if( ir-wsize<0 ) return numeric_limits<double>::max();
    if( il+wsize>heightl-1 ) return numeric_limits<double>::max();
    if( ir+wsize>heightr-1 ) return numeric_limits<double>::max();
    if( jl-wsize<0 ) return numeric_limits<double>::max();
    if( jr-wsize<0 ) return numeric_limits<double>::max();
    if( jl+wsize>widthl-1 ) return numeric_limits<double>::max();
    if( jr+wsize>widthr-1 ) return numeric_limits<double>::max();

    double q=0.;
    for( int di=-wsize; di<=wsize; di++ ) {
        for( int dj=-wsize; dj<=wsize; dj++ ) {
            int indexl=((il+di)*widthl+jl+dj)*3;
            int indexr=((ir+di)*widthr+jr+dj)*3;
            double dr=(double)imgl[indexl]-(double)imgr[indexr];
            double dg=(double)imgl[indexl+1]-(double)imgr[indexr+1];
            double db=(double)imgl[indexl+2]-(double)imgr[indexr+2];
            q+=dr*dr+dg*dg+db*db;
        }
    }
    return q;
}

vector<MATCH> matchingM(int heightl, int widthl, unsigned char *imgL,
                       int heightr, int widthr, unsigned char *imgR,vector<KEYPOINT>pointsL, vector<KEYPOINT>pointsR, int size){
    //preference lists for points from left and right image
    vector<MATCHPOINT> *listL=new vector<MATCHPOINT>[pointsL.size()];
    vector<MATCHPOINT> *listR=new vector<MATCHPOINT>[pointsR.size()];
    //fill lists
    double h;
    for (int l = 0; l < pointsL.size(); ++l) {
        for (int r = 0; r < pointsR.size(); ++r) {
            h=diff(heightl,widthl,imgL,pointsL[l].y,pointsL[l].x,heightr,widthr,imgR,pointsR[r].y,pointsR[r].x,size);
            if (h<numeric_limits<double>::max()){
                //if points are a possible match add them to the preference lists
                listL[l].push_back(MATCHPOINT(l,r,h));
                listR[r].push_back(MATCHPOINT(l,r,h));
            }
        }
    }
    //sort preference lists
    for (int i = 0; i < pointsL.size(); ++i) {
        sort(listL[i].begin(), listL[i].end(), comp);
    }
    for (int i = 0; i < pointsR.size(); ++i) {
        sort(listR[i].begin(), listR[i].end(), comp);
    }

    //best proposal for each right point
    MATCHPOINT *proposal=new MATCHPOINT[pointsR.size()];

    //whether left points are engaged
    bool *engaged=new bool[pointsL.size()];
    for (int i = 0; i < pointsL.size(); ++i) {
        engaged[i]=false;
    }
    bool finished=false;
    while (!finished) {
        finished=true;
        for (int l = 0; l < pointsL.size(); ++l) {
            if ((!engaged[l])&&listL[l].size()>0) {
                //looking for new engagement
                int z=listL[l].size();
                MATCHPOINT df=listL[l][0];
                if (proposal[listL[l][0].r].l==-1){
                    //no previous engagement
                    proposal[listL[l][0].r]=listL[l][0].copy();
                    engaged[l]=true;
                }else if (proposal[listL[l][0].r].v>listL[l][0].v){
                    //change engagement
                    engaged[proposal[listL[l][0].r].l]=false;
                    finished=false;
                    proposal[listL[l][0].r]=listL[l][0].copy();
                    engaged[l]=true;
                }
                MATCHPOINT check=proposal[listL[l][0].r];
                listL[l].erase(listL[l].begin());
                cout<<check.l<<","<<check.r<<","<<check.v<<"\n";

            }
        }
    }
    vector<MATCH> matches;
    for (int i = 0; i < pointsR.size(); ++i) {
        MATCHPOINT check=proposal[i];
        cout<<check.l<<","<<check.r<<","<<check.v<<"\n";
        if (check.l!=-1)
            matches.push_back(MATCH(pointsL[check.l],pointsR[check.r]));
    }

    return matches;
}
