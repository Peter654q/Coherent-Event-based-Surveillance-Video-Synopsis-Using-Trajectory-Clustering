#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp> 
#include <fstream>
#include <sstream> 
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <queue>
#include "object_tracking.h"
#include "kalman.h"
using namespace std;  
using namespace cv;

int main(int argc, char* argv[]){

    
	bool saveVideo = false;
    double transp = 0.7;//transparency
    double delay_time = 1.0;
	char c;
	while((c=getopt(argc, argv, "vtsn")) != -1)
	{
		  switch(c)
		  {
		  	case 'v':
					cout << "save video" << endl;
					saveVideo = true;
                break;
            case 't':
                    cout << "input your transparency(double) of object(max to 1.0, default:0.7)" << endl;
                    cin >> transp;
                break;
            case 's':
                    cout << "input your delay time(double) for each object(default:1 sec)" << endl;
                    cin  >> delay_time; 
                break;
            case 'n':
                    cout << "Open night mode!" << endl;
                    delay_time = 2;
                break;
		    default:
		    	break;
		  }	
	}

    

    VideoCapture video(argv[argc-1]);
    if (!video.isOpened()){
        return -1;
    }
    
    object_tracking(argc, argv);
    cout << "object_tracking done!" << endl;
    kalman(argc, argv);
    cout << "kalman tracking done!" << endl;
	
    const char * split = "."; 
    char * folder;
    folder = strtok(argv[argc-1], split);

    Size videoSize = Size((int)video.get(CV_CAP_PROP_FRAME_WIDTH),(int)video.get(CV_CAP_PROP_FRAME_HEIGHT));
    VideoWriter writer;
    stringstream ss;
    if(saveVideo){
        ss << "../" << folder << "/output2.avi";
        string str2 = ss.str();
        writer.open(str2, CV_FOURCC('M', 'J', 'P', 'G'), 30, videoSize);
    }

//make obj_txt file///////////////////////////////////[frame, x ,y, width, height]/////////////////
    fstream fin;
    ss.str("");
    ss << "../" << folder << "/kalman_trajectory.txt";
    string str1 = ss.str();
	fin.open(str1.c_str(), ios::in);
	int tmp;
	int txt_frame;
	fin >> tmp;

    int obj_maxnum=0;//for next step
    while(fin >> tmp){
        txt_frame = tmp;
        while(fin >> tmp){
            if(tmp==-2)
                break;
            int obj_num = tmp;
            if(obj_num>obj_maxnum)
                obj_maxnum = obj_num;
            fstream fobj_out;
            ss.str("");
            ss << "../" << folder << "/obj_txt/o" << obj_num << ".txt";
            string str3 = ss.str();
            fobj_out.open(str3.c_str(), ios::out|ios::app);
            fobj_out << txt_frame << " ";
            for(int i =0; i<4;i++){
                fin >> tmp;
                fobj_out << tmp << " ";
            }
            fobj_out << endl;
        }
    }
    fin.close();

//modify the obj's bounding box and clean some short trajectory//////////////////////////////////////////////////////////////////////////////////////////////////////
    const float pad_m=0.99;
    const float pad_p=1.20;
    int obj_maxframe=0;
    for(int obj_count=0; obj_count<=obj_maxnum; obj_count++){
        fstream fobj;
        ss.str("");
        ss << "../" << folder << "/obj_txt/o" << obj_count << ".txt";
        string str4 = ss.str();
        fobj.open(str4.c_str(), ios::in);
        if(fobj){
            int deltax, deltay;
            int maxw=0;
            int maxh=0;
            int max2w=0;
            int max2h=0;
            int tmp;
            int x, y, width, height;
            int maxx=0;
            int maxy=0;
            int minx=10000;
            int miny=10000;
            while(fobj >> tmp){
                fobj >> x >> y >> width >> height;
                if(x>maxx)
                    maxx = x;
                if(y>maxy)
                    maxy = y;
                if(x<minx)
                    minx = x;
                if(y<miny)
                    miny = y;
                if(width*height>maxw*maxh){
                    max2w = maxw;
                    max2h = maxh;
                    maxw = width;
                    maxh = height;
                }
            }
            //cout << obj_count << " " << maxx << " " << maxy << " " << minx << " " << miny << endl;
            deltax = maxx - minx;
            deltay = maxy - miny;
            if(deltax<200 && deltay<200){
                string rm = "rm " + str4;
                system(rm.c_str());//delete noise and short trajectory
            }
            fobj.close();
            fobj.open(str4.c_str(), ios::in);
            if(fobj){
                fstream fout;
                ss.str("");
                ss << "../" << folder << "/obj_txt/obj_" << obj_count << ".txt";
                string str5 = ss.str();
                fout.open(str5.c_str(), ios::out|ios::app);
                int noww = max2w*2;
                int nowh = max2h*2;
                while(fobj >> tmp){
                    fobj >> x >> y >> width >> height;
                    if(width*2<noww && height*2<nowh){
                        noww = noww * pad_m;
                        nowh = nowh * pad_m;
                    }else if(width*2>noww && height*2>nowh){
                        noww = noww * pad_p;
                        nowh = nowh * pad_p;
                        //noww = width*2;
                        //nowh = height*2;
                    }
                    fout << tmp << " " << x << " " << y << " " << noww << " " << nowh << endl;
                    //for make trajectory2.txt
                    fstream fout2;
                    ss.str("");
                    ss << "../" << folder << "/obj_txt/frame_" << tmp << ".txt";
                    string str6 = ss.str();
                    fout2.open(str6.c_str(), ios::out|ios::app);
                    fout2 << obj_count << " " << x << " " << y << " " << noww << " " << nowh << endl;
                    if(tmp>obj_maxframe)
                        obj_maxframe=tmp;
                    fout2.close();
                }
                fout.close();
                fobj.close();
                string rm = "rm " + str4;
                system(rm.c_str());
            }

        }
    }

//make kalman_trajectory2.txt for making jpg file////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //fout
    fstream fout;
    ss.str("");
    ss << "../" << folder << "/kalman_trajectory2.txt";
    string str7 = ss.str();
    fout.open(str7.c_str(), ios::out);
    for(int frame=1;frame<=obj_maxframe;frame++){
        //fin
        ss.str("");
        ss << "../" << folder << "/obj_txt/frame_" << frame << ".txt";
        string str8 = ss.str();
        fin.open(str8.c_str(), ios::in);
        int x, y, width, height;
        if(fin){
            fout << "-2 " << frame << endl;
            while(fin >> tmp){
                fin >> x >> y >> width >> height;
                fout << tmp << " " << x << " " << y << " "  << width << " " << height << endl;
            }
            fin.close();
            //string rm = "rm " + str8;
            //system(rm.c_str());//delete frame_XXXX.txt(slow)
        }
    }
    fout.close();

//make jpg file////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Mat videoFrame;
    bool txt_end = false;
    int frame_count=1;
    //fin
    ss.str("");
    ss << "../" << folder << "/kalman_trajectory2.txt";
    string str9 = ss.str();
    fin.open(str9.c_str(), ios::in);
    fin >> tmp;
    fin >> txt_frame;

    while(!txt_end){
        video >> videoFrame;
        if(videoFrame.empty()){
            break;
        }
        
        if(frame_count==txt_frame){
            int tmp;
            int obj, x, y, rect_w, rect_h;//center=(x,y)
            while(fin >> tmp){
                if(tmp==(-2))
                    break;
                obj = tmp;
                fin >> x >> y >> rect_w >> rect_h;
                rect_w = rect_w + 40;
                rect_h = rect_h + 40;//padding for not cutting obj
                x = x - (rect_w/2);//change(x,y) from center to leftt top corner
                y = y - (rect_h/2);
                if(x<0)
                    x=0;
                if(y<0)
                    y=0;
                if(x+rect_w>videoSize.width)
                    rect_w = videoSize.width - x;
                if(y+rect_h>videoSize.height)
                    rect_h = videoSize.height - y;
                if(x>videoSize.width || y>videoSize.height || rect_w<=0 || rect_h<=0){
                    x = videoSize.width-20;
                    y = videoSize.height-20;
                    rect_w = 20;
                    rect_h = 20;
                }
                Mat obj_frame = videoFrame(Rect(x, y, rect_w, rect_h));
                ss.str("");
                ss << "../" << folder << "/obj_t/F" << frame_count << "_o" << obj << ".jpg";
                string str10 = ss.str();
                
                imwrite(str10, obj_frame);
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
    fin.close();

//attach obj jpg to make video2///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int appear_obj[10][4];//10 object, [obj_number][start frame][end frame][now index]
    int frame_obj[10][3600][5];//10 object, 2 min frame buffer, [frame number][x][y][width][height]
    int appear_obj_cnt = 0;
    int frame_end = 10000;
    int obj=0;//obj start from number 0
    bool end = false;
    int previous_x = 0;
    int previous_y = 0;
    queue<int> queue;//for object number
    for(int i=0;i<10;i++){
        for(int j=0;j<4;j++)
            appear_obj[i][j]=-1;
    }
    for(int frame=1;frame<frame_end;frame++){
        int BG_number=500;
        BG_number = (frame/500)*500;
        if(BG_number==0)
            BG_number=500;
        ss.str("");
        ss << "../" << folder << "/BG/" << BG_number << ".jpg";
        string str11 = ss.str();
        Mat BG = imread(str11.c_str(), CV_LOAD_IMAGE_UNCHANGED);
        //new a obj every 30 frame
        int delay = delay_time*30;
        if(frame%delay==1 && appear_obj_cnt<10){
            int index=0;
            int newobj_num=-1;
            while(true){
                if(appear_obj[index][0]==(-1))
                    break;
                else
                    index++;
            }
            bool found=false;
            int tmp;
            int frame, x, y, rect_w, rect_h;//center=(x,y)
            int count=0;
            bool newobj_from_queue = false;
            string str14;
            if(obj>=obj_maxnum){//for not staying in the queue for deadlock in the end
                previous_x = -999;
                previous_y = -999;
                delay = 2;
            }
            if(!queue.empty()){
                int queue_newobj = queue.front();
                ss.str("");
                ss << "../" << folder << "/obj_txt/obj_" << queue_newobj << ".txt";
                str14 = ss.str();
                fin.close();
                fin.open(str14.c_str(), ios::in);
                fin >> tmp >> x >> y;
                cout << x << "," << y << " " << previous_x << "," << previous_y << endl;
                if(sqrt((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y)) > 500){//threshold=400, 
                    cout << sqrt((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y)) << endl;
                    queue.pop();
                    newobj_from_queue = true;
                    previous_x = x;
                    previous_y = y;
                    newobj_num = queue_newobj;
                    cout << "new object from queue!" << newobj_num << endl;
                }
            }
            if(!newobj_from_queue && obj<=obj_maxnum){
                while(!found){
                    ss.str("");
                    ss << "../" << folder << "/obj_txt/obj_" << obj << ".txt";
                    str14 = ss.str();
                    fin.close();
                    fin.open(str14.c_str(), ios::in);
                    if(fin){
                        fin >> tmp >> x >> y;
                        if(sqrt((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y)) < 500){//threshold=400,bmay need bigger 
                            queue.push(obj);
                            obj++;
                        }else{
                            found = true;
                            previous_x = x;
                            previous_y = y;
                            newobj_num = obj;
                            obj++;
                        }  
                    }else{
                        obj++;
                    }
                    if(obj>=obj_maxnum)
                        break;
                }
            }
            if(newobj_num!=-1){
                fin.close();
                fin.open(str14.c_str(), ios::in);
                while(fin >> tmp){
                    frame = tmp;
                    fin >> x >> y >> rect_w >> rect_h;
                    rect_w = rect_w + 40;
                    rect_h = rect_h + 40;//padding for not cutting obj
                    x = x - (rect_w/2);//change(x,y) from center to leftt top corner
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
                    frame_obj[index][count][0] = frame;
                    frame_obj[index][count][1] = x;
                    frame_obj[index][count][2] = y;
                    frame_obj[index][count][3] = rect_w;
                    frame_obj[index][count][4] = rect_h;
                    count++;
                }
                appear_obj[index][0] = newobj_num;
                appear_obj[index][1] = frame_obj[index][0][0];
                appear_obj[index][2] = frame_obj[index][count-1][0];
                appear_obj[index][3] = 0;
                appear_obj_cnt++;
                fin.close(); 
            }
            
        }
        //delete obj when object won't appear again
        for(int i=0;i<10;i++){
            if(appear_obj[i][0]!=-1 && frame_obj[i][appear_obj[i][3]][0]>=appear_obj[i][2]){
                for(int j=0;j<4;j++){
                    appear_obj[i][j]=(-1);
                }
                for(int j=0;j<3600;j++){
                    for(int k=0;k<5;k++){
                        frame_obj[i][j][k]=(-1);
                    }
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
                int obj_nowframe = appear_obj[i][3];
               
                x=frame_obj[i][obj_nowframe][1];
                y=frame_obj[i][obj_nowframe][2];
                rect_w=frame_obj[i][obj_nowframe][3];
                rect_h=frame_obj[i][obj_nowframe][4];
                       
                ss.str("");
                ss << "../" << folder << "/obj_t/F" << frame_obj[i][obj_nowframe][0] << "_o" << appear_obj[i][0] << ".jpg";
                string str13 = ss.str();
                if (FILE * file = fopen(str13.c_str(), "r"))
                {
                    fclose(file);
                    Mat obj_img = imread(str13, CV_LOAD_IMAGE_UNCHANGED);
                    double transparency[3] = {0.3, 0.5, transp};
                    double windowsize[3] = {1.0, 0.75, 0.5};
                    for(int j=0;j<3;j++){
                        Mat obj_img_roi = obj_img(Rect((1-windowsize[j])/2*obj_img.cols, (1-windowsize[j])/2*obj_img.rows, obj_img.cols*windowsize[j], obj_img.rows*windowsize[j]));
                        //rectangle(obj_img_roi, Point(0, 0), Point(obj_img_roi.cols-1, obj_img_roi.rows-1), Scalar(255, 0, 0), 2, 8, 0);
                        //Mat BG_roi = BG(Rect(x, y, rect_w, rect_h));
                        Mat BG_roi = BG(Rect(x + (1-windowsize[j])/2*obj_img.cols, y + (1-windowsize[j])/2*obj_img.rows, obj_img.cols*windowsize[j], obj_img.rows*windowsize[j]));
                        addWeighted(obj_img_roi, transparency[j], BG_roi, 1-transparency[j], 0, BG_roi);
                    }
                    
                    int second = frame_obj[i][obj_nowframe][0]/30;
                    int minute = second/60;
                    second = second%60;
                    ss.str("");
                    ss << setw(2) << setfill('0') << minute << ":" << setw(2) << setfill('0') << second;
                    string str = ss.str();
                    putText(BG, string(str), Point(x+(obj_img.cols/2)-35, y + (1-windowsize[2])/2*obj_img.rows+20), 0, 1, Scalar(0,255,0), 3);
                }
                appear_obj[i][3] = appear_obj[i][3] + 1;//index++
            }
            if(obj>=obj_maxnum && appear_obj_cnt==0 && queue.empty())
                end=true;
        }
        //imshow("result", BG);
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
}