/*
*  Copyright (c) 2018-2019 Ling Zhang (lingzhang (at) whu.edu.cn) and
*  Qingan Yan (yanqingan (at) whu.edu.cn &&
*  yanqinganssg (at) gmail.com) and Wuhan UNiversity.
*
*  This code implements the core part of paper [1].
*  If you use it for your research, please cite the following paper:
*
*  [1] Ling Zhang, Qingan Yan, Yao Zhu, Xiaolong Zhang, Chunxia Xiao.
*  Effective Shadow Removal via Multi-scale Image Decomposition.
*  The Visual Computer (TVC), 2019. (Proceedings of Computer Graphics International 2019)
*
*  For more information, please move to my homepage
*  https://yanqingan.github.io/ or github https://github.com/yanqingan
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*/

#include "stdafx.h"
#include <math.h>
#include <opencv.hpp>
using namespace cv;
using namespace std;


void imagesmooth()
{
	int k = 3;
	Mat image = imread("11a.jpg");
	namedWindow("image0");
	imshow("image0", image);
	blackimg = cvLoadImage("11a.jpg");
	int height = image.rows;
	int width = image.cols;
	int size = height*width;
	cout<<"rows="<<height<<"  cols="<<width<<endl;

	double** I;
	I = new double*[3];
	for (int i=0;i<3;i++)
	{
		I[i] = new double [size];
	}

	int** MaskFlag = new int*[height];
	for (int i=0;i<height;i++)
	{
		MaskFlag[i] = new int[width];
	}

	int num_0=0;
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{

			Vec3b t0 = image.at<Vec3b>(i,j);
			I[0][i*width+j] = t0[0];
			I[1][i*width+j] = t0[1];
			I[2][i*width+j] = t0[2];  

			CvScalar data1 = cvGet2D(blackimg,i,j);
			//cout<<i<<" "<<j<<"   ";
			if (data1.val[0] == 0)
			{
				MaskFlag[i][j] = 1;
				num_0++;
			}
			else
				MaskFlag[i][j] = 0;
		}
	}

	Mat G;
	Mat dst;
	Mat tmp;
	image.copyTo(tmp);
	dst = image;
	std::vector<float> detail0;
	std::vector<float> detail1;
	std::vector<float> detail2;
	int smooth_number = 10;

	for (int i=0;i<10;i++)
	{
		dst.copyTo(tmp);
		localsmooth(I, MaskFlag, height, width, dst,tmp,i);
		//smooth(I, MaskFlag, dst, detail0, detail1, detail2);
		for (int ii = 0; ii < dst.rows; ii++)
		{
			for (int jj = 0; jj < dst.cols; jj++)
			{
				//if (MaskFlag[ii][jj] == 0)
				{
					Vec3b dst_data = dst.at<Vec3b>(ii,jj);
					Vec3b tmp_data = tmp.at<Vec3b>(ii,jj);
					float aa = tmp_data[0] - dst_data[0];
					//cout<<aa<<" ";
					detail0.push_back(aa);
					aa = tmp_data[1] - dst_data[1];
					detail1.push_back(aa);
					aa = tmp_data[2] - dst_data[2];
					detail2.push_back(aa);
				}



			}
		}
		stringstream ss;
		ss<<i;
		string savefile="filter"+ss.str()+".bmp";
		imwrite(savefile.c_str(),dst);

	}
}


void detailadd()
{
	IplImage* i1 = cvLoadImage("52a.JPG");
	IplImage* i2 = cvLoadImage("52filter2.bmp");
	IplImage* i3 = cvLoadImage("52s2.jpg");
	for (int i=0;i<i1->height;i++)
	{
		for (int j=0;j<i1->width;j++)
		{
			CvScalar d1, d2, d3;
			d1 = cvGet2D(i1,i,j);
			d2 = cvGet2D(i2,i,j);
			d3 = cvGet2D(i3,i,j);
			d3.val[0] = d3.val[0] + (d1.val[0] - d2.val[0])*1.0;
			d3.val[1] = d3.val[1] + (d1.val[1] - d2.val[1])*1.0;
			d3.val[2] = d3.val[2] + (d1.val[2] - d2.val[2])*1.0;
			cvSet2D(i3,i,j,d3);
		}
	}
	cvSaveImage("detailretsult.bmp",i3);
}