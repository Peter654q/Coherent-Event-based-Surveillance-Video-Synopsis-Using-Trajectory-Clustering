#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp> 
#include <fstream>
#include <sstream> 
#include <iostream>  
using namespace std;  
using namespace cv;

int main(){

	fstream fin;
	fin.open("../kalman/kalman_trajectory.txt", ios::in);
	int tmp;
	int txt_frame;
	fin >> tmp;
	fin >> txt_frame;

	VideoCapture video("../00000.avi");
	int frame_count=1;
    if (!video.isOpened()){
        return -1;
    }
    
    int frame_obj[10000][20][5];
    for(int i=0; i<10000; i++){
    	for(int j=0; j<20; j++){
    		for(int k=0; k<5; k++){
    			frame_obj[i][j][k]=-1;
    		}
    	}
    }

    Size videoSize = Size((int)video.get(CV_CAP_PROP_FRAME_WIDTH),(int)video.get(CV_CAP_PROP_FRAME_HEIGHT));
    Mat videoFrame;
    bool txt_end = false;
    while(!txt_end){
        video >> videoFrame;
        if(videoFrame.empty()){
            break;
        }
      	
      	if(frame_count==txt_frame){
      		int tmp;
      		int obj_cnt=0;
      		int obj, x, y;
      		while(fin >> tmp){
      			if(tmp==(-2))
      				break;
      			obj = tmp;
      			fin >> x >> y;
      			
      			if(x>0 && y>0 && x<videoSize.width && y<videoSize.height){
      				int rect_w, rect_h;
      				if(videoSize.width-x > 200){
      					rect_w = 200;
      				}else{
      					rect_w = videoSize.width-x;
      				}
      				if(videoSize.height-y > 200){
      					rect_h = 200;
      				}else{
      					rect_h = videoSize.height-y;
      				}
      				if(x>(rect_w/2))
      					x = x-(rect_w/2);
      				else
      					x=0;
      				if(y>(rect_h/2))
      					y = y-(rect_h/2);
      				else
      					y=0;
      				//cout << rect_w << " " << rect_h << endl;
      				Mat obj_frame = videoFrame(Rect(x, y, rect_w, rect_h));
      				stringstream ss1;
		        	ss1 << "obj/F" << frame_count << "_o" << obj << ".jpg";
		        	string str1 = ss1.str();
		        	//imwrite(str1, obj_frame);
		        	frame_obj[frame_count][obj_cnt][0] = obj;
		        	frame_obj[frame_count][obj_cnt][1] = x;
		        	frame_obj[frame_count][obj_cnt][2] = y;
		        	frame_obj[frame_count][obj_cnt][3] = rect_w;
		        	frame_obj[frame_count][obj_cnt][4] = rect_h;
		        	obj_cnt++;
      			}
      		}
      		if(fin >> tmp)
      			txt_frame = tmp;
      		else
      			txt_end = true;
      	}
      	if(frame_count%100==0)
      		cout << frame_count <<endl;
      	frame_count++;
    }
    /*for(int j=0;j<20;j++){
    	for(int i=0;i<5;i++)
    		cout << frame_obj[25][j][i] << " ";
    	cout << endl;
    }*/

    int appear_obj[10][4];//10 object, [obj_number][start frame][end frame][now frame]
    int appear_obj_cnt = 0;
    int frame_end = 10000;
    int obj=0;//obj start from number 0
    
    for(int i=0;i<10;i++){
    	for(int j=0;j<4;j++)
    		appear_obj[i][j]=-1;
    }
    for(int frame=1;frame<frame_end;frame++){
    	Mat BG = imread("BG.jpg",CV_LOAD_IMAGE_UNCHANGED);
    	//new a obj every 5 frame
    	if(frame%30==1 && appear_obj_cnt<10){
    		appear_obj_cnt++;
    		int index=0;
    		while(true){
    			if(appear_obj[index][0]==(-1))
    				break;
    			else
    				index++;
    		}
    		//cout << frame << " " << appear_obj_cnt << endl;
    		int start=0;
    		int end=0;
    		bool found=false;
    		for(int i=0;i<10000;i++){
    			for(int j=0;j<20;j++){
    				if(frame_obj[i][j][0]==obj){
    					start = i;
    					found = true;
    				}
    			}
    			if(found)
    				break;	
    		}
    		for(int i=start;i<10000;i++){
    			for(int j=0;j<20;j++){
    				if(frame_obj[i][j][0]==obj){
    					end = i;
    				}
    			}	
    		}
    		appear_obj[index][0] = obj;
    		appear_obj[index][1] = start;
    		appear_obj[index][2] = end;
    		appear_obj[index][3] = start;
    		obj++;
    	}
    	//delete obj when object won't appear again
    	for(int i=0;i<10;i++){
    		if(appear_obj[i][0]!=-1 && appear_obj[i][2]==appear_obj[i][3]){
    			for(int j=0;j<4;j++){
    				appear_obj[i][j]=(-1);
    			}
    			appear_obj_cnt--;
    		}
    	}
    	//attach obj in frame
    	for(int i=0;i<10;i++){
    		if(appear_obj[i][0]!=(-1)){
    			//get x,y,rect_w, rect_h
    			int x, y, rect_w, rect_h;
    			for(int j=0;j<20;j++){
    				int obj_nowframe = appear_obj[i][3];
    				if(appear_obj[i][0] == frame_obj[obj_nowframe][j][0]){
    					x=frame_obj[obj_nowframe][j][1];
    					y=frame_obj[obj_nowframe][j][2];
    					rect_w=frame_obj[obj_nowframe][j][3];
    					rect_h=frame_obj[obj_nowframe][j][4];
    					break;
    				}
    			}
    			stringstream ss1;
		        ss1 << "obj/F" << appear_obj[i][3] << "_o" << appear_obj[i][0] << ".jpg";
		        string str1 = ss1.str();
		        if (FILE * file = fopen(str1.c_str(), "r"))
    			{
        			fclose(file);
        			Mat obj_img = imread(str1, CV_LOAD_IMAGE_UNCHANGED);
    				Mat imgROI = BG(Rect(x, y, rect_w, rect_h));
    				addWeighted(imgROI, 0.5, obj_img, 0.5, 0, imgROI);
    			}
    			
    			appear_obj[i][3] = appear_obj[i][3] + 1;
    		}
    	}
    	imshow("result", BG);
    	char key=(char)cvWaitKey(1);//s(S) to stop and start  
        if (key==83 || key==115){    
        	while(true){
        		char key=(char)cvWaitKey(30);
        		if (key==83 || key==115)
        			break;
        	}     
        }
        //cout << frame << " " << obj << endl;
        /*for(int i=0;i<10;i++){
        	cout << appear_obj[i][0] << " ";
        }
        cout << endl;*/
        cout << appear_obj_cnt << endl;
    }
    



    return 0;
}