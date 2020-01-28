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

void GetInit(IplImage* Img, IplImage* Img_sample, CvScalar &sample_S, CvScalar &sample_L, CvScalar &avgMs, CvScalar & avgMl, double &gama, double &avgmasks, double &avgmaskl, IplImage* mask, IplImage* pmask1, IplImage* pmask2)
{
	//	IplImage *Img_Lab = cvCloneImage(Img);
	int height = Img->height;
	int width = Img->width;
	//	cvCvtColor(Img,Img_Lab,CV_BGR2Lab);
	CvScalar trimap_data, Lab_data, sample_data;
	CvScalar sum_L = cvScalarAll(0.0);
	CvScalar sum_S = cvScalarAll(0.0);
	double sum_masks = 0.0;
	double sum_maskl = 0.0;
	int num_S = 0, num_L = 0;
	IplImage* ms = cvCreateImage(cvGetSize(Img),8,1);
	IplImage* ml = cvCreateImage(cvGetSize(Img_sample),8,1);
	CvScalar msd, mld, /*avgMl, avgMs,*/ maskd;
	msd = cvScalarAll(0);
	mld = cvScalarAll(255);
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			trimap_data = cvGet2D(pmask1,i,j);
			Lab_data = cvGet2D(Img,i,j);
			//	maskd = cvGet2D(mask,i,j);
			cvSet2D(ms,i,j,msd);
			///	cvSet2D(ml,i,j,msd);
			if (trimap_data.val[0] == 0)
			{
				sum_S.val[0] = sum_S.val[0] + Lab_data.val[0];
				sum_S.val[1] = sum_S.val[1] + Lab_data.val[1];
				sum_S.val[2] = sum_S.val[2] + Lab_data.val[2];
				//		sum_masks = sum_masks + maskd.val[0];
				//		cout<<maskd.val[0]<<" ";
				num_S++;
				cvSet2D(ms,i,j,mld);
			}			
		}
	}
	for (int i=0;i<Img_sample->height;i++)
	{
		for (int j=0;j<Img_sample->width;j++)
		{
			trimap_data = cvGet2D(pmask2,i,j);
			Lab_data = cvGet2D(Img_sample,i,j);
			//	maskd = cvGet2D(mask,i,j);
			//	cvSet2D(ms,i,j,msd);
			cvSet2D(ml,i,j,msd);
			if (trimap_data.val[0] == 255)
			{
				sum_L.val[0] = sum_L.val[0] + Lab_data.val[0];
				sum_L.val[1] = sum_L.val[1] + Lab_data.val[1];
				sum_L.val[2] = sum_L.val[2] + Lab_data.val[2];
				//			sum_maskl = sum_maskl + maskd.val[0];
				num_L++;
				cvSet2D(ml,i,j,mld);
			}

		}
	}
	cvAvgSdv( Img, NULL, &avgMs, ms );
	cvAvgSdv( Img_sample, NULL, &avgMl, ml );
	cout<<"s:"<<avgMs.val[0]<<endl;
	cout<<"L:"<<avgMl.val[0]<<endl;

	gama = avgMl.val[0] / avgMs.val[0];
	cout<<"gama:"<<gama<<endl;

	//cvNamedWindow("ms",1);
	//cvShowImage("ms",ms);
	//cvNamedWindow("ml",1);
	//cvShowImage("ml",ml);

	sample_L.val[0] = sum_L.val[0]/num_L;
	sample_L.val[1] = sum_L.val[1]/num_L;
	sample_L.val[2] = sum_L.val[2]/num_L;
	sample_S.val[0] = sum_S.val[0]/num_S;
	sample_S.val[1] = sum_S.val[1]/num_S;
	sample_S.val[2] = sum_S.val[2]/num_S;
	//avgmasks = sum_masks/num_S;
	//avgmaskl = sum_maskl/num_L;
	cout<<"sample_S:"<<sample_S.val[0]<<" "<<sample_S.val[1]<<" "<<sample_S.val[2]<<endl;
	cout<<"sample_L:"<<sample_L.val[0]<<" "<<sample_L.val[1]<<" "<<sample_L.val[2]<<endl;
}

