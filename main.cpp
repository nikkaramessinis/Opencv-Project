// opencvtry.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#define _CRT_SECURE_NO_WARNINGS
#include <sstream>
#include <string>
//#include <opencv\highgui.h>

using namespace std;
using namespace cv;

const int fps = 60;

int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;

const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morph Operations";

// This function gets called whenever a trackbar is changed.
void on_trackbar(int, void*) 
{
}

string intToString(int number) 
{ 
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars()
{
	namedWindow("trackbars", 0);
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);

	createTrackbar("H_MIN", "trackbars", &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", "trackbars", &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", "trackbars", &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", "trackbars", &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", "trackbars", &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", "trackbars", &V_MAX, V_MAX, on_trackbar);

}

void drawObject(int x, int y, Mat& frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}

/* Dilation
 * The value of the output pixel is the maximum value of all pixels in the neighborhood.In a binary image, a pixel is set to 1 if any of the neighboring pixels have the value 1.
 * Erosion
 *The value of the output pixel is the minimum value of all pixels in the neighborhood. In a binary image, a pixel is set to 0 if any of the neighboring pixels have the value 0.
 * Applying dilation and erosion.
 */
void morphOps(Mat& thresh)
{
	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	// Dilate with a larger element to make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);


}

void trackFilteredObject(int& x, int& y, Mat threshold, Mat& cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
int main()
{
	bool useMorphOps = true;
	bool trackObjects = true;
	enum View
	{
		thresholdView,
		HSVView,
		Raw
	};
	Mat cameraFeed;

	Mat HSV;

	Mat threshold;

	int x = 0, y = 0;
	
	VideoCapture capture(0);
	namedWindow("image", WINDOW_NORMAL);


	//create slider bars for HSV filtering
	createTrackbars();

	capture.set(CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	if (!capture.isOpened())
	{
		return -1;
	}

	char c;
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
    //all of our operations will be performed within this loop
	View view = Raw;
	while (capture.read(cameraFeed))
	{
		// convert frame from BGR TO HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

		// Filter HSV image between values and store filtered image to 
		// threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);


		if (useMorphOps)
		{
			morphOps(threshold);
		}



		//pass in thresholded frame to our object tracking function
	//this function will return the x and y coordinates of the
	//filtered object
		if (trackObjects)
		{
			trackFilteredObject(x, y, threshold, cameraFeed);
		}

		if (view == thresholdView)
		{
			imshow(windowName, threshold);
		}
		else if ( view == HSVView)
		{
			imshow(windowName, HSV);
		}
		else 
		{
			imshow(windowName, cameraFeed);
		}

		if (c = waitKey(1000 / fps))
		{
			if (c == 'n')
			{
				std::cout << "Exit " << std::endl;
				break;
			}
			if (c == 't')
			{
				view = thresholdView;
				std::cout << "Threshold " << std::endl;
				
			}
			if (c == 'r')
			{
				view = Raw;
				std::cout << "Threshold " << std::endl;

			}
			if (c == 'h')
			{
				view = HSVView;
				std::cout << "Threshold " << std::endl;

			}
		}
	}

	return 1;
}
