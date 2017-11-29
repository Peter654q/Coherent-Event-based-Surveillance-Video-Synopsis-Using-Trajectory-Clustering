    #include <cv.h>  
    #include <cxcore.h>  
    #include <highgui.h>  
    #include <fstream>
	#include <sstream>
    #include <cmath>  
    #include <vector>  
    #include <iostream>  
    using namespace std;  
      
    const int winHeight=1080;  
    const int winWidth=1920;  
      
      
    //CvPoint mousePosition=cvPoint(winWidth>>1,winHeight>>1);  
      
    //mouse event callback  
    /*void mouseEvent(int event, int x, int y, int flags, void *param )  
    {  
        if (event==CV_EVENT_MOUSEMOVE) {  
            mousePosition=cvPoint(x,y);  
        }  
    }*/  
      
    int main (void)  
    {  

        //1.kalman filter setup  
        const int stateNum=4;  
        const int measureNum=2;
        CvKalman* kalman[20];//for 20 objs
        kalman[0] = cvCreateKalman( stateNum, measureNum, 0 );//state(x,y,detaX,detaY)
        //CvMat* process_noise = cvCreateMat( stateNum, 1, CV_32FC1 );  
        CvMat* measurement[20];
        measurement[0] = cvCreateMat( measureNum, 1, CV_32FC1 );//measurement(x,y)  
        CvRNG rng = cvRNG(-1);  
        float A[stateNum][stateNum] ={//transition matrix  
            1,0,1,0,  
            0,1,0,1,  
            0,0,1,0,  
            0,0,0,1  
        };  
      
        memcpy( kalman[0]->transition_matrix->data.fl,A,sizeof(A));  
        cvSetIdentity(kalman[0]->measurement_matrix,cvRealScalar(1) );  
        cvSetIdentity(kalman[0]->process_noise_cov,cvRealScalar(1e-5));  
        cvSetIdentity(kalman[0]->measurement_noise_cov,cvRealScalar(1e-1));  
        cvSetIdentity(kalman[0]->error_cov_post,cvRealScalar(1));  
        //initialize post state of kalman filter at random  
        cvRandArr(&rng,kalman[0]->state_post,CV_RAND_UNI,cvRealScalar(0),cvRealScalar(winHeight>winWidth?winWidth:winHeight));  
      
        CvFont font;  
        cvInitFont(&font,CV_FONT_HERSHEY_SCRIPT_COMPLEX,1,1);  
      
        cvNamedWindow("kalman");  
        //cvSetMouseCallback("kalman",mouseEvent);  
        IplImage* img=cvLoadImage("BG.jpg",CV_LOAD_IMAGE_UNCHANGED);  

        kalman[1] = cvCreateKalman( stateNum, measureNum, 0 );//state(x,y,detaX,detaY)
        measurement[1] = cvCreateMat( measureNum, 1, CV_32FC1 );//measurement(x,y) 
        memcpy( kalman[1]->transition_matrix->data.fl,A,sizeof(A));  
        cvSetIdentity(kalman[1]->measurement_matrix,cvRealScalar(1) );  
        cvSetIdentity(kalman[1]->process_noise_cov,cvRealScalar(1e-5));  
        cvSetIdentity(kalman[1]->measurement_noise_cov,cvRealScalar(1e-1));  
        cvSetIdentity(kalman[1]->error_cov_post,cvRealScalar(1));  
        //initialize post state of kalman filter at random  
        cvRandArr(&rng,kalman[1]->state_post,CV_RAND_UNI,cvRealScalar(0),cvRealScalar(winHeight>winWidth?winWidth:winHeight));

        int frameNumber=1915;
        double array1[20][8] = {0};

        while (1){  
            //2.kalman prediction  
            for (int o=0; o<2 ; o++){
	            const CvMat* prediction=cvKalmanPredict(kalman[o],0);  
	            CvPoint predict_pt=cvPoint((int)prediction->data.fl[0],(int)prediction->data.fl[1]);  
	      
	            //3.update measurement 
	            if ((frameNumber <= 1940 && frameNumber >= 1930) || (frameNumber <= 1990 && frameNumber >= 1980))
	            {	
	            	measurement[o]->data.fl[0]=(float)prediction->data.fl[0];  
		            measurement[o]->data.fl[1]=(float)prediction->data.fl[1];
	            } 
	            else
	            {
	            	fstream fp1;
					stringstream ss1;
					ss1 << "26/F_out" << frameNumber << ".txt";
					//ss1 << "F55.txt";
					string str1 = ss1.str();
					fp1.open(str1.c_str(), ios::in);
					for(int i=0;i<20;i++){
						for(int j=0;j<8;j++){
							fp1 >> array1[i][j];
						}
					}
		            measurement[o]->data.fl[0]=(float)array1[o][6];  
		            measurement[o]->data.fl[1]=(float)array1[o][7]; 
	            }
				 
	      		frameNumber++;
	            //4.update  
	            cvKalmanCorrect( kalman[o], measurement[o] );       
	      
	            //draw   
	            //cvSet(img,cvScalar(255,255,255,0));
	            if (o==0)  
	            	cvCircle(img,predict_pt,5,CV_RGB(0,255,0),3);//predicted point with green  
	        	else
	        		cvCircle(img,predict_pt,5,CV_RGB(255,0,0),3);
	            //cvCircle(img,mousePosition,5,CV_RGB(255,0,0),3);//current position with red  
	            //char buf[256];  
	            //printf("predicted position:(%3d,%3d)\n",predict_pt.x,predict_pt.y);  
	            //cvPutText(img,buf,cvPoint(10,30),&font,CV_RGB(0,0,0));  
	            //printf("current position :(%3d,%3d)\n",mousePosition.x,mousePosition.y);  
	            //cvPutText(img,buf,cvPoint(10,60),&font,CV_RGB(0,0,0));  
	              
	            cvShowImage("kalman", img);  
	            
	        }       
	        char key=(char)cvWaitKey(30);  
	        if (key==27){//esc     
	            break;     
	        } 
      	}

        cvReleaseImage(&img);  
        cvReleaseKalman(&kalman[0]);  
        return 0;  
    }  
