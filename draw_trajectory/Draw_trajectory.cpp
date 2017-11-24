#include <iostream>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

static int txt_start = 25;
static int txt_end = 1999;

void draw(Mat bg, int num, Point p1, Point p2);

int main(){

	Mat Background_ori = imread("BG.jpg");
	Mat Background = Background_ori.clone();
	
	int max_num = 50;

	for(int frame_number = txt_start+1; frame_number < txt_end+1; frame_number++){
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
				if(f_in){
					Background = Background_ori.clone();
					break;
				}
			}
		}

		fstream f_in_pre;
		stringstream ss_pre;
		ss_pre << "txt/F_out" << frame_number-1 << ".txt";
		string str_pre = ss_pre.str();
		f_in_pre.open(str_pre.c_str(), ios::in);
		if(!f_in_pre){
			while(true)
			{
				frame_number++;
				stringstream ss;
				ss << "txt/F_out" << frame_number << ".txt";
				string str = ss.str();
				f_in_pre.open(str.c_str(), ios::in);
				if(f_in){
					break;
				}
			}
		}
		
		/*read txt*/
		int trash;
		int count = 0;
		int number1[max_num];
		double point1x[max_num];
		double point1y[max_num];
		for(int i=0;i<max_num;i++)
			number1[i]=-1;
				
		while(f_in >> number1[count]){
			for(int i=0;i<5;i++)
				f_in >> trash;
			f_in >> point1x[count];
			f_in >> point1y[count];
			count++;
		}
		
		int number2[max_num];
		double point2x[max_num];
		double point2y[max_num];
		count = 0;
		for(int i=0;i<max_num;i++)
			number2[i]=-1;
			
		while(f_in_pre >> number2[count]){
			for(int i=0;i<5;i++)
				f_in_pre >> trash;
			f_in_pre >> point2x[count];
			f_in_pre >> point2y[count];
			count++;
		}

		/*compare*/
		int trajectory[max_num][2];//number, count
		double point_tra[max_num][4];//point1x, point1y, point2x, point2y
		for(int i=0; i<max_num; i++){//init
			trajectory[i][0] = (-1);
			trajectory[i][1] = 0; 			
			for(int j=0; j<4; j++){
				point_tra[i][j] = 0;
			}
		}

		count=0;
		while(number1[count]!=-1){
			int count2=0;
			bool match=true;
			while(number1[count]!=number2[count2]){
				if(number2[count2]==-1){
					match=false;
					break;
				}
				count2++;
			}
			if(match==true){
				trajectory[count2][0] = number1[count];
				trajectory[count2][1] = trajectory[count2][1] + 1;
				if(trajectory[count2][1]>1){
					point_tra[count2][0] = (point_tra[count2][0] + point1x[count] * (trajectory[count2][1]-1)) / trajectory[count2][1];
					point_tra[count2][1] = (point_tra[count2][1] + point1y[count] * (trajectory[count2][1]-1)) / trajectory[count2][1];
					point_tra[count2][2] = (point_tra[count2][2] + point2x[count2] * (trajectory[count2][1]-1)) / trajectory[count2][1];
					point_tra[count2][3] = (point_tra[count2][3] + point2y[count2] * (trajectory[count2][1]-1)) / trajectory[count2][1];
				}else{
					point_tra[count2][0] = point1x[count];
					point_tra[count2][1] = point1y[count];
					point_tra[count2][2] = point2x[count2];
					point_tra[count2][3] = point2y[count2];
				}
			}
			count++;
		}

		/*draw*/
		for(int i=0; i<max_num; i++){//init
			if(trajectory[i][0]!=-1)
			{
				draw(Background, trajectory[i][0], Point(point_tra[i][0], point_tra[i][1]), Point(point_tra[i][2], point_tra[i][3]));
			}
		}

		imshow("result", Background);
		waitKey(1);
	}
	waitKey(0);
	
	return 0;
}

void draw(Mat bg, int num, Point p1, Point p2){

	int color_b[9] = {0, 51, 102, 153, 204, 255, 165, 50, 100};
	int color_g[9] = {255, 204, 153, 102, 51, 0, 122, 100, 22};
	int color_r[9] = {153, 102, 204, 0, 255, 51, 50, 0, 200};
	
	line(bg, p1, p2, Scalar(color_b[num%9], color_g[num%9], color_r[num%9]), 2, CV_AA, 0);
}


















