#include<iostream>
#include <vector>
#include <list>
#include "opencv2/opencv.hpp"
#include "glog/logging.h"
#ifdef _MSC_VER
#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif // !GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif 
 int pickWindowBaseBg(const cv::Mat&img,const cv::Vec3b&bg, cv::Rect&rect,const unsigned int&diff=2)
{	 
	 if (img.empty())return -1;
	 cv::Mat mask = cv::Mat::zeros(img.size(), CV_8UC1); 
	 for (int r = 0; r < img.rows; r++)
	 {
		 for (int c = 0; c < img.cols; c++)
		 {
			 int b_d = img.at<cv::Vec3b>(r, c)[0] - bg[0];
			 int g_d = img.at<cv::Vec3b>(r, c)[1] - bg[1];
			 int r_d = img.at<cv::Vec3b>(r, c)[2] - bg[2];
			 if (abs(b_d) <= diff&& abs(g_d) <= diff&& abs(r_d) <= diff)
			 {
				 mask.ptr<uchar>(r)[c] = 255;
			 }
		 }
	 }
	 std::vector<std::vector<cv::Point>> contours;
	 cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	 if (contours.size()<1)return -1;
	 
	 int maxArea = 0; 
	 for (int i = 0; i < contours.size(); i++)
	 {
		 cv::Rect region = cv::boundingRect(contours[i]);
		 if (region.area()>maxArea)
		 {
			 maxArea = region.area();
			 rect = region;
		 };
	 }

	 cv::Mat sheepWin;
	 img(rect).copyTo(sheepWin);
	 mask = cv::Mat::ones(sheepWin.size(), CV_8UC1)*255;
	 for (int r = 0; r < sheepWin.rows; r++)
	 {
		 for (int c = 0; c < sheepWin.cols; c++)
		 {
			 int b_d = sheepWin.at<cv::Vec3b>(r, c)[0] - bg[0];
			 int g_d = sheepWin.at<cv::Vec3b>(r, c)[1] - bg[1];
			 int r_d = sheepWin.at<cv::Vec3b>(r, c)[2] - bg[2];
			 if (abs(b_d) <= diff && abs(g_d) <= diff && abs(r_d) <= diff)
			 {
				 mask.ptr<uchar>(r)[c] = 0;
			 }
		 }
	 }
	 contours.clear();
	 cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	 if (contours.size() < 1)return -1;
	 maxArea = 0;
	 cv::Rect centraRect;
	 for (int i = 0; i < contours.size(); i++)
	 {
		 cv::Rect region = cv::boundingRect(contours[i]);
		 if (region.area() > maxArea)
		 {
			 maxArea = region.area();
			 centraRect = region;
		 };
	 }
	 rect.x += centraRect.x;
	 rect.y += centraRect.y;
	 rect.width = centraRect.width;
	 rect.height = centraRect.height;
	 return 0;
}


 int analysis(const cv::Mat&img,const cv::Rect&rect, cv::Vec3b& fg, cv::Vec3b& bg, const unsigned int& diff = 2)
 {
	 cv::Mat sheepWin;
	 img(rect).copyTo(sheepWin);

	 cv::Mat frontMask = cv::Mat::zeros(sheepWin.size(), CV_8UC1);
	 cv::Mat behindMask = cv::Mat::zeros(sheepWin.size(), CV_8UC1);
	 for (int r = 0; r < sheepWin.rows; r++)
	 {
		 for (int c = 0; c < sheepWin.cols; c++)
		 {
			 int b_d = sheepWin.at<cv::Vec3b>(r, c)[0] - fg[0];
			 int g_d = sheepWin.at<cv::Vec3b>(r, c)[1] - fg[1];
			 int r_d = sheepWin.at<cv::Vec3b>(r, c)[2] - fg[2];
			 if (abs(b_d) <= diff && abs(g_d) <= diff && abs(r_d) <= diff)
			 {
				 frontMask.ptr<uchar>(r)[c] = 255;
			 }
			 b_d = sheepWin.at<cv::Vec3b>(r, c)[0] - bg[0];
			 g_d = sheepWin.at<cv::Vec3b>(r, c)[1] - bg[1];
			 r_d = sheepWin.at<cv::Vec3b>(r, c)[2] - bg[2];
			 if (abs(b_d) <= diff && abs(g_d) <= diff && abs(r_d) <= diff)
			 {
				 behindMask.ptr<uchar>(r)[c] = 255;
			 }
		 }
	 }
	 cv::imwrite("a.bmp", frontMask);
	 cv::imwrite("b.bmp", behindMask);

	 std::vector<std::vector<cv::Point>> contours;
	 cv::findContours(frontMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	 if (contours.size() < 1)return -1;

	 std::vector<cv::Rect>frontPatterns;
	 int minHeight = rect.width*10;
	 int minWidth =  rect.width*10;
	 int maxHeight = 0;
	 int maxWidth = 0;
	 for (int i = 0; i < contours.size(); i++)
	 {
		 cv::Rect region = cv::boundingRect(contours[i]);
		 if (region.width > 1 && region.height > 1 )
		 {
			 double f = 1. * region.height / region.width;
			 if (f>0.95 &&f<1.05)
			 {
				 if (minHeight > region.height)minHeight = region.height;
				 if (minWidth > region.width)minWidth = region.width;
				 if (maxHeight< region.height)maxHeight = region.height;
				 if (maxWidth <region.width)maxWidth = region.width;
				 frontPatterns.emplace_back(region);
			 }

		 }
	 }

	 int heightThre = 0.5 * 0.1 * (minHeight + maxHeight);
	 int widthThre = 0.5 * 0.1 * (minWidth + maxWidth);

	 contours.clear();
	 cv::findContours(behindMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); 
	 std::vector<cv::Rect>behindPatterns;
	 for (int i = 0; i < contours.size(); i++)
	 {
		 cv::Rect region = cv::boundingRect(contours[i]);
		 if (region.width > widthThre && region.height > heightThre)
		 {
			 std::vector<int> hull;
			 cv::convexHull(contours[i], hull, true, false); 
			 CHECK(hull.size() > 2); 
			 cv::Rect bbox = cv::boundingRect(contours[i]);
			 cv::Point a(bbox.x, bbox.y);
			 cv::Point b(bbox.x + bbox.width - 1, bbox.y);
			 cv::Point c(bbox.x, bbox.y + bbox.height - 1);;
			 cv::Point d(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1);
			 std::vector<cv::Point> hullPts(hull.size());
			 for (int j = 0; j < hull.size(); j++)
			 {
				 hullPts[j] = contours[i][hull[j]];
			 }
			 



			 cv::Mat planeTemp = cv::Mat::zeros(sheepWin.size(), CV_8UC1);
			 cv::fillPoly(planeTemp, hullPts, cv::Scalar(255));

		 }
		 behindPatterns.emplace_back(region);
	 }


	 return 0;
 }

int main()
{
	cv::Mat img = cv::imread("C:/Users/Administrator/Downloads/sheep.bmp");
	cv::Rect sheepWin;
	auto grassColor = cv::Vec3b(139, 254, 195);
	auto fgColor = cv::Vec3b(205,255,245);//front
	auto bgColor = cv::Vec3b(123,153,147);//behind
	if (pickWindowBaseBg(img, grassColor, sheepWin))
	{
		LOG(ERROR) << "pickWindowBaseBg";
	}

	analysis(img,sheepWin, fgColor, bgColor);


	return 0;
}