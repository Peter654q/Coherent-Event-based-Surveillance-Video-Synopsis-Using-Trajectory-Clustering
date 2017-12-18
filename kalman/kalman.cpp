#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp> 
#include <fstream>
#include <sstream> 
#include <iostream>  
using namespace std;  
using namespace cv;

const int winHeight=1080;  
const int winWidth=1920;  
  
int main (void)  
{  
    Mat img=imread("BG.jpg",CV_LOAD_IMAGE_UNCHANGED);// Background img
    double obj_status[20][7] = {0};//Object array:obj_num, last_frame, (x,y) , live time ,width ,height
    for(int i=0;i<20;i++){
        obj_status[i][0] = -1;//-1 for no objs
        obj_status[i][4] = 0;
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
    

    int txt_start=25;
    int txt_end = 9334;
    
    fstream f_out;
    f_out.open("kalman_trajectory.txt", ios::out|ios::app);

    for(int frame_number = txt_start; frame_number < txt_end+1; frame_number++){
        fstream f_in;
        stringstream ss;
        ss << "../distance_tracking/out/F_out" << frame_number << ".txt";
        string str = ss.str();
        f_in.open(str.c_str(), ios::in);
        if(!f_in){
            while(true)
            {
                frame_number++;
                stringstream ss;
                ss << "../distance_tracking/out/F_out" << frame_number << ".txt";
                string str = ss.str();
                f_in.open(str.c_str(), ios::in);
                if(f_in)
                	img=imread("BG.jpg",CV_LOAD_IMAGE_UNCHANGED);
                    break;
            }
        }
        int trash;
        double tmp[6];//obj_num, last_frame, (x,y) , width , height
        while(f_in >> tmp[0]){
            for(int i=0;i<2;i++){
                f_in >> trash;
            }
            f_in >> tmp[4];
            f_in >> tmp[5];
            f_in >> trash;
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
                    obj_status[i][4]++;
                    obj_status[i][5]=tmp[4];
                    obj_status[i][6]=tmp[5];
                    compare = true;
                }
            }
            if(!compare){
                double min_dist = 999.9;
                int min_index = -1;
                double dist_thresh = 60.0;
                for (int j=0;j<20;j++){
                    if (obj_status[j][0] != -1){
                        double dist = pow(pow((obj_status[j][2] - tmp[2]), 2) + pow((obj_status[j][3]-tmp[3]), 2), 0.5);// L2 distance
                        if (dist < min_dist){
                            min_dist = dist;
                            min_index = j;
                        }
                    }
                }
                if (min_dist < dist_thresh){//match the old obj
                    //obj_status[min_index][0] = tmp[0];
                    obj_status[min_index][1] = tmp[1];
                    obj_status[min_index][2] = tmp[2];
                    obj_status[min_index][3] = tmp[3];
                    obj_status[min_index][4]++;
                    obj_status[min_index][5]=tmp[4];
                    obj_status[min_index][6]=tmp[5];
                }else{//new obj
                    for(int i=0;i<20;i++){
                        if(obj_status[i][0] == -1){
                            obj_status[i][0] = tmp[0];
                            obj_status[i][1] = tmp[1];
                            obj_status[i][2] = tmp[2];
                            obj_status[i][3] = tmp[3];
                            obj_status[i][4] = 1;
                            obj_status[i][5]=tmp[4];
                            obj_status[i][6]=tmp[5];
                            //1.kalman filter setup 
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
        f_out << "-2" << " " << frame_number << endl;// -2=framenumber
        for (int i=0 ; i<20; i++){
            bool predict=false;
            if(obj_status[i][0] != -1){
                const CvMat* prediction=cvKalmanPredict(kalman[i],0);  
                CvPoint predict_pt=cvPoint((int)prediction->data.fl[0],(int)prediction->data.fl[1]);
                if (obj_status[i][1] != frame_number)
                {   
                	//int ratio = (obj_status[i][3]/1080*0.2)+1.0;
                    measurement[i]->data.fl[0]=(float)prediction->data.fl[0];  
                    measurement[i]->data.fl[1]=(float)prediction->data.fl[1];
                    obj_status[i][2] =(float)prediction->data.fl[0]*1.1; 
                    obj_status[i][3] =(float)prediction->data.fl[1]*1.1;
                    predict=true;
                } 
                else
                {
                    measurement[i]->data.fl[0]=obj_status[i][2];  
                    measurement[i]->data.fl[1]=obj_status[i][3]; 
                }
                //3. update
                cvKalmanCorrect( kalman[i], measurement[i] );
                if(predict && obj_status[i][4]>20){
                    int obj_num = obj_status[i][0];
                    int b = (obj_num*23)%200+55;
                    int g = (obj_num*34)%200+55;
                    int r = (obj_num*45)%200+55;
                    circle(img,predict_pt, 5, CV_RGB(b ,g, r),3);
                    f_out << obj_num << " " << predict_pt.x << " " << predict_pt.y <<" " << obj_status[i][5]<< " " << obj_status[i][6]<< endl;//if predict, output obj_num = -1
                    //circle(img,predict_pt, 5, CV_RGB(255 ,255, 255),3);
                    //f_out << "-1" << " " << predict_pt.x << " " << predict_pt.y << endl;
                }
                else if(!predict){
                    int obj_num = obj_status[i][0];
                    int b = (obj_num*23)%200+55;
                    int g = (obj_num*34)%200+55;
                    int r = (obj_num*45)%200+55;
                    circle(img,predict_pt, 5, CV_RGB(b ,g, r),3);
                    f_out << obj_num << " " << predict_pt.x << " " << predict_pt.y <<" " << obj_status[i][5]<< " " << obj_status[i][6]<< endl;//if not predict, output obj_num
                }

                
                if (!img.empty()) {
    				//imshow("kalman", img);
				}
                char key=(char)cvWaitKey(1);//s(S) to stop and start  
                if (key==83 || key==115){    
                    while(true){
                        char key=(char)cvWaitKey(30);
                        if (key==83 || key==115)
                            break;
                    }     
                }
            }

        }

        

  	}

    //cvReleaseImage(&img);  
      
    return 0;  
}  
