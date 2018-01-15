/**
 * @file main-opencv.cpp
 * @date July 2014 
 * @brief An exemplative main file for the use of ViBe and OpenCV
 */
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string> 
#include <sstream>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include "opencv2/imgproc/imgproc.hpp"

#include "vibe-background-sequential.h"

using namespace cv;
using namespace std;

/** Function Headers */
void processVideo_NADI(char* videoFilename,bool saveBG,bool saveMK, bool saveFG);
void saveBGimages(bool buildBG, int frameNumber, Mat frame);
void saveMKimages(bool buildBG, int frameNumber, Mat frame);
void saveFGimages(bool buildBG, int frameNumber, Mat frame);

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
  namedWindow("Frame");
  namedWindow("Segmentation by ViBe");
  
  bool saveBG = false;
  bool saveMK = false;
  bool saveFG = false;
  char c;
  while((c=getopt(argc, argv, "bmf")) != -1)
  {
      switch(c)
      {
      case 'b':
          cout<<"save background images"<<endl;
          saveBG = true;
          break;
      case 'm':
          cout<<"save mask images"<<endl;
          saveMK = true;
          break;
      case 'f':
          cout<<"save foreground images"<<endl;
          saveFG = true;
          break;
      } 
  }

  processVideo_NADI(argv[argc-1],saveBG,saveMK , saveFG);

  /* Destroy GUI windows. */
  destroyAllWindows();
  return EXIT_SUCCESS;
}

/**
 * Processes the video. The code of ViBe is included here. 
 *
 * @param videoFilename  The name of the input video file. 
 */
void processVideo_NADI(char* videoFilename,bool saveBG,bool saveMK , bool saveFG)
{ 
  /* Create the capture object. */
  VideoCapture capture(videoFilename);

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
  writer.open("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, videoSize);

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
    //int32_t background_ptr = libvibeModel_Sequential_Segmentation_8u_C3R(model, frame.data, segmentationMap.data);
    //cout << background_ptr;
    Mat background_img = frame.clone();
    libvibeModel_Sequential_Segmentation_8u_C3R(model, frame.data, segmentationMap.data, background_img.data);
    libvibeModel_Sequential_Update_8u_C3R(model, frame.data, segmentationMap.data);

    /* Post-processes the segmentation map. This step is not compulsory. 
       Note that we strongly recommend to use post-processing filters, as they 
       always smooth the segmentation map. For example, the post-processing filter 
       used for the Change Detection dataset (see http://www.changedetection.net/ ) 
       is a 5x5 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3); /* 3x3 median filtering */

    /*build object frame*/
    Mat objframe, segmentationMap_inv, segmentationMap_inv_color, objframe_blue;
    frame.copyTo(objframe, segmentationMap);
    bitwise_not(segmentationMap, segmentationMap_inv, noArray());
    cvtColor(segmentationMap_inv, segmentationMap_inv_color, CV_GRAY2RGB);
    segmentationMap_inv_color.setTo(Scalar(255, 0, 0), segmentationMap_inv);
    add(segmentationMap_inv_color, objframe, objframe_blue, noArray(), -1);

    /* Shows the current frame and the segmentation map. */
    imshow("Frame", frame);
    imshow("Segmentation by ViBe", segmentationMap);//output ViBe image
    imshow("Object", objframe_blue);
    imshow("background", background_img);
    
    if (saveBG)
      saveBGimages(buildBG,frameNumber,background_img);
    if (saveMK)
      saveMKimages(buildBG,frameNumber,segmentationMap);
    if(saveFG)
      saveFGimages(buildBG,frameNumber,objframe_blue);

    
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

void saveBGimages(bool buildBG, int frameNumber, Mat frame){
  if (buildBG && (frameNumber % 1) == 0) { 
    stringstream ss1;
    ss1 <<"BG_"<< frameNumber << ".jpg";
    string str1 = ss1.str();
    imwrite(str1, frame);//save images
  }
}

void saveMKimages(bool buildBG, int frameNumber, Mat frame){
  if (buildBG && (frameNumber % 1) == 0) { 
    stringstream ss1;
    ss1 <<"MK_"<< frameNumber << ".jpg";
    string str1 = ss1.str();
    imwrite(str1, frame);//save images
  }
}

void saveFGimages(bool buildBG, int frameNumber, Mat frame){
  if (buildBG && (frameNumber % 1) == 0) { 
    stringstream ss1;
    ss1 <<"FG_"<< frameNumber << ".jpg";
    string str1 = ss1.str();
    imwrite(str1, frame);//save images
  }
}




