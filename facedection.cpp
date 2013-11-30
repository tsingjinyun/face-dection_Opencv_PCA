//����ʱ������cout��new���������ַ�����F10����Ҫ��F11
#include <stdio.h>          //char *itoa()
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include "cv.h"
#include "highgui.h"
using namespace std;

#define stdImgwidth 90
#define stdImgheight 120

int pFaceRectSeq_numtest=0; //ѵ����������
int pFaceRectSeq_numtrain=0;//������������

//��ǩ��������
double hScale=0.5;
double vScale=0.5;
int lineWidth=1;
CvFont font;


void displayDetections(IplImage * pInpImg,CvSeq * pFaceRectSeq,char * FileName,int n);
void ReconPca();//Pca�㷨ʶ��
void loadImg_row(int pFaceRectSeq_num,int n);//�������ͼ�����е���ʽ�洢��һ��ѵ�������У�һ������ͼ�����е���ʽ�洢��һ��ѵ��������
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

	////picture�Ĵ���
	pInpImg = cvLoadImage("picture.jpg",CV_LOAD_IMAGE_COLOR);
	if(!pInpImg||!pStorage||!pCascade)
	{
		cout<<"Initialization failed:"<<endl;

		return 0;
	}
	pFaceRectSeq = cvHaarDetectObjects(pInpImg,pCascade,pStorage,1.1,3,CV_HAAR_DO_CANNY_PRUNING,cvSize(20,20));
	displayDetections(pInpImg,pFaceRectSeq,"picture.jpg",0);      

	pFaceRectSeq_numtrain=pFaceRectSeq->total;
	//photo�Ĵ���
	
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

void displayDetections(IplImage * pInpImg,CvSeq * pFaceRectSeq,char * FileName,int n)//int n:����picture��photo
{
	char * DISPLAY_WINDOW =new char[30];  /*��������ַ���ʱ��FileName�ռ䲻��������*/
	strcpy(DISPLAY_WINDOW,"Haar Window");
	
	int i;
	IplImage * dst = cvCreateImage(cvSize(stdImgwidth,stdImgheight),pInpImg->depth,pInpImg->nChannels);//pInpImg->depthӦ�ù�һ��������
	IplImage* dst2 = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);//��������ͼ��ע�⣺IPL_DEPTH_8U
	

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


		cvSetImageROI(pInpImg,*r);       //��������֮�����ñ�ǩ

		cvResize(pInpImg,dst,CV_INTER_LINEAR);
		
		//��ɫ��������
		
		strcat(names,".bmp");   
		cvSaveImage(names,dst);
		delete []names;

	    //�������Ᵽ��
		cvCvtColor(dst,dst2,CV_BGR2GRAY);  //IPL_DEPTH_8U���Ҷ�ͼ
		cvEqualizeHist( dst2, dst2);
		char * name2=new char[18];
		itoa(i+1+30*n,name2, 10); 
		strcat(name2,".jpg");
		//strcat(name2,".pgm");
		cvSaveImage(name2,dst2);
		delete []name2;

		cvResetImageROI(pInpImg);//ȡ��ѡ��

	/*	CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32SC1);
		cvConvert(dst2,mat);
		CvMat row_header, *row;
		row = cvReshape( mat, &row_header, 0, 1 );  //row��mat���б���ʽ
		cout<<"mat->height*mat->width-dst2->height*dst2->width: "<<mat->height*mat->width-dst2->height*dst2->width<<endl;  //0
		cout<<"row->width-mat->height*mat->width: "<<row->width-mat->height*mat->width<<endl;                          //0
		cout<<"row->width-dst2->height*dst2->widthStep: "<<row->width-dst2->height*dst2->widthStep<<endl;                 //-240
		cout<<"row->width-dst2->imageSize: "<<row->width-dst2->imageSize<<endl;                                        //-240
		cout<<"row->width-90*120 "<<row->width-90*120<<endl;                                                            //0
*/

	}


	strcat(DISPLAY_WINDOW,FileName);
	cvNamedWindow(DISPLAY_WINDOW,CV_WINDOW_AUTOSIZE); /*�������ͱ�����������ɱ���������ౣ�����������⣬�ر����Ǽ���*/
	cvShowImage(DISPLAY_WINDOW,pInpImg);       

	char * p=new char[5];
	itoa(n,p,10); 
	strcat(p,".bmp");         /*FileName��.jpg��׺*/
 	cvSaveImage(p,pInpImg);

	cvWaitKey(0);
 	cvReleaseImage(&dst);
	cvReleaseImage(&dst2);
	cvReleaseImage(&pInpImg);   /*��û����һ�䣬������ͼƬ�ֹ���pInpImg�����ܻ���ɳ�ͻ����ò�ͬͼƬʹ�ò�ͬ��IplImage*���� */
	
    cout<<n<<"       pFaceRectSeq->total: "<<pFaceRectSeq->total<<endl;
}

void loadImg_row(int pFaceRectSeq_num,int n)//pFaceRectSeq_num����������int n��ѵ��or��������
{
	IplImage * dst2 = 0;
	int a;
	char * name2=new char[8];  
	
	CvMat row_header, *row;

	if(n==0)  
	{
	
		TrainImgs_row = cvCreateMat( pFaceRectSeq_num,stdImgwidth*stdImgheight, CV_32FC1 );  //ÿ��ͼƬ��һ�С�
		

		for( a=0;a<pFaceRectSeq_num;a++)
		{
			itoa(a+1+30*n,name2, 10); 
			strcat(name2,".jpg");   
			dst2=cvLoadImage(name2,0);
			CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32FC1);
			cvConvert(dst2,mat);         //dst2=cvLoadImage(name2,1)����ֵ��Դ��󣬲�ƥ�䡣�����Ǳ���ͼƬ���Ѿ��ǻҶ�ͼ������ɫ��ʽ�����ı� �������� ��      
			//CvMat row_header, *row;
			row = cvReshape( mat, &row_header, 0, 1 );  //row��mat���б���ʽ
		
			
		    float* ptr=(float*)(TrainImgs_row->data.fl+a*TrainImgs_row->step/4);  //____step/4
			float* ptr2=(float*)row->data.fl;
			
			for(int b=0;b<TrainImgs_row->cols;b++)     ////��TrainImgs_row->step������
			{
				 *ptr = *ptr2;
				 ptr++;
				 ptr2++;
			}
			
		}
		cout<<"ѵ���������سɹ���"<<a<<"��"<<endl;
	}
	else if(n==1)
	{
		cout<<"n: "<<n<<endl;
		TestImg_row = cvCreateMat( pFaceRectSeq_num,stdImgwidth*stdImgheight, CV_32FC1 );  //һ��ͼƬ��һ��

		for(int a=0;a<pFaceRectSeq_num;a++)
		{
			itoa(a+1+30*n,name2, 10); 
			strcat(name2,".jpg");   
			dst2=cvLoadImage(name2,0);

			CvMat* mat=cvCreateMat(dst2->height,dst2->width,CV_32FC1);
			cvConvert(dst2,mat);                                             
			//CvMat row_header, *row;
			row = cvReshape( mat, &row_header, 0, 1 );  //row��mat���б���ʽ
		
			
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
		
			cout<<"�����������سɹ���"<<a<<"��"<<endl;
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
	//�ܵ���������TrainImgs_row->rows������ά����TrainImgs_row->cols�����ɷ���Ŀ:min(TrainImgs_row->rows,TrainImgs_row->cols)�����Լ�������ȡǰ���ٸ�����������?????
	loadImg_row(pFaceRectSeq_numtrain,0);
	loadImg_row(pFaceRectSeq_numtest,1);          
	CvMat* avg = cvCreateMat(1,TrainImgs_row->cols,CV_32FC1);
	CvMat* eigenvalues = cvCreateMat(1,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);
	CvMat* eigenvectors = cvCreateMat(min(TrainImgs_row->rows,TrainImgs_row->cols),TrainImgs_row->cols,CV_32FC1);
	CvMat* result = cvCreateMat(TrainImgs_row->rows,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);//���ɷ���Ŀ:min(TrainImgs_row->rows,TrainImgs_row->cols)
	CvMat* result2 = cvCreateMat(TestImg_row->rows,min(TrainImgs_row->rows,TrainImgs_row->cols),CV_32FC1);
	/*CvMat* avg ;
	CvMat* eigenvalues ;
	CvMat* eigenvectors ;
	CvMat* result ;
	CvMat* result2 ;*/          /*��������*/

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
	int dex=1;                           //���������Ǵ�1��ʼ��
	float temp=*diffr;
	for(int ii=1;ii<result->rows;ii++)
	{	
			diffr++;	
			if(temp>*diffr)
			{
				dex=ii+1;            //���������Ǵ�1��ʼ��
				temp=*diffr;
			}
	

	}
	cout<<endl;
	cout<<"���������ƥ�������: "<<dex<<".bmp"<<endl;
	//cout<<"��С����: "<<temp<<endl;

	IplImage * dexImg;
	IplImage * teImg;

	char * pp2=new char[5];
	itoa(dex,pp2,10); 
	strcat(pp2,".bmp");     
	dexImg=cvLoadImage(pp2,1);
 	cvNamedWindow("ƥ������",CV_WINDOW_AUTOSIZE); 
	cvShowImage("ƥ������",dexImg); 

	
	teImg=cvLoadImage("31.bmp",1);
 	cvNamedWindow("��������",CV_WINDOW_AUTOSIZE); 
	cvShowImage("��������",teImg); 


	cvWaitKey(0);
//	delete []pp2;             //�����ͳ������ݲ�֪��Ϊʲô
	cvReleaseImage(&dexImg);
	cvReleaseImage(&teImg);


}


