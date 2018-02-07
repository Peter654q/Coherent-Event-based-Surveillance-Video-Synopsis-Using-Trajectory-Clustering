#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp> 
#include <fstream>
#include <sstream> 
#include <iostream>
#include <unistd.h>
#include <string.h>
using namespace std;  
using namespace cv;

int main(int argc, char* argv[]){

	bool saveVideo = false;
	char c;
	while((c=getopt(argc, argv, "v")) != -1)
	{
		  switch(c)
		  {
		  	case 'v':
					cout << "save video" << endl;
					saveVideo = true;
		      break;
		    default:
		    	break;
		  }	
	}

    VideoCapture video(argv[argc-1]);
    int frame_count=1;
    if (!video.isOpened()){
        return -1;
    }

	const char * split = "."; 
    char * folder;
    folder = strtok(argv[argc-1], split);

	fstream fin;
    stringstream ss1;
    ss1 << "../" << folder << "/kalman_trajectory.txt";
    string str1 = ss1.str();
	fin.open(str1.c_str(), ios::in);
	int tmp;
	int txt_frame;
	fin >> tmp;
	fin >> txt_frame;

    Size videoSize = Size((int)video.get(CV_CAP_PROP_FRAME_WIDTH),(int)video.get(CV_CAP_PROP_FRAME_HEIGHT));
    VideoWriter writer;
	if(saveVideo){
        stringstream ss2;
        ss2 << "../" << folder << "/output2.avi";
        string str2 = ss2.str();
		writer.open(str2, CV_FOURCC('M', 'J', 'P', 'G'), 30, videoSize);
	}

    int frame_obj[10000][20][5];
    for(int i=0; i<10000; i++){
    	for(int j=0; j<20; j++){
    		for(int k=0; k<5; k++){
    			frame_obj[i][j][k]=-1;
    		}
    	}
    }

    Mat videoFrame;
    bool txt_end = false;
    int last_obj=0;
    while(!txt_end){
        video >> videoFrame;
        if(videoFrame.empty()){
            break;
        }
      	
      	if(frame_count==txt_frame){
      		int tmp;
      		int obj_cnt=0;
      		int obj, x, y, rect_w, rect_h;//center=(x,y)
      		while(fin >> tmp){
      			if(tmp==(-2))
      				break;
      			obj = tmp;
      			fin >> x >> y >> rect_w >> rect_h;
                rect_w = rect_w + 40;
                rect_h = rect_h + 40;
                x = x - (rect_w/2);
                y = y - (rect_h/2);
      			if(x+rect_w>videoSize.width)
                    rect_w = videoSize.width - x;
                if(y+rect_h>videoSize.height)
                    rect_h = videoSize.height - y;
                if(x<0)
                	x=0;
                if(y<0)
                	y=0;
                if(x>videoSize.width || y>videoSize.height || rect_w<=0 || rect_h<=0){
                	x = videoSize.width-20;
                	y = videoSize.height-20;
                	rect_w = 20;
                	rect_h = 20;
                }
  				Mat obj_frame = videoFrame(Rect(x, y, rect_w, rect_h));
  				stringstream ss3;
	        	ss3 << "../" << folder << "/obj/F" << frame_count << "_o" << obj << ".jpg";
	        	string str3 = ss3.str();
	        	imwrite(str3, obj_frame);
	        	frame_obj[frame_count][obj_cnt][0] = obj;
	        	frame_obj[frame_count][obj_cnt][1] = x;
	        	frame_obj[frame_count][obj_cnt][2] = y;
	        	frame_obj[frame_count][obj_cnt][3] = rect_w;
	        	frame_obj[frame_count][obj_cnt][4] = rect_h;
	        	obj_cnt++;
	        	last_obj = obj;
      			
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
    bool end = false;
    for(int i=0;i<10;i++){
    	for(int j=0;j<4;j++)
    		appear_obj[i][j]=-1;
    }
    for(int frame=1;frame<frame_end;frame++){
        int BG_number=500;
        BG_number = (frame/500)*500;
        if(BG_number==0)
            BG_number=500;
        stringstream ss4;
        ss4 << "../" << folder << "/BG/" << BG_number << ".jpg";
        string str4 = ss4.str();
    	Mat BG = imread(str4.c_str(), CV_LOAD_IMAGE_UNCHANGED);
    	//new a obj every 30 frame
    	if(frame%30==1 && appear_obj_cnt<10){
    		int index=0;
    		while(true){
    			if(appear_obj[index][0]==(-1))
    				break;
    			else
    				index++;
    		}

    		int start=0;
    		int end=0;
    		bool found=false;
    		while(start==0 && obj<=last_obj){
    			for(int i=0;i<10000;i++){
	    			for(int j=0;j<20;j++){
	    				if(frame_obj[i][j][0]==obj){
	    					start = i;
	    					found = true;
	    				}
	    			}
	    			if(found){
	    				appear_obj_cnt++;
	    				break;	
	    			}
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
    	}
    	//delete obj when object won't appear again
    	for(int i=0;i<10;i++){
    		if(appear_obj[i][0]!=-1 && appear_obj[i][3]>=appear_obj[i][2]){
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
    			int x=0;
    			int y=0;
    			int rect_w=0;
    			int rect_h=0;
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
    			stringstream ss5;
		        ss5 << "../" << folder << "/obj/F" << appear_obj[i][3] << "_o" << appear_obj[i][0] << ".jpg";
		        string str5 = ss5.str();
		        if (FILE * file = fopen(str5.c_str(), "r"))
    			{
        			fclose(file);
        			Mat obj_img = imread(str5, CV_LOAD_IMAGE_UNCHANGED);
        			int second = appear_obj[i][3]/30;
        			int minute = second/60;
        			second = second%60;
        			stringstream ss;
        			ss << appear_obj[i][0] << " " << minute << ":" << second;
        			string str = ss.str();
        			putText(obj_img, string(str), Point(0,20), 0, 0.5, Scalar(0,255,0),2);
    				Mat imgROI = BG(Rect(x, y, rect_w, rect_h));
    				addWeighted(imgROI, 0.5, obj_img, 0.5, 0, imgROI);
    			}
    			appear_obj[i][3] = appear_obj[i][3] + 1;
    		}
    		if(obj>=last_obj && appear_obj_cnt==0)
    			end=true;
    	}
    	imshow("result", BG);
    	if(saveVideo){
    		//writer.set(VIDEOWRITER_PROP_QUALITY,1);
        	writer.write(BG);
    	}
    	char key=(char)cvWaitKey(1);//s(S) to stop and start  
        if (key==83 || key==115){    
        	while(true){
        		char key=(char)cvWaitKey(30);
        		if (key==83 || key==115)
        			break;
        	}     
        }
        if(end)
        	break;
    }
    return 0;
}