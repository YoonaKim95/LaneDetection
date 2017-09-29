#include "opencv2/opencv.hpp"
#include <cmath>
#include "opencv/cv.hpp"
#include "opencv/highgui.h"
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <queue>

using namespace std;
using namespace cv;

int interest_y = 300;  // mono.avi = 168  // school = 270 
int interest_x = 0;

void findandDrawContour(Mat& roi, char* windowName, int type);
Mat preprocess(Mat& frame);
void getMinMax(Mat& roi, double& min, double& max);

Mat preprocess(Mat& frame) {
	Mat contourCanny, matForContour;

	char contourWindow[20] = "Contour";

	int width = frame.cols; //width of ROI
	int height = frame.rows - interest_y; //height of ROI
	int subROIHeight = height / 16;  // Calculate the height of sub_ROIs

	matForContour = frame.clone();

	//VP standard
	circle(frame, Point(frame.cols / 2, frame.rows / 4), 5, Scalar(0, 0, 0), 3, LINE_AA);

	// Setting ROIs
	Point rec3_4_point(interest_x, interest_y + subROIHeight * 3);
	Rect rec3_4(rec3_4_point, Size(width, subROIHeight * 13));

	Mat grayFrame, afterInRange, afterCanny;
	double min, max;

	cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
	GaussianBlur(grayFrame, grayFrame, Size(3, 3), 3);

	Mat roi(grayFrame, rec3_4);

	/***************** inragne ********************/
	getMinMax(roi, min, max);
	inRange(roi, min, max, afterInRange);
	Canny(afterInRange, afterCanny, 150, 250);

	dilate(afterCanny, afterCanny, Mat(), Point(-1, -1), 8);
	erode(afterCanny, afterCanny, Mat(), Point(-1, -1), 8);
	dilate(afterCanny, afterCanny, Mat(), Point(-1, -1), 8);
	erode(afterCanny, afterCanny, Mat(), Point(-1, -1), 8);

	imshow("afterInrangeAndCanny", afterCanny);

	/********************************************/
	/***************** Canny ********************/

	Canny(matForContour(rec3_4), contourCanny, 100, 200);

	dilate(contourCanny, contourCanny, Mat(), Point(-1, -1), 5);
	erode(contourCanny, contourCanny, Mat(), Point(-1, -1), 5);
	dilate(contourCanny, contourCanny, Mat(), Point(-1, -1), 5);
	erode(contourCanny, contourCanny, Mat(), Point(-1, -1), 5);

	imshow("afterCanny", contourCanny);
	/********************************************/

	// Mat andOperation = inrange + canny 
	Mat andOperation = afterCanny & contourCanny;
	dilate(andOperation, andOperation, Mat(), Point(-1, -1), 4);
	erode(andOperation, andOperation, Mat(), Point(-1, -1), 4);
	dilate(andOperation, andOperation, Mat(), Point(-1, -1), 4);
	erode(andOperation, andOperation, Mat(), Point(-1, -1), 4);

	imshow("andOperation", andOperation);

	imshow("testThreeafterContour", andOperation);

	return andOperation;
}

void findandDrawContour(Mat &roi, char* windowName, int type) {
	vector<vector<Point> > contours;
	int k = 0, j = 0;

	int mode = RETR_EXTERNAL;
	int method = CHAIN_APPROX_NONE;
	vector<Vec4i> hierarchy;
	findContours(roi, contours, hierarchy, mode, method);
	cvtColor(roi, roi, COLOR_GRAY2BGR);

	if (contours.size() > 1) {
		vector<Rect> rect(contours.size());
		vector<Mat> matArr(contours.size());
		drawContours(roi, contours, -1, Scalar(255, 255, 255));

		for (int i = 0; i < contours.size(); i++) {
			Rect temp = boundingRect(Mat(contours[i]));
			rect[k] = temp;
			k++;
		}

		char name[10];
		for (int i = 0; i < k; i++)	{
			sprintf(name, "Mat%d", i);
			matArr[i] = Mat(roi, rect[i]);
			Mat Cur_Mat = matArr[i];
			cvtColor(Cur_Mat, Cur_Mat, CV_BGR2GRAY);

			int row = Cur_Mat.rows;
			int col = Cur_Mat.cols;
			vector<int> whiteCount(row);

			for (int y = 0; y < row; y++) {
				Mat row = Cur_Mat.row(y);
				whiteCount[y] = countNonZero(row);
			}

			float sum = 0.0, mean, standardDeviation = 0.0;

			for (int z = 0; z < row; z++)
				sum += whiteCount[z];

			mean = sum / row;

			for (int zz = 0; zz < row; zz++)
				standardDeviation += pow(whiteCount[zz] - mean, 2);

			float stdevOfWhite = sqrt(standardDeviation / row);

			if (type == 0) {
				if (stdevOfWhite >= 10 || mean >= 20 || Cur_Mat.cols * Cur_Mat.rows < 150 || Cur_Mat.cols * Cur_Mat.rows > 20000)
					matArr[i].setTo(0);
			}
		}
	}
}

