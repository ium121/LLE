// Low_Light.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char const* argv[])
{
	cv::Mat img = cv::imread("image.jpg");
	if (!img.data){
		cout << "Something Wrong";
		return -1;
	}
	namedWindow("Input Image", CV_WINDOW_AUTOSIZE);
	imshow("Input Image", img);

	double alpha = 0.05;
	Mat haze_img = Mat::zeros(img.rows, img.cols, CV_8UC3);
	Mat dark_channel = Mat::zeros(haze_img.size(), CV_8UC1);
	Mat op_img = Mat::zeros(haze_img.size(), CV_8UC3);
	vector<Mat> layers;

	for (int i = 0; i < img.rows; i++){
		for (int j = 0; j < img.cols; j++){
			haze_img.at<Vec3b>(i, j)[0] = 255 - img.at<Vec3b>(i, j)[0];
			haze_img.at<Vec3b>(i, j)[1] = 255 - img.at<Vec3b>(i, j)[1];
			haze_img.at<Vec3b>(i, j)[2] = 255 - img.at<Vec3b>(i, j)[2];
		}
	}

	Mat rgbmin = Mat::zeros(haze_img.rows, haze_img.cols, CV_8UC1);
	Mat MDCP;
	Vec3b intensity;

	for (int m = 0; m < haze_img.rows; m++)
	{
		for (int n = 0; n < haze_img.cols; n++)
		{
			intensity = haze_img.at<Vec3b>(m, n);
			rgbmin.at<uchar>(m, n) = min(min(intensity.val[0], intensity.val[1]), intensity.val[2]);
		}
	}
	medianBlur(rgbmin, MDCP, 1);
	dark_channel = MDCP;

	//namedWindow("Dark Channel", CV_WINDOW_AUTOSIZE);
	//imshow("Dark Channel", dark_channel);

	double minDC, maxDC;
	minMaxLoc(dark_channel, &minDC, &maxDC);
	int al = maxDC;

	double w = 0.95;
	Mat transmission = Mat::zeros(dark_channel.rows, dark_channel.cols, CV_8UC1);
	Scalar intens;

	for (int m = 0; m<dark_channel.rows; m++)
	{
		for (int n = 0; n<dark_channel.cols; n++)
		{
			intens = dark_channel.at<uchar>(m, n);
			transmission.at<uchar>(m, n) = (1 - w * intens.val[0] / al) * 255;
		}
	}

	double tmin = 0.1;
	double tmax;

	Scalar inttran;
	Vec3b intsrc;
	Mat dehazed = Mat::zeros(haze_img.rows, haze_img.cols, CV_8UC3);

	for (int i = 0; i<haze_img.rows; i++)
	{
		for (int j = 0; j<haze_img.cols; j++)
		{
			inttran = transmission.at<uchar>(i, j);
			intsrc = haze_img.at<Vec3b>(i, j);
			tmax = (inttran.val[0] / 255) < tmin ? tmin : (inttran.val[0] / 255);
			for (int k = 0; k<3; k++)
			{
				dehazed.at<Vec3b>(i, j)[k] = abs((intsrc.val[k] - al) / tmax + al) > 255 ? 255 : abs((intsrc.val[k] - al) / tmax + al);
			}
		}
	}

	for (int i = 0; i < dehazed.rows; i++){
		for (int j = 0; j < dehazed.cols; j++){
			op_img.at<Vec3b>(i, j)[0] = 255 - dehazed.at<Vec3b>(i, j)[0];
			op_img.at<Vec3b>(i, j)[1] = 255 - dehazed.at<Vec3b>(i, j)[1];
			op_img.at<Vec3b>(i, j)[2] = 255 - dehazed.at<Vec3b>(i, j)[2];
		}
	}

	namedWindow("De-Hazed Image", CV_WINDOW_AUTOSIZE);
	imshow("De-Hazed Image", dehazed);


	namedWindow("Output Image", CV_WINDOW_AUTOSIZE);
	imshow("Output Image", op_img);
	//imwrite("out1.jpg", op_img);
	waitKey(0);
	destroyAllWindows;

	return 0;
}
