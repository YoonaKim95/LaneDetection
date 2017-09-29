//#include "opencv2/opencv.hpp"
//#include <cmath>
//#include "opencv/cv.hpp"
//#include "opencv/highgui.h"
//#include <iostream>
//#include <algorithm>
//#include <stdio.h>
//#include <queue>
//
//using namespace std;
//using namespace cv;
// 
//Mat FindLargestArea(Mat origin, Mat cannies);
//Mat nonedge_area(Mat src, float sky_rate, int window_size);
//Mat roadFilter(const Mat& src, double sigma, Mat mask);
//Mat roadFilter2(const Mat& src, double sigma, Mat mask);
//Mat FindRoad(Mat src); 
//
//int main() {
//	char title[100] = "curve.avi";
//	VideoCapture capture(title);
//
//	Mat frame, gray, canny, poly, Final;
//
//	int key, frameNum = 1, frame_rate = 30, leftKept = 0, rightKept = 0;
//	
//	// videoRead
//	while (1) {
//		if (!capture.read(frame))
//			break;
//
//		Final = FindRoad(frame);
//		imshow("Final", Final); 
//
//		char textfps[255];
//		sprintf(textfps, "FPS: %d", (int)capture.get(CV_CAP_PROP_FPS));
//		putText(frame, textfps, Point(10, 20), FONT_HERSHEY_COMPLEX_SMALL, 1, 255, 1);
//
//		//putText(frame, text, Point(10, 20), FONT_HERSHEY_COMPLEX_SMALL, 1, 255, 1);
//		imshow("frame", frame); 
//		 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//		
//		key = waitKey(frame_rate);
//		if (key == 32) {
//			if (frame_rate == 30)
//				frame_rate = 0;
//			else
//				frame_rate = 30;
//		}
//		else if (key == ']') {
//			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum + 90);
//			frameNum += 90;
//		}
//		else if (key == '[') {
//			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum - 90);
//			frameNum -= 90;
//		}
//		else if (key == 'd') {
//			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum + 30);
//			frameNum += 30;
//		}
//		else if (key == 'a') {
//			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum - 30);
//			frameNum -= 30;
//		}
//		else if (key == 27) {
//			break;
//		}
//		frameNum++;
//	}
//	return 0;
//}
// 
//Mat FindLargestArea(Mat origin, Mat cannies) {
//	Mat src;
//	Mat source;
//	int i = 0, count = 0;
//	int x = 0, y = 0;
//	int nBlue = 0, nGreen = 0, nRed = 0;
//
//	double maxcontour = 0;
//	/*
//	for (i = 0; i < 3; i++)
//	bgr[i] = 0;*/
//
//	vector<vector<Point>> contours;
//	vector<Vec4i>hierarchy;
//
//
//	src = origin.clone();
//
//	findContours(cannies, contours, hierarchy, 2, CV_CHAIN_APPROX_SIMPLE);
//
//	for (i = 0; i < contours.size(); i++){
//		if (contourArea(contours[i])>maxcontour){
//			maxcontour = contourArea(contours[i]);
//			count = i;
//		}
//	}
//	source = origin.clone();
//	cvtColor(source, source, CV_RGB2GRAY);
//	source = Scalar(0);
//	drawContours(source, contours, count, Scalar(255), CV_FILLED, 8, hierarchy);
// 
//	return source;
//}
// 
//Mat nonedge_area(Mat src, float sky_rate, int window_size) {
//	/*
//	Mat src :  원본 영상(에지처리후 -> 2 진화영상으로 변환된 영상이어야함.
//	float sky_rate : 하늘에 해당하는 비율 (ex/ 0.3 : 상위 30%를 무시한다)
//	int window_size : 윈도우의 크기 : 낮을수록 정밀하게 검색.
//	*/
//
//	int i, i2 = 0;
//	int j, j2 = 0;
//	int src_height, src_width;
//
//	src_height = src.rows;
//	src_width = src.cols;
//
//	Mat window;
//	Mat output(src_height, src_width, src.type(), Scalar(0));
//
//	float m_height = src_height * sky_rate;
//
//	for (i = m_height; i + window_size <= src_height; i = i + window_size / 5) {
//		if (i + window_size >= src_height)
//			i2 = src_height;
//		else
//			i2 = i + window_size;
//
//		for (j = 0; j + window_size <= src_width; j = j + window_size / 5) {
//			if (j + window_size >= src_width)
//				j2 = src_width;
//			else
//				j2 = j + window_size;
//
//			window = src(Range(i, i2), Range(j, j2));
//			if (sum(window) == Scalar(0)) // should be changed.
//				output(Range(i, i2), Range(j, j2)) = Scalar(255);
//		}
//	}
//	return output;
//
//}
//
//Mat roadFilter(const Mat& src, double sigma, Mat mask) {
//	/* In Lab Color space, Filtering only L's value with sigma*/
//
//	assert(src.type() == CV_8UC3);
//
//	Mat filter;
//
//	Scalar mean;
//	Scalar dev;
//
//	double mean_v[3];
//	double dev_v[3];
//	double sigma_v[3];
//
//	meanStdDev(src, mean, dev, mask);
//
//	for (int i = 0; i < 3; i++)
//		sigma_v[i] = (sigma*dev.val[i]);
//
//	for (int i = 0; i < 3; i++){
//		mean_v[i] = mean.val[i];
//		dev_v[i] = dev.val[i];
//	} 
//	if ((sigma_v[1] + sigma_v[2]) <= 20){
//		sigma_v[1] = 7;
//		sigma_v[2] = 10;
//	} 
//	inRange(src, Scalar(mean_v[0] - 70, mean_v[1] - sigma_v[1], mean_v[2] - sigma_v[2]), Scalar(255, mean_v[1] + sigma_v[1], mean_v[2] + sigma_v[2]), filter); //Threshold the image
//
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//
//	return filter;
//}
//
//Mat roadFilter2(const Mat& src, double sigma, Mat mask) { 
//	assert(src.type() == CV_8UC3);
//	Mat filter;
//
//	Scalar mean;
//	Scalar dev;
//
//	double mean_v[3];
//	double dev_v[3];
//	double sigma_v[3];
//
//	meanStdDev(src, mean, dev, mask);
//
//	for (int i = 0; i < 3; i++)
//		sigma_v[i] = (sigma*dev.val[i]);
//
//	for (int i = 0; i < 3; i++){
//		mean_v[i] = mean.val[i];
//		dev_v[i] = dev.val[i];
//	} 
//	inRange(src, Scalar(mean_v[0] - sigma_v[0], mean_v[1] - sigma_v[1], mean_v[2] - sigma_v[2]), Scalar(200, 200, 200), filter); //Threshold the image
//
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	erode(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//	dilate(filter, filter, getStructuringElement(MORPH_RECT, Size(10, 10)));
//
//	return filter;
//}
//
//Mat FindRoad(Mat src) {
//	erode(src, src, Mat());
//	dilate(src, src, Mat());
//
//	Mat back, canny, gray;
//	Mat box, box3, lab_back, filter, box4, Color_Mask;
//
//	/*bilateralFilter(src, back, 30, 60, 10);
//	cvtColor(back, gray, CV_RGB2GRAY);*/
//
//	cvtColor(src, gray, CV_RGB2GRAY);
//
//	Canny(gray, canny, 15, 25, 3);
//	box = nonedge_area(canny, 0.7, 20);
//	box3 = FindLargestArea(src, box); // this is the mask
//
//	//Input LAB Matrix && Largest Area's Mask.
//	cvtColor(src, lab_back, CV_BGR2Lab);
//	Scalar value = mean(lab_back, box3); // box3 = Mask,
//	filter = roadFilter(lab_back, 1.2, box3);
//	filter = filter > 128;
//
//	//Input BGR Matrix && Largest Area's Mask.
//	box4 = roadFilter2(src, 2.5, box3);
//	box4 = box4 > 128;
//
//	//AND MASK FILTER&&BOX4
//	bitwise_and(filter, box4, Color_Mask);
//	//imshow("f1", filter);
//	//imshow("box",box4);
//	return Color_Mask;
//}