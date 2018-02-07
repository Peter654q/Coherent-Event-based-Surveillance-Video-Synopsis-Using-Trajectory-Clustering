/**
 * @file main-opencv.cpp
 * @date July 2014 
 * @brief An exemplative main file for the use of ViBe and OpenCV
 */
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string.h> 
#include <sstream>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include "opencv2/imgproc/imgproc.hpp"

#include "vibe-background-sequential.h"

using namespace cv;
using namespace std;

/** Function Headers */
void processVideo(char* videoFilename,bool saveImages,bool showBB, bool saveTxt);
void saveBGimages(char * folder, bool buildBG, int frameNumber, Mat frame, Mat seg, Mat mor);
Mat doMorphological(Mat seg);

/**
 * Displays instructions on how to use this program.
 */

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use ViBe with OpenCV                            " << endl
    << "Usage:"                                                                     << endl
    << "./main-opencv <video filename>"                                             << endl
    << "for example: ./main-opencv video.avi"                                       << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * Main program. It shows how to use the grayscale version (C1R) and the RGB version (C3R). 
 */
int main(int argc, char* argv[])
{
  /* Print help information. */
  //help();

  /* Check for the input parameter correctness. 
  if (argc != 2) {
    cerr <<"Incorrect input" << endl;
    cerr <<"exiting..." << endl;
    return EXIT_FAILURE;
  }*/

  /* Create GUI windows. */
  //namedWindow("Frame");
  //namedWindow("Segmentation by ViBe");
	
	bool saveImages = false;
	bool showBB = false;
	bool saveTxt = false;
	char c;
	while((c=getopt(argc, argv, "srt")) != -1)
	{
		  switch(c)
		  {
		  case 's':
					cout<<"save object images"<<endl;
					saveImages = true;
		      break;
		  case 'r':
					cout<<"show bounding boxes"<<endl;
					showBB = true;
		      break;
		  case 't':
					cout<<"save txt files"<<endl;
					saveTxt = true;
		      break;
		  }	
	}

  processVideo(argv[argc-1],saveImages,showBB , saveTxt);

  /* Destroy GUI windows. */
  destroyAllWindows();
  return EXIT_SUCCESS;
}

/**
 * Processes the video. The code of ViBe is included here. 
 *
 * @param videoFilename  The name of the input video file. 
 */