void shadoremoval()
{
	IplImage* input = cvLoadImage("E:/project/ShadowReflectance/ShadowReflectance/zl/T1.jpg");
	IplImage* matte = cvLoadImage("E:/project/ShadowReflectance/ShadowReflectance/zl/T1mask.jpg");
	int height = input->height;
	int width = input->width;
	IplImage* Lab = cvCloneImage(input);
	//IplImage* Lab_sample = cvCloneImage(sample_image);
	//IplImage* mask = cvLoadImage("apply1matte2.bmp");
	IplImage* mask = cvCloneImage(input);
	CvScalar white = cvScalarAll(255);

	IplImage* image_L = cvCloneImage(input);
	IplImage* image_S = cvCloneImage(input);
	CvScalar gf_data;
	CvScalar zero = cvScalarAll(0);
	for (int i=0;i<input->height;i++)
	{
		for (int j=0;j<input->width;j++)
		{
			cvSet2D(mask,i,j,white);
			gf_data = cvGet2D(mask,i,j);
			if (gf_data.val[0] == 0)
			{
				cvSet2D(image_S,i,j,zero);
			}
			else
				cvSet2D(image_L,i,j,zero);
		}
	}
	/*cvNamedWindow("result",1);
	cvShowImage("result",mask);
	cvNamedWindow("s",1);
	cvShowImage("s",image_S);
	cvNamedWindow("l",1);
	cvShowImage("l",image_L);*/
	cvSaveImage("imageS.bmp",image_S);
	cvSaveImage("imageL.bmp",image_L);
	IplImage* free = cvCloneImage(input);
	cvCvtColor(input,Lab,CV_BGR2Lab);
//	cvCvtColor(sample_image,Lab_sample,CV_BGR2Lab);
	IplImage *coarse_beta = cvCreateImage(cvGetSize(Lab),8,1);
	IplImage* src1 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	IplImage* src2 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	IplImage* src3 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	cvSplit(Lab, src1, src2, src3, NULL);
	cvNamedWindow("L0",1);
	cvShowImage("L0",src1);
	cvNamedWindow("a0",1);
	cvShowImage("a0",src2);
	cvNamedWindow("b0",1);
	cvShowImage("b0",src3);
	cvSaveImage("t11.bmp",src1);
	cvSaveImage("t12.bmp",src2);
	cvSaveImage("t13.bmp",src3);

	double *estimate;
	estimate=new double [height*width];
	
	IplImage* pmask1 = cvLoadImage("");// load significant region 
	
	IplImage* pmask2 = cvLoadImage("");// load significant region 
//	GetInitia(Lab, estimate, coarse_beta);
	
	CvScalar sample_S, sample_L, free_data, Lab_data, beta, avgMs, avgMl;
	avgMs = cvScalarAll(0.0);
	avgMl = cvScalarAll(0.0);
	sample_S = cvScalarAll(0);
	sample_L = cvScalarAll(0);
	double gama = 0.0;
	double avgmasks = 0.0;
	double avgmaskl = 0.0;

	GetInit(input/*Lab*/, input/*Lab_sample*/,sample_S, sample_L, avgMs, avgMl, gama, avgmasks, avgmaskl, mask, pmask1, pmask2);
	cout<<"avgMs:"<<avgMs.val[0]<<endl;
	cout<<"avgMl:"<<avgMl.val[0]<<endl;
//	RemovalColorTrans(Lab, mask, sample_S, sample_L, avgMs, avgMl);
	//GetRecolorInit(input, sample_S, sample_L, mask, pmask1);
//	double intesty_rate;
	CvScalar intesty_rate, tmask_data;
//	IplImage* tmask = cvLoadImage("ty_m.jpg");
	//for (int k=0;k<3;k++)
	//{
	//	intesty_rate.val[k] = sample_L.val[k]/sample_S.val[k] -1;
	//	//cout<<"rate:"<<intesty_rate.val[k]<<endl;
	//}
	
	cout<<"gama:"<<gama<<endl;
	double *avg;
	avg = new double [height*width];
	int r = 1;//
	IplImage* avg_map = cvCloneImage(mask);
	CvScalar temp, mask_d;
	double sum_gray = 0.0;
	int num_gray = 0;
	double avg_gray = 0.0;

	CvScalar inputdata;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float sum_avg = 0.0;
			int num_avg = 0;
			int up_left_x = (j - r) > 0 ? (j - r) : 0;
			int up_left_y = (i - r) > 0 ? (i - r) : 0;
			int bottom_right_x = (j + r) < width ? (j + r) : (width - 1);
			int bottom_right_y = (i + r) < height ? (i + r) : (height - 1);
			mask_d = cvGet2D(matte,i,j);
			inputdata = cvGet2D(input,i,j);

			for (int k=0;k<3;k++)
			{
				if (inputdata.val[k] == 0)
				{
					sum_avg = 0.0;
					num_avg = 0;
					for (int y = up_left_y;y<bottom_right_y;y++ )
					{
						for (int x = up_left_x;x<bottom_right_x;x++)
						{
							beta = cvGet2D(input,y,x);
							sum_avg = sum_avg + beta.val[k];
							num_avg++;
						}
					}
					inputdata.val[k] = sum_avg / num_avg;
					//temp.val[0] = temp.val[1] = temp.val[2] = avg[i*width+j];
					
					//sum_gray = sum_gray +avg[i*width+j];
					//num_gray++;
				}
			}
			cvSet2D(input,i,j,inputdata);
			
			//else
				avg[i*width+j] = mask_d.val[0];
			//cout<<avg[i*width+j]<<" ";
		}
	}
	avg_gray = sum_gray / num_gray;
	cvNamedWindow("avg_map",1);
	cvShowImage("avg_map",avg_map);
	cvSaveImage("avg.bmp",avg_map);
	IplImage* avg_temp = cvCloneImage(avg_map);


	for (int i=0;i<input->height;i++)
	{
		for (int j=0;j<input->width;j++)
		{
			Lab_data = cvGet2D(/*Lab*/input,i,j);
			beta = cvGet2D(mask,i,j);


			if (avg[i*width+j]>0)
			{
				beta.val[0] = (255-avg[i*width+j]/*beta.val[0]*/)/255;
				//	beta.val[0] = (avg[i*width+j]/*beta.val[0]*/)/255;
				for (int k=0;k<3;k++)
				{	
						intesty_rate.val[k] = (avgMl.val[k]*(Lab_data.val[k]-sample_S.val[k])/avgMs.val[k]+sample_L.val[k])/Lab_data.val[k] -1;
					free_data.val[k] = Lab_data.val[k] * (1+intesty_rate.val[k])/(beta.val[0]*intesty_rate.val[k]+1);
					//		free_data.val[k] = free_data.val[k] * Lab_data.val[k] / sample_S.val[k];
					if (free_data.val[k]<0)
					{
						free_data.val[k] = 0;
					}
					if (free_data.val[k]>255)
					{
						free_data.val[k] = 255;
					}
				}
				cvSet2D(free,i,j,free_data);
			}
			
		}
	}
//	cvCvtColor(free,free,CV_Lab2BGR);
	cvNamedWindow("free0",1);
	cvShowImage("free0",free);
	cvSaveImage("E:/project/ShadowReflectance/ShadowReflectance/zl/result0.jpg",free);
	cvCvtColor(free,free,CV_BGR2Lab);
	/*for (int k=0;k<5;k++)
	{
		cvSmooth(free,free,CV_GAUSSIAN,7,7,0,0);
	}*/

	IplImage* src10 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	IplImage* src20 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	IplImage* src30 = cvCreateImage(cvSize(input->width,input->height),IPL_DEPTH_8U,1);
	IplImage* free_filter = cvCloneImage(free);
	cvSplit(free, src10, src20, src30, NULL);
	cvNamedWindow("L",1);
	cvShowImage("L",src10);
	cvNamedWindow("a",1);
	cvShowImage("a",src20);
	cvNamedWindow("b",1);
	cvShowImage("b",src30);
	cvWaitKey(-1);
}