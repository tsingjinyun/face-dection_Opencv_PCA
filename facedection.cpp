//调试时，遇到cout、new这类特殊字符，按F10而不要用F11
#include <stdio.h>          //char *itoa()
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include "cv.h"
#include "highgui.h"
using namespace std;

#define stdImgwidth 90
#define stdImgheight 120

int pFaceRectSeq_numtest=0; //训练样本数量
int pFaceRectSeq_numtrain=0;//测试样本数量

//标签文字设置
double hScale=0.5;
double vScale=0.5;
int lineWidth=1;
CvFont font;


void displayDetections(IplImage * pInpImg,CvSeq * pFaceRectSeq,char * FileName,int n);
void ReconPca();//Pca算法识别
void loadImg_row(int pFaceRectSeq_num,int n);//多个样本图像以行的形式存储到一个训练矩阵中，一个测试图像以行的形式存储到一个训练矩阵中
CvMat* TrainImgs_row=0;
CvMat* TestImg_row=0;


int main(int argc,char ** argv)
{
	IplImage * pInpImg = 0;
	IplImage * pInpImg2 = 0;
	CvHaarClassifierCascade * pCascade = 0;
	CvMemStorage * pStorage = 0;
	CvSeq * pFaceRectSeq;



	pStorage = cvCreateMemStorage(0);
	pCascade = (CvHaarClassifierCascade * )cvLoad(("haarcascade_frontalface_default.xml"),0,0,0);

	////picture的处理
	pInpImg = cvLoadImage("picture.jpg",CV_LOAD_IMAGE_COLOR);
	if(!pInpImg||!pStorage||!pCascade)
	{
		cout<<"Initialization failed:"<<endl;

		return 0;
	}
	pFaceRectSeq = cvHaarDetectObjects(pInpImg,pCascade,pStorage,1.1,3,CV_HAAR_DO_CANNY_PRUNING,cvSize(20,20));
	displayDetections(pInpImg,pFaceRectSeq,"picture.jpg",0);      

	pFaceRectSeq_numtrain=pFaceRectSeq->total;
	//photo的处理
	
	pInpImg2 = cvLoadImage("photo.jpg",CV_LOAD_IMAGE_COLOR);
	if(!pInpImg2)
	{
		cout<<"Initialization failed:"<<endl;

		return 0;
	}
	pFaceRectSeq = cvHaarDetectObjects(pInpImg2,pCascade,pStorage,1.1,3,CV_HAAR_DO_CANNY_PRUNING,cvSize(20,20));
	displayDetections(pInpImg2,pFaceRectSeq,"photo.jpg",1); 
	
	pFaceRectSeq_numtest=pFaceRectSeq->total;

	ReconPca();

	if(pCascade) cvReleaseHaarClassifierCascade(&pCascade);
	if(pStorage) cvReleaseMemStorage(&pStorage);

	return 0;
}

void displayDetections(IplImage * pInpImg,CvSeq * pFaceRectSeq,char * FileName,int n)//int n:区分picture和photo
{
	char * DISPLAY_WINDOW =new char[30];  /*解决连接字符串时，FileName空间不够的问题*/
	strcpy(DISPLAY_WINDOW,"Haar Window");
	
	int i;
	IplImage * dst = cvCreateImage(cvSize(stdImgwidth,stdImgheight),pInpImg->depth,pInpImg->nChannels);//pInpImg->depth应该归一化？？？
	IplImage* dst2 = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);//创建样本图像。注意：IPL_DEPTH_8U
	

	cout<<"pFaceRectSeq->total: "<<pFaceRectSeq->total<<endl;



	for(i=0;i<(pFaceRectSeq?pFaceRectSeq->total:0);i++)
	{
		CvRect * r = (CvRect*)cvGetSeqElem(pFaceRectSeq,i);
		CvPoint pt1 = {r->x,r->y};
		CvPoint pt2 = {r->x+r->width,r->y+r->height};
		//CvPoint pt3 = {r->x+r->width/4,r->y+r->height/4};

		char * names=new char[18];
		itoa(i+1+30*n,names, 10); 

		cvRectangle(pInpImg,pt1,pt2,CV_RGB(0,255,0),3,4,0);
		CvFont font;
		cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, hScale,vScale,0,lineWidth);
        cvPutText (pInpImg,names,pt1, &font, cvScalar(255,255,255));


		cvSetImageROI(pInpImg,*r);       //不能在这之后设置标签

		cvResize(pInpImg,dst,CV_INTER_LINEAR);
		
		//彩色样本保存
		
		strcat(names,".bmp");   
		cvSaveImage(names,dst);
		delete []names;

	    //样本均衡保存
		cvCvtColor(dst,dst2,CV_BGR2GRAY);  //IPL_DEPTH_8U，灰度图
		cvEqualizeHist( dst2, dst2);
		char * name2=new char[18];
		itoa(i+1+30*n,name2, 10); 
		strcat(name2,".jpg");
		//strcat(name2,".pgm");
		cvSaveImage(name2,dst2);
		delete []name2;

		cvResetImageROI(pInpImg);//取消选定

	/*	CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32SC1);
		cvConvert(dst2,mat);
		CvMat row_header, *row;
		row = cvReshape( mat, &row_header, 0, 1 );  //row是mat的行变形式
		cout<<"mat->height*mat->width-dst2->height*dst2->width: "<<mat->height*mat->width-dst2->height*dst2->width<<endl;  //0
		cout<<"row->width-mat->height*mat->width: "<<row->width-mat->height*mat->width<<endl;                          //0
		cout<<"row->width-dst2->height*dst2->widthStep: "<<row->width-dst2->height*dst2->widthStep<<endl;                 //-240
		cout<<"row->width-dst2->imageSize: "<<row->width-dst2->imageSize<<endl;                                        //-240
		cout<<"row->width-90*120 "<<row->width-90*120<<endl;                                                            //0
*/

	}


	strcat(DISPLAY_WINDOW,FileName);
	cvNamedWindow(DISPLAY_WINDOW,CV_WINDOW_AUTOSIZE); /*若有它就报错，可能是杀毒软件这类保护软件的问题，关闭它们即可*/
	cvShowImage(DISPLAY_WINDOW,pInpImg);       

	char * p=new char[5];
	itoa(n,p,10); 
	strcat(p,".bmp");         /*FileName有.jpg后缀*/
 	cvSaveImage(p,pInpImg);

	cvWaitKey(0);
 	cvReleaseImage(&dst);
	cvReleaseImage(&dst2);
	cvReleaseImage(&pInpImg);   /*若没有这一句，而几张图片又共用pInpImg，可能会造成冲突。最好不同图片使用不同的IplImage*名字 */
	
    cout<<n<<"       pFaceRectSeq->total: "<<pFaceRectSeq->total<<endl;
}

void loadImg_row(int pFaceRectSeq_num,int n)//pFaceRectSeq_num：样本数；int n：训练or测试样本
{
	IplImage * dst2 = 0;
	int a;
	char * name2=new char[8];  
	
	CvMat row_header, *row;

	if(n==0)  
	{
	
		TrainImgs_row = cvCreateMat( pFaceRectSeq_num,stdImgwidth*stdImgheight, CV_32FC1 );  //每张图片做一行。
		

		for( a=0;a<pFaceRectSeq_num;a++)
		{
			itoa(a+1+30*n,name2, 10); 
			strcat(name2,".jpg");   
			dst2=cvLoadImage(name2,0);
			CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32FC1);
			cvConvert(dst2,mat);         //dst2=cvLoadImage(name2,1)会出现调试错误，不匹配。估计是保存图片（已经是灰度图）后，颜色格式有所改变 ，不明白 。      
			//CvMat row_header, *row;
			row = cvReshape( mat, &row_header, 0, 1 );  //row是mat的行变形式
		
			
		    float* ptr=(float*)(TrainImgs_row->data.fl+a*TrainImgs_row->step/4);  //____step/4
			float* ptr2=(float*)row->data.fl;
			
			for(int b=0;b<TrainImgs_row->cols;b++)     ////若TrainImgs_row->step，错误。
			{
				 *ptr = *ptr2;
				 ptr++;
				 ptr2++;
			}
			
		}
		cout<<"训练样本加载成功！"<<a<<"个"<<endl;
	}
	else if(n==1)
	{
		cout<<"n: "<<n<<endl;
		TestImg_row = cvCreateMat( pFaceRectSeq_num,stdImgwidth*stdImgheight, CV_32FC1 );  //一张图片做一行

		for(int a=0;a<pFaceRectSeq_num;a++)
		{
			itoa(a+1+30*n,name2, 10); 
			strcat(name2,".jpg");   
			dst2=cvLoadImage(name2,0);

			CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32FC1);
			cvConvert(dst2,mat);                                             
			//CvMat row_header, *row;
			row = cvReshape( mat, &row_header, 0, 1 );  //row是mat的行变形式
		
			
		//	TrainImgs_row->data.ptr+a*TrainImgs_row->step=(double float*)row->data.ptr;
			float* ptr=(float*)(TestImg_row->data.fl+a*TestImg_row->step/4);
			float* ptr2=(float*)row->data.fl;
			for(int b=0;b<TestImg_row->cols;b++)  
			{
				 *ptr = *ptr2;
				 ptr=ptr+1;
				 ptr2=ptr2+1;
			}
		}
		
			cout<<"测试样本加载成功！"<<a<<"个"<<endl;
	}

	else
	{
		cout<<"n is wrong!"<<endl;
	}
	delete []name2;
	cout<<"ok"<<endl;
		
}

void ReconPca()
{
	//总的样本数：TrainImgs_row->rows；样本维数：TrainImgs_row->cols；主成分数目:min(TrainImgs_row->rows,TrainImgs_row->cols)——自己决定（取前多少个特征向量）?????
	loadImg_row(pFaceRectSeq_numtrain,0);
	loadImg_row(pFaceRectSeq_numtest,1);          
	CvMat* avg = cvCreateMat(1,TrainImgs_row->cols,CV_32FC1);
	CvMat* eigenvalues = cvCreateMat(1,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);
	CvMat* eigenvectors = cvCreateMat(min(TrainImgs_row->rows,TrainImgs_row->cols),TrainImgs_row->cols,CV_32FC1);
	CvMat* result = cvCreateMat(TrainImgs_row->rows,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);//主成分数目:min(TrainImgs_row->rows,TrainImgs_row->cols)
	CvMat* result2 = cvCreateMat(TestImg_row->rows,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);
	/*CvMat* avg ;
	CvMat* eigenvalues ;
	CvMat* eigenvectors ;
	CvMat* result ;
	CvMat* result2 ;*/          /*这样不行*/

	cvCalcPCA( TrainImgs_row, avg, eigenvalues, eigenvectors, CV_PCA_DATA_AS_ROW );       //wrong?????????????


	cvProjectPCA( TrainImgs_row, avg, eigenvectors, result );

	cvProjectPCA( TestImg_row, avg, eigenvectors, result2 );

	float *diff = new float[result->rows]; 
	float *diffr = diff;

	 
  	float* resptr1;
	float* resptr2;

	float su=0.0;
	for(int i=0;i<result->rows;i++)
	{	
		su=0.0;
		resptr1=(float*)result->data.fl+i*result->step/4;
		resptr2=(float*)result2->data.fl;
		for(int j=0;j<result->cols;j++)
		{			
			su=(float)(*resptr2-*resptr1)*(*resptr2-*resptr1)+su;
			
			resptr2++;
			resptr1++;
			
		}
		*diff = su;
		diff++;

	}
	cout<<"diffr: "<<*(diffr+2)<<endl;
	int dex=1;                           //样本命名是从1开始的
	float temp=*diffr;
	for(int ii=1;ii<result->rows;ii++)
	{	
			diffr++;	
			if(temp>*diffr)
			{
				dex=ii+1;            //样本命名是从1开始的
				temp=*diffr;
			}
	

	}
	cout<<endl;
	cout<<"与测试样本匹配的人脸: "<<dex<<".bmp"<<endl;
	//cout<<"最小方差: "<<temp<<endl;

	IplImage * dexImg;
	IplImage * teImg;

	char * pp2=new char[5];
	itoa(dex,pp2,10); 
	strcat(pp2,".bmp");     
	dexImg=cvLoadImage(pp2,1);
 	cvNamedWindow("匹配人脸",CV_WINDOW_AUTOSIZE); 
	cvShowImage("匹配人脸",dexImg); 

	
	teImg=cvLoadImage("31.bmp",1);
 	cvNamedWindow("测试人脸",CV_WINDOW_AUTOSIZE); 
	cvShowImage("测试人脸",teImg); 


	cvWaitKey(0);
//	delete []pp2;             //有它就出错，暂不知道为什么
	cvReleaseImage(&dexImg);
	cvReleaseImage(&teImg);


}