void processVideo(char* videoFilename,bool saveImages,bool showBB , bool saveTxt)
{ 
  /* Create the capture object. */
  VideoCapture capture(videoFilename);
  const char * split = "."; 
  char * folder; 
  folder = strtok (videoFilename,split);

  if (!capture.isOpened()) {
    /* Error in opening the video input. */
    cerr << "Unable to open video file: " << videoFilename << endl;
    exit(EXIT_FAILURE);
  }

  /* Variables. */
  static int frameNumber = 1; /* The current frame number */
  Mat frame;                  /* Current frame. */
  Mat segmentationMap;        /* Will contain the segmentation map. This is the binary output map. */
  int keyboard = 0;           /* Input from keyboard. Used to stop the program. Enter 'q' to quit. */

  /* Model for ViBe. */
  vibeModel_Sequential_t *model = NULL; /* Model used by ViBe. */

  /* video writer. */
  VideoWriter writer;
  Size videoSize = Size((int)capture.get(CV_CAP_PROP_FRAME_WIDTH),(int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
  stringstream ss;
  ss << folder << "/output1.avi";
  string str = ss.str();
  writer.open(str, CV_FOURCC('M', 'J', 'P', 'G'), 30, videoSize);

  /*build the background model until frame 3000. */
  bool buildBG = false;
  int frameToRestart = 3000; 
  /* use for mean-shift tracking */
  Mat pre_frame;
  /* Read input data. ESC or 'q' for quitting. */
  while ((char)keyboard != 'q' && (char)keyboard != 27) {
    /* Read the current frame. */
    if (!capture.read(frame)) {
      cerr << "Unable to read next frame." << endl;
      cerr << "Exiting..." << endl;
      exit(EXIT_FAILURE);
    }

    /* Applying ViBe.
     * If you want to use the grayscale version of ViBe (which is much faster!):
     * (1) remplace C3R by C1R in this file.
     * (2) uncomment the next line (cvtColor).
     */
    /* cvtColor(frame, frame, CV_BGR2GRAY); */

    if (frameNumber == 1) {
      segmentationMap = Mat(frame.rows, frame.cols, CV_8UC1);
      model = (vibeModel_Sequential_t*)libvibeModel_Sequential_New();
      libvibeModel_Sequential_AllocInit_8u_C3R(model, frame.data, frame.cols, frame.rows);
    }
	
	if (!buildBG && frameNumber==frameToRestart){
    	capture.set(CV_CAP_PROP_POS_FRAMES,1); 
		buildBG = true;
		frameNumber=1;
	}
    /* ViBe: Segmentation and updating. */
    libvibeModel_Sequential_Segmentation_8u_C3R(model, frame.data, segmentationMap.data);
    libvibeModel_Sequential_Update_8u_C3R(model, frame.data, segmentationMap.data);

    /* Post-processes the segmentation map. This step is not compulsory. 
       Note that we strongly recommend to use post-processing filters, as they 
       always smooth the segmentation map. For example, the post-processing filter 
       used for the Change Detection dataset (see http://www.changedetection.net/ ) 
       is a 5x5 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3); /* 3x3 median filtering */
    

	Mat morphological;
    morphological = doMorphological(segmentationMap);//clean the noisies
	
	Mat labelImage;
	Mat stats, centroids;
	int nLabels=0;
	nLabels = connectedComponentsWithStats(morphological, labelImage, stats, centroids, 8, CV_32S);

	/*output object result
	vector<cv::Vec3b> colors(nLabels+1);
	colors[0] = Vec3b(0,0,0);
	for(int i = 1; i < nLabels; i++ ){
		colors[i] = Vec3b(rand()%256, rand()%256, rand()%256);
		if( stats.at<int>(i, cv::CC_STAT_AREA) < 200 )
			colors[i] = Vec3b(0,0,0); 
	}
	Mat objImage = Mat::zeros(morphological.size(), CV_8UC3);
	for( int y = 0; y < objImage.rows; y++ ){
		for( int x = 0; x < objImage.cols; x++ ){
			int label = labelImage.at<int>(y, x);
			CV_Assert(0 <= label && label <= nLabels);
			objImage.at<cv::Vec3b>(y, x) = colors[label];
		}
	}*/

    bool bool_obj=false;    

    Mat copyframe;//for imageROI
    frame.copyTo(copyframe);
	for(int label = 1; label < nLabels; ++label){ // 2 = nLabels
        //object's left, top, width, height
    	int left = stats.at<int>(label, CC_STAT_LEFT);
		int top = stats.at<int>(label, CC_STAT_TOP);
		int width = stats.at<int>(label, CC_STAT_WIDTH);
		int height = stats.at<int>(label, CC_STAT_HEIGHT);
        //area threshold = 450
		if (stats.at<int>(label,CC_STAT_AREA)>350){
            bool_obj=true;
            //draw rectangle
			if (showBB == true)
	    		rectangle(frame, Point(left-3, top-18), Point(left+width+3, top+height+3), Scalar(0,0,255), 3, 8, 0); // add bias for the disappear helmets
			if(buildBG && saveTxt){
		  		//output object jpg file
				Mat imageROI;		
				if(top>=15){
		        	imageROI = copyframe(Rect(left, top-15, width, height));
				}
				else{
					imageROI = copyframe(Rect(left, top, width, height));
				}
		        stringstream ss1;
		        ss1 << folder << "/obj_n/F" << frameNumber << "_o" << label << ".jpg";
		        string str1 = ss1.str();
				if (saveImages == true)
		        	imwrite(str1, imageROI);
					
		        //ouput object information txt file
			    
		        fstream fp;
		        stringstream ss2;
		        ss2 << folder << "/txt_n/F" << frameNumber << ".txt";
		        string str2 = ss2.str();
		        
		        fp.open(str2.c_str(), ios::out|ios::app);
		        fp << label << endl;
		        fp << left << endl;
		        fp << top << endl;
		        fp << width << endl;
		        fp << height << endl;
		        fp << stats.at<int>(label,cv::CC_STAT_AREA) << endl;
		        fp << centroids.at<double>(label, 0) << endl; 
			    fp << centroids.at<double>(label, 1) << endl << endl;
			}
        }//end if
	}//end for

    //video write
    if(buildBG && bool_obj){
    	writer.set(VIDEOWRITER_PROP_QUALITY,1);
        writer.write(frame);
    }

    /* Shows the current frame and the segmentation map. */
    //imshow("Frame", frame);
    //imshow("Segmentation by ViBe", segmentationMap);//output ViBe image
    //imshow("morphological_op", morphological);//after morphological operation
    //imshow("Label Image", objImage);
	
	
    saveBGimages(folder, buildBG, frameNumber, frame, segmentationMap, morphological);//save background images
	pre_frame = frame.clone();
    ++frameNumber;

    /* Gets the input from the keyboard. */

    keyboard = waitKey(1);

    if (keyboard==83 || keyboard==115){    
        while(true){
            char keyboard=(char)cvWaitKey(30);
            if (keyboard==83 || keyboard==115)
                break;
        }     
    }//press S or s can pause
  }

  /* Delete capture object. */
  capture.release();

  /* Frees the model. */
  libvibeModel_Sequential_Free(model);
}

void saveBGimages(char * folder, bool buildBG, int frameNumber, Mat frame, Mat seg, Mat mor){
	if (buildBG && (frameNumber % 500) == 0) { 
		//cout << "Frame number = " << frameNumber << endl;
		
		//output jpg files
		stringstream ss1;
        ss1 << folder << "/BG/"<< frameNumber << ".jpg";
        string str1 = ss1.str();
		imwrite(str1, frame);//save images
        /*
		stringstream ss2;
        ss2 << frameNumber << "SO.jpg";
        string str2 = ss2.str();
		imwrite(str2, seg);

		stringstream ss3;
        ss3 << frameNumber << "M.jpg";
        string str3 = ss3.str();
		imwrite(str3, mor);
        */
	}
}

Mat doMorphological(Mat seg){
    Mat mor;
    erode(seg,mor,Mat());
	erode(mor,mor,Mat());
	dilate(mor,mor,Mat());
	dilate(mor,mor,Mat());

	//fill up the yellow line
	for(int x=0;x<6;x++)
		dilate(mor,mor,Mat());
	for(int x=0;x<6;x++)
		erode(mor,mor,Mat());

    return mor;
}



