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
      
    int main (void)  
    {  
        IplImage* img=cvLoadImage("BG.jpg",CV_LOAD_IMAGE_UNCHANGED);// Background img
        double obj_status[20][4] = {0};//Object array:obj_num, last_frame, (x,y)
        for(int i=0;i<20;i++){
            obj_status[i][0] = -1;//-1 for no objs
        }
        //init value
        const int stateNum=4;  
        const int measureNum=2;
        CvKalman* kalman[20];//for 20 objs
        CvMat* measurement[20];
        CvRNG rng = cvRNG(-1);  
        float A[stateNum][stateNum] ={//transition matrix  
            1,0,1,0,  
            0,1,0,1,  
            0,0,1,0,  
            0,0,0,1  
        };
        //1.kalman filter setup 
        /*kalman[0] = cvCreateKalman( stateNum, measureNum, 0 );//state(x,y,detaX,detaY)
        measurement[0] = cvCreateMat( measureNum, 1, CV_32FC1 );//measurement(x,y)  
        memcpy( kalman[0]->transition_matrix->data.fl,A,sizeof(A));  
        cvSetIdentity(kalman[0]->measurement_matrix,cvRealScalar(1) );  
        cvSetIdentity(kalman[0]->process_noise_cov,cvRealScalar(1e-5));  
        cvSetIdentity(kalman[0]->measurement_noise_cov,cvRealScalar(1e-1));  
        cvSetIdentity(kalman[0]->error_cov_post,cvRealScalar(1));  
        //initialize post state of kalman filter at random  
        cvRandArr(&rng,kalman[0]->state_post,CV_RAND_UNI,cvRealScalar(0),cvRealScalar(winHeight>winWidth?winWidth:winHeight));  */
      
        //cvNamedWindow("kalman");  
        //cvSetMouseCallback("kalman",mouseEvent);  
         

        int txt_start=25;
        int txt_end = 1998;
        

        for(int frame_number = txt_start; frame_number < txt_end+1; frame_number++){
            fstream f_in;
            stringstream ss;
            ss << "txt/F_out" << frame_number << ".txt";
            string str = ss.str();
            f_in.open(str.c_str(), ios::in);
            if(!f_in){
                while(true)
                {
                    frame_number++;
                    stringstream ss;
                    ss << "txt/F_out" << frame_number << ".txt";
                    string str = ss.str();
                    f_in.open(str.c_str(), ios::in);
                    if(f_in)
                        break;
                }
            }
            int trash;
            double tmp[4];//obj_num, last_frame, (x,y)
            while(f_in >> tmp[0]){
                for(int i=0;i<5;i++){
                    f_in >> trash;
                }
                tmp[1] = frame_number;
                f_in >> tmp[2];
                f_in >> tmp[3];

                //compare to obj_status
                bool compare = false;// compare the obj num
                for(int i=0;i<20;i++){
                    if(obj_status[i][0]==tmp[0]){
                        obj_status[i][1]=tmp[1];
                        obj_status[i][2]=tmp[2];
                        obj_status[i][3]=tmp[3];
                        compare = true;
                    }
                }
                if(!compare){
                    double min_dist = 999.9;
                    int min_index = -1;
                    double thresh = 50.0;
                    for (int j=0;j<20;j++){
                        if (obj_status[j][0] != -1){
                            double dist = pow(pow((obj_status[j][2] - tmp[2]), 2) + pow((obj_status[j][3]-tmp[3]), 2), 0.5);// L2 distance
                            if (dist < min_dist){
                                min_dist = dist;
                                min_index = j;
                            }
                        }
                    }
                    if (min_dist < thresh){
                        obj_status[min_index][0] = tmp[0];
                        obj_status[min_index][1] = tmp[1];
                        obj_status[min_index][2] = tmp[2];
                        obj_status[min_index][3] = tmp[3];
                    }else{
                        for(int i=0;i<20;i++){
                            if(obj_status[i][0] == -1){
                                obj_status[i][0] = tmp[0];
                                obj_status[i][1] = tmp[1];
                                obj_status[i][2] = tmp[2];
                                obj_status[i][3] = tmp[3];

                                //init kalman
                                kalman[i] = cvCreateKalman( stateNum, measureNum, 0 );//state(x,y,detaX,detaY)
                                measurement[i] = cvCreateMat( measureNum, 1, CV_32FC1 );//measurement(x,y)  
                                memcpy( kalman[i]->transition_matrix->data.fl,A,sizeof(A));  
                                cvSetIdentity(kalman[i]->measurement_matrix,cvRealScalar(1) );  
                                cvSetIdentity(kalman[i]->process_noise_cov,cvRealScalar(1e-5));  
                                cvSetIdentity(kalman[i]->measurement_noise_cov,cvRealScalar(1e-1));  
                                cvSetIdentity(kalman[i]->error_cov_post,cvRealScalar(1));  
                                //initialize post state of kalman filter at random  
                                cvRandArr(&rng,kalman[i]->state_post,CV_RAND_UNI,cvRealScalar(0),cvRealScalar(winHeight>winWidth?winWidth:winHeight));
                                break;
                            }
                        }
                    }
                }
            }
            //empty the obj_status array for 30 frames not upload's obj
            for(int i=0;i<20;i++){
                if((frame_number - obj_status[i][1] > 60) && obj_status[i][0]!=-1){ 
                    obj_status[i][0] = -1;
                    cvReleaseKalman(&kalman[i]);
                }
            }

            //2.kalman prediction
            
            for (int i=0 ; i<20; i++){
                bool predict=false;
                if(obj_status[i][0] != -1){
                    const CvMat* prediction=cvKalmanPredict(kalman[i],0);  
                    CvPoint predict_pt=cvPoint((int)prediction->data.fl[0],(int)prediction->data.fl[1]);
                    if (obj_status[i][1] != frame_number)
                    {   
                        measurement[i]->data.fl[0]=(float)prediction->data.fl[0];  
                        measurement[i]->data.fl[1]=(float)prediction->data.fl[1];
                        obj_status[i][2] =(float)prediction->data.fl[0]; 
                        obj_status[i][3] =(float)prediction->data.fl[1];
                        predict=true;
                    } 
                    else
                    {
                        measurement[i]->data.fl[0]=obj_status[i][2];  
                        measurement[i]->data.fl[1]=obj_status[i][3]; 
                    }
                    cvKalmanCorrect( kalman[i], measurement[i] );
                    if(predict)
                        cvCircle(img,predict_pt,5,CV_RGB(255,255,255),3);
                    else
                        cvCircle(img,predict_pt,5,CV_RGB(0,255,0),3);

                    

                    cvShowImage("kalman", img);
                }

            }
            /*
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
                  
                cvShowImage("kalman", img);  
                
            }
            */
            char key=(char)cvWaitKey(30);  
            if (key==27){//esc     
                break;     
            } 
      	}

        cvReleaseImage(&img);  
          
        return 0;  
    }  