Point findLineAndVP(Mat& white, Mat& frame, float& prev_Rslope, float& prev_Lslope, Point intersectionPoint, int& leftKept, int& rightKept) {
	Mat canny;
	Canny(white, canny, 150, 300, 3);

	int halfWidth = canny.cols / 2;

	/*************** divide into left and right ROIs ***************/
	Point leftRectPoint(0, 0);
	Rect leftRect(leftRectPoint, Size(halfWidth, white.rows));
	Mat left(canny, leftRect);

	Point rightRectPoint(halfWidth, 0);
	Rect rightRect(rightRectPoint, Size(halfWidth, white.rows));
	Mat right(canny, rightRect);

	imshow("left", left);
	imshow("right", right);
	/************************************************************/

	Point rec_point(0, interest_y + ((frame.rows - interest_y) / 16) * 3);

	// declaration of x,y variables used in lines.
	float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	float x3 = 0, x4 = 0, y3 = 0, y4 = 0;
	float countright = 0, countleft = 0;
	float a1 = 0, a2 = 0, a3 = 0, a4 = 0;
	float b1 = 0, b2 = (float)frame.rows, b3 = 0, b4 = (float)frame.rows;
	float Rslope, Lslope, rb, lb;
	vector<Vec4i> leftL;
	vector<Vec4i> rightL;

	// 20, 10, 140
	HoughLinesP(left, leftL, 1, CV_PI / 180, 20, 10, 15);
	HoughLinesP(right, rightL, 1, CV_PI / 180, 20, 10, 15);

	/****************************************** LEFT ******************************************/
	float prev_leftb = intersectionPoint.y - prev_Lslope * intersectionPoint.x;
	vector<Vec4i> leftLCandidates;

	//////////////////////////////// Check parallel lines & distance //////////////////////////////
	for (size_t i = 0; i < leftL.size(); i++) {
		Vec4i LToCompare = leftL[i];
		float slopeToCompare = ((float)LToCompare[3] - (float)LToCompare[1]) / ((float)LToCompare[2] - (float)LToCompare[0]);

		// CHECKPOINT1: slope check 
		if (slopeToCompare <= -0.3  && slopeToCompare >= -3) {
			for (size_t j = 0; j < leftL.size(); j++) {
				if (j > i) {
					Vec4i LToCompareWith = leftL[j];
					float slopeToCompareWith = (((float)LToCompareWith[3] - (float)LToCompareWith[1]) / ((float)LToCompareWith[2] - (float)LToCompareWith[0]));
					float candidateSlopeDiff = abs(slopeToCompare - slopeToCompareWith);

					// CHECKPOINT2: slope difference between two candidates 
					if (candidateSlopeDiff < 0.01) {
						float interceptDiff = abs(((LToCompare[1] + rec_point.y) - slopeToCompare * LToCompare[0]) - ((LToCompareWith[1] + rec_point.y) - slopeToCompareWith * LToCompareWith[0]));

						// CHECKPOINT3: intercept difference between two candidates 
						if (interceptDiff < 8) {
							leftLCandidates.push_back(LToCompare);
							leftLCandidates.push_back(LToCompareWith);
						}
					}
				}
			}
		}
	}


	//////////////////////////////////// choosing among candidates ////////////////////////////////////
	int leftMax = 0;

	if (leftLCandidates.size() == 0 && prev_Lslope != 0) {
		Lslope = prev_Lslope;
		lb = intersectionPoint.y - Lslope * intersectionPoint.x;
		leftKept++;
	}
	else if (leftKept > 3) {
		//	choosing one lane from the candidates
		for (size_t i = 0; i < leftLCandidates.size(); i++) {
			Vec4i l = leftLCandidates[i];
			if (l[2] > leftMax) {
				leftMax = l[2];
				x3 = (float)l[0];
				y3 = (float)l[1] + rec_point.y;
				x4 = (float)l[2];
				y4 = (float)l[3] + rec_point.y;
			}
		}
		leftKept = 0;
		Lslope = (y4 - y3) / (x4 - x3);
		lb = (y3)-Lslope * (x3);
		prev_Lslope = Lslope;
	}
	else { // there exists candidates and prev_slope was not kept for 5 times
		for (size_t i = 0; i < leftLCandidates.size(); i++) {
			Vec4i l = leftLCandidates[i];
			float slopeCandidate = (((float)l[3] - (float)l[1]) / ((float)l[2] - (float)l[0]));
			float bCandidate = (l[1] + rec_point.y) - slopeCandidate * l[0];
			// intercept compare 
			float slopeDiff = abs(slopeCandidate - prev_Lslope);
			float bDiff = abs(bCandidate - prev_leftb);

			if (prev_Lslope == 0 || (slopeDiff < 0.3 && bDiff < 65)) {
				if (l[2] > leftMax) {
					leftMax = l[2];
					x3 = (float)l[0];
					y3 = (float)l[1] + rec_point.y;
					x4 = (float)l[2];
					y4 = (float)l[3] + rec_point.y;
				}
			}
		}
		if (leftMax == 0 && prev_Lslope != 0) {
			Lslope = prev_Lslope;
			lb = intersectionPoint.y - Lslope * intersectionPoint.x;
			leftKept++;
		}
		else {
			Lslope = (y4 - y3) / (x4 - x3);
			lb = (y3)-Lslope * (x3);
			prev_Lslope = Lslope;
		}
	}

	/****************************************** RIGHT ******************************************/
	float prev_rightb = intersectionPoint.y - prev_Rslope * intersectionPoint.x;
	vector<Vec4i> rightLCandidates;

	//////////////////////////////// Check parallel lines & distance //////////////////////////////	
	for (size_t i = 0; i < rightL.size(); i++) {
		Vec4i LToCompare = rightL[i];
		float slopeToCompare = ((float)LToCompare[3] - (float)LToCompare[1]) / ((float)LToCompare[2] - (float)LToCompare[0]);

		// CHECKPOINT1: slope check 
		if (slopeToCompare >= 0.3 && slopeToCompare <= 3) {
			for (size_t j = 0; j < rightL.size(); j++) {
				if (j > i) {
					Vec4i LToCompareWith = rightL[j];
					float slopeToCompareWith = (((float)LToCompareWith[3] - (float)LToCompareWith[1]) / ((float)LToCompareWith[2] - (float)LToCompareWith[0]));
					float candidateSlopeDiff = abs(slopeToCompare - slopeToCompareWith);
					// CHECKPOINT2: slope difference between two candidates 
					// checking if there is a prallel line 
					if (candidateSlopeDiff  < 0.01) {
						float interceptToCompare = (LToCompare[1] + rec_point.y) - slopeToCompare * LToCompare[0];
						float interceptToCompareWith = (LToCompareWith[1] + rec_point.y) - slopeToCompareWith * LToCompareWith[0];
						float interceptDiff = abs(interceptToCompare - interceptToCompareWith);

						// CHECKPOINT3: intercept difference between two candidates 
						if (interceptDiff < 4) {
							rightLCandidates.push_back(LToCompare);
							rightLCandidates.push_back(LToCompareWith);
						}
					}
				}
			}
		}
	}

	//////////////////////////////////// choosing among candidates ////////////////////////////////////
	int rightMax = 0;
	if (rightLCandidates.size() == 0 && prev_Rslope != 0) {
		Rslope = prev_Rslope;
		rb = intersectionPoint.y - Rslope * intersectionPoint.x;
		rightKept++;
	}
	else if (rightKept >  3) {
		for (size_t i = 0; i < rightLCandidates.size(); i++) {
			Vec4i l = rightLCandidates[i];
			if (l[2] > rightMax) {
				rightMax = l[2];
				x1 = (float)l[0] + halfWidth;
				y1 = (float)l[1] + rec_point.y;
				x2 = (float)l[2] + halfWidth;
				y2 = (float)l[3] + rec_point.y;
			}
		}
		rightKept = 0;
		Rslope = (y2 - y1) / (x2 - x1);
		rb = (y1)-Rslope * (x1);
		prev_Rslope = Rslope;
	}
	else {
		for (size_t i = 0; i < rightLCandidates.size(); i++) {
			Vec4i l = rightLCandidates[i];
			float slopeCandidate = (((float)l[3] - (float)l[1]) / ((float)l[2] - (float)l[0]));
			float bCandidate = (l[1] + rec_point.y) - slopeCandidate * (l[0] + halfWidth);

			// intercept compare 
			float slopeDiff = abs(slopeCandidate - prev_Rslope);
			float bDiff = abs(bCandidate - prev_rightb);

			if (prev_Rslope == 0 || (slopeDiff < 0.3 && bDiff < 75)) {
				if (l[2] > rightMax) {
					rightMax = l[2];
					x1 = (float)l[0] + halfWidth;
					y1 = (float)l[1] + rec_point.y;
					x2 = (float)l[2] + halfWidth;
					y2 = (float)l[3] + rec_point.y;
				}
			}
		}
		if (rightMax == 0 && prev_Rslope != 0) {
			Rslope = prev_Rslope;
			rb = intersectionPoint.y - prev_Rslope * intersectionPoint.x;
			rightKept++;
		}
		else {
			Rslope = (y2 - y1) / (x2 - x1);
			rb = (y1)-Rslope * (x1);
			prev_Rslope = Rslope;
		}
	}

	/******************		INTERSECTION_POINT		****************************/
	a1 = (0 - rb) / Rslope;
	a2 = (frame.rows - rb) / Rslope;
	a3 = ((0 - lb) / Lslope);
	a4 = ((frame.rows - lb) / Lslope);

	float dataA[] = { (b2 - b1) / (a2 - a1), -1, (b4 - b3) / (a4 - a3), -1 };
	Mat A(2, 2, CV_32F, dataA);
	Mat invA;
	invert(A, invA);

	float dataB[] = { a1*(b2 - b1) / (a2 - a1) - b1, a3*(b4 - b3) / (a4 - a3) - b3 };
	Mat B(2, 1, CV_32F, dataB);
	//vanishing point.
	Mat X = invA*B;

	// left and right lanes + intersection point
	line(frame, Point((int)X.at<float>(0, 0), (int)X.at<float>(1, 0)), Point((int)a2, frame.rows), Scalar(255, 0, 155), 2); // right
	line(frame, Point((int)X.at<float>(0, 0), (int)X.at<float>(1, 0)), Point((int)a4, frame.rows), Scalar(255, 0, 155), 2); // left
	circle(frame, Point((int)X.at<float>(0, 0), (int)X.at<float>(1, 0)), 5, Scalar(255, 200, 20), 3, LINE_AA);

	return Point((int)X.at<float>(0, 0), (int)X.at<float>(1, 0));
}

