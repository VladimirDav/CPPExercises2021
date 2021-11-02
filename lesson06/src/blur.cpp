#pragma once
#pragma GCC optimize("O3","Ofast","unroll-loops")
#pragma GCC target("avx2")
#include "blur.h"
#include <iostream>
#include <utility>
#include <vector>
#include <math.h>
#include <algorithm>
#include <map>
#include <string>
#include <set>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <ctime>
#include <opencv2/highgui.hpp>
using namespace std;
const double pi=3.141592653589;
vector <double> operator + (vector <double> a,vector <double> b)
{
    assert(a.size()==b.size());
    for(int i=0;i<a.size();++i) a[i]+=b[i];
    return a;
}
vector <double> operator * (vector <double> a,double val)
{
    for(auto &h:a) h*=val;
    return a;
}
cv::Mat blur(cv::Mat img, double sigma) {
    double tm1=clock();
    vector <int> v={0,0,0};
    int sz=max(4,(int) (3*sigma));
    // TODO формулу весов можно найти тут:
    int n=img.rows;int m=img.cols;
    cout<<n<<" n "<<m<<" m "<<endl;
    vector<vector<vector <double> > > imgall(n);
    for(int i=0;i<n;++i) imgall[i].resize(m);
    for(int i=0;i<n;++i)
    {
        for(int j=0;j<m;++j)
        {
            cv::Vec3b color=img.at<cv::Vec3b>(i, j);
            imgall[i][j].resize(3);
            imgall[i][j][0]=color[0];imgall[i][j][1]=color[1];imgall[i][j][2]=color[2];
        }
    }
    double sum=0;
    for(int k=(-sz);k<=sz;++k) {
        for (int l = (-sz); l <= sz; ++l) {
            double val = (1.0 / (2.0 * pi * (sigma * sigma))) * exp(-(k * k + l * l) / (2 * sigma * sigma));
            sum += val;
        }
    }
    for(int i=sz;i<n-sz;++i)
    {
        for(int j=sz;j<m-sz;++j)
        {
            cv::Vec3b &color=img.at<cv::Vec3b>(i, j);
            vector <double> res={0,0,0};
            for(int k=(-sz);k<=sz;++k)
            {
                for(int l=(-sz);l<=sz;++l)
                {
                    double val=(1.0/(2.0*pi*(sigma*sigma)))*exp(-(k*k+l*l)/(2*sigma*sigma));
                    vector <double> o=imgall[i+k][j+l]*((val+0.0)/(sum+0.0));
                    res=res+o;
                }
            }
            color[0]=res[0];color[1]=res[1];color[2]=res[2];
        }
    }
    cout<<img.rows<<' '<<img.cols<<endl;
    double tm2=clock();
    cout<<((tm2-tm1+0.0))/CLOCKS_PER_SEC<<endl;
    // https://ru.wikipedia.org/wiki/%D0%A0%D0%B0%D0%B7%D0%BC%D1%8B%D1%82%D0%B8%D0%B5_%D0%BF%D0%BE_%D0%93%D0%B0%D1%83%D1%81%D1%81%D1%83
    return img;
}