void getMinMax(Mat& roi, double& min, double& max) {
	double meanVal = 0, stdDevVal = 0, maxPixelVal;
	Point maxPoint;
	Scalar mean, stdDev;

	// find min and max value and point
	minMaxLoc(roi, 0, &maxPixelVal, 0, &maxPoint);

	// Calculate mean and stdDev of ROI
	meanStdDev(roi, mean, stdDev);
	meanVal = mean.val[0];
	stdDevVal = stdDev.val[0];

	// calculate min value of inRange
	min = meanVal + 2 * stdDevVal;

	// max value = mean + stdDev
	max = meanVal + 4 * stdDevVal; // maxPixelVal;
}

int main() {
	char title[100] = "curve.AVI";
	VideoCapture capture(title);

	Mat frame, afterPreprocess;

	int key, frameNum = 1, frame_rate = 30, leftKept = 0, rightKept = 0;
	float prev_Rslope = 0, prev_Lslope = 0;
	Point prev_intersectionPoint(0, 0);


	// videoRead
	while (1) {
		if (!capture.read(frame))
			break;

		prev_intersectionPoint = findLineAndVP(preprocess(frame), frame, prev_Rslope, prev_Lslope, prev_intersectionPoint, leftKept, rightKept);

		char textfps[255];
		sprintf(textfps, "FPS: %d", (int)capture.get(CV_CAP_PROP_FPS));
		putText(frame, textfps, Point(10, 20), FONT_HERSHEY_COMPLEX_SMALL, 1, 255, 1);
		imshow("frame", frame);

		key = waitKey(frame_rate);
		if (key == 32) {
			if (frame_rate == 30)
				frame_rate = 0;
			else
				frame_rate = 30;
		}
		else if (key == ']') {
			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum + 90);
			frameNum += 90;
			prev_Rslope = 0, prev_Lslope = 0;
			leftKept = 0, rightKept = 0;
		}
		else if (key == '[') {
			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum - 90);
			frameNum -= 90;
			prev_Rslope = 0, prev_Lslope = 0;
			leftKept = 0, rightKept = 0;
		}
		else if (key == 'd') {
			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum + 30);
			frameNum += 30;
			prev_Rslope = 0, prev_Lslope = 0;
			leftKept = 0, rightKept = 0;
		}
		else if (key == 'a') {
			capture.set(CV_CAP_PROP_POS_FRAMES, frameNum - 30);
			frameNum -= 30;
			prev_Rslope = 0, prev_Lslope = 0;
			leftKept = 0, rightKept = 0;
		}
		else if (key == 27) {
			break;
		}
		frameNum++;
	}
	return 0;
}
