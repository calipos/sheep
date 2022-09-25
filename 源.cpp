#include<iostream>
#include <vector>
#include <list>
#include <tuple>
#include "opencv2/opencv.hpp"
#include "glog/logging.h"
#include "common.h"
#ifdef _MSC_VER
#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif // !GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif 
#include "labelUpdata.h"
cv::Mat global;
Label lableblock;
extern int whichCovers(const cv::Mat& img, const std::vector<std::pair<cv::Rect, int>>& frontReture,
	const std::vector<std::tuple<cv::Rect, std::list<cv::Point>, int>>& coveredReture);
float getNearestPoint(const std::vector<cv::Point>& pts, const cv::Point& pt)
{
	CHECK(pts.size() > 1);
	std::vector<float>dists(pts.size());
	for (int i = 0; i < pts.size(); i++)
	{
		dists[i] = cv::norm(pts[i] - pt);
	}
	return  *std::min_element(dists.begin(), dists.end());;
}
int mergeTwoQuars(
	std::tuple<cv::Rect, cv::Rect, std::list<cv::Point>>& a, 
	std::tuple<cv::Rect, cv::Rect, std::list<cv::Point>>& b)
{
	cv::Rect bbox_a = std::get<0>(a);
	cv::Rect bbox_b = std::get<0>(b);
	cv::Rect bbox_a_full = std::get<1>(a);
	std::vector<cv::Point> bboxCorner = {
	cv::Point(bbox_a.x, bbox_a.y),
	cv::Point(bbox_a.x + bbox_a.width - 1, bbox_a.y),
	cv::Point(bbox_a.x + bbox_a.width - 1, bbox_a.y + bbox_a.height - 1),
	cv::Point(bbox_a.x, bbox_a.y + bbox_a.height - 1) ,
	cv::Point(bbox_b.x, bbox_b.y),
	cv::Point(bbox_b.x + bbox_b.width - 1, bbox_b.y),
	cv::Point(bbox_b.x + bbox_b.width - 1, bbox_b.y + bbox_b.height - 1),
	cv::Point(bbox_b.x, bbox_b.y + bbox_b.height - 1) };
	cv::Rect newbbox = cv::boundingRect(bboxCorner);
	
	CHECK(newbbox.width<0.66* newbbox.height || newbbox.height < 0.66 * newbbox.width);
	std::get<0>(a) = newbbox;
	std::vector<cv::Point> fullbboxCorner = {
	cv::Point(bbox_a_full.x,  bbox_a_full.y),
	cv::Point(bbox_a_full.x + bbox_a_full.width - 1, bbox_a_full.y),
	cv::Point(bbox_a_full.x + bbox_a_full.width - 1, bbox_a_full.y + bbox_a_full.height - 1),
	cv::Point(bbox_a_full.x,  bbox_a_full.y + bbox_a_full.height - 1)  };

	std::vector<float>dists(4);
	for (int i = 0; i < 4; i++)
	{ 
		dists[i] = getNearestPoint(bboxCorner, fullbboxCorner[i]);
	}
	auto firstFarEle = std::min_element(dists.begin(), dists.end());
	*firstFarEle = 0;
	auto secondFarEle = std::min_element(dists.begin(), dists.end()); 
	std::get<2>(a).emplace_back();
	std::list<cv::Point> newCoverPt;
	newCoverPt.emplace_back(fullbboxCorner[firstFarEle - dists.begin()]);
	newCoverPt.emplace_back(fullbboxCorner[secondFarEle - dists.begin()]);
	std::get<2>(a) = newCoverPt;
	std::get<2>(b).clear();
	return 0;
}
float bbOverlap(const cv::Rect& box1, const cv::Rect& box2)
{
	if (box1.x > box2.x + box2.width) { return 0.0; }
	if (box1.y > box2.y + box2.height) { return 0.0; }
	if (box1.x + box1.width < box2.x) { return 0.0; }
	if (box1.y + box1.height < box2.y) { return 0.0; }
	float colInt = std::min(box1.x + box1.width, box2.x + box2.width) - std::max(box1.x, box2.x);
	float rowInt = std::min(box1.y + box1.height, box2.y + box2.height) - std::max(box1.y, box2.y);
	float intersection = colInt * rowInt;
	float area1 = box1.width * box1.height;
	float area2 = box2.width * box2.height;
	return intersection / (area1 + area2 - intersection);
}
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
	 //contours.clear();
	 //cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	 //if (contours.size() < 1)return -1;
	 //maxArea = 0;
	 //cv::Rect centraRect;
	 //for (int i = 0; i < contours.size(); i++)
	 //{
		// cv::Rect region = cv::boundingRect(contours[i]);
		// if (region.area() > maxArea)
		// {
		//	 maxArea = region.area();
		//	 centraRect = region;
		// };
	 //}
	 //rect.x += centraRect.x;
	 //rect.y += centraRect.y;
	 //rect.width = centraRect.width;
	 //rect.height = centraRect.height;
	 return 0;
}

 //visiable full
 std::vector<std::tuple<cv::Rect, cv::Rect,std::list<cv::Point>>> patternCover(
	 const std::vector<std::vector<cv::Point>>& behindPatternsContor, 
	 const cv::Size& sheepWindowSize, 
	 const int& standardHeight, 
	 const int& standardWidth)
 {
	 std::vector<int>threeAnglePatterns;
	 std::vector<int>twoAnglePatterns;
	 std::vector<int>oneAnglePatterns;
	 for (size_t i = 0; i < behindPatternsContor.size(); i++)
	 {
		 const auto& patternContor = behindPatternsContor[i];
		 cv::Rect bbox = cv::boundingRect(patternContor);

		 if (bbox.width > 0.75 * standardWidth&& bbox.height > 0.75 * standardHeight)
		 {
			 threeAnglePatterns.emplace_back(i);
		 }
		 else if (bbox.width < 0.75 * standardWidth && bbox.height < 0.75 * standardHeight)
		 {
			 oneAnglePatterns.emplace_back(i);
		 }
		 else
		 {
			 twoAnglePatterns.emplace_back(i);
		 } 


	 }
	 
	 std::vector<std::tuple<cv::Rect, cv::Rect, std::list<cv::Point>>> patternAndCover;

	 for (size_t i = 0; i < threeAnglePatterns.size(); i++)
	 {
		 const auto& patternContor = behindPatternsContor[threeAnglePatterns[i]];
		 cv::Rect bbox = cv::boundingRect(patternContor);
		 std::vector<cv::Point> bboxCorner = {
			 cv::Point(bbox.x, bbox.y),
			 cv::Point(bbox.x + bbox.width - 1, bbox.y),
			 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1),
			 cv::Point(bbox.x, bbox.y + bbox.height - 1) };
		 std::vector<cv::Point> hullPts;
		 cv::convexHull(patternContor, hullPts, true);
		 std::vector<float> dists(4);
		 dists[0] = getNearestPoint(hullPts, bboxCorner[0]);
		 dists[1] = getNearestPoint(hullPts, bboxCorner[1]);
		 dists[2] = getNearestPoint(hullPts, bboxCorner[2]);
		 dists[3] = getNearestPoint(hullPts, bboxCorner[3]); 
		 int farId = std::max_element(dists.begin(), dists.end()) - dists.begin(); 
		 patternAndCover.emplace_back(bbox, bbox, std::list<cv::Point>{ bboxCorner[farId] });
	 } 

	 //cv::Mat test = cv::Mat::zeros(sheepWindowSize, CV_8UC1); 
	 for (size_t i = 0; i < twoAnglePatterns.size(); i++)
	 {
		 const auto& patternContor = behindPatternsContor[twoAnglePatterns[i]]; 
		 std::vector<cv::Point>fillContor;
		 fillContor.reserve(patternContor.size());
		 for (int j = 0; j < patternContor.size(); j++)
		 {
			 int prevJ = j == 0 ? (patternContor.size() - 1) : j - 1;
			 if (patternContor[prevJ].x == patternContor[j].x && 1 < abs(patternContor[prevJ].y - patternContor[j].y))
			 {
				 int sign = patternContor[prevJ].y - patternContor[j].y > 0 ? -1 : 1;
				 for (int k = patternContor[prevJ].y + sign; k != patternContor[j].y; k += sign)
				 {
					 fillContor.emplace_back(patternContor[prevJ].x,k);
				 }
			 }
			 if (patternContor[prevJ].y == patternContor[j].y && 1 < abs(patternContor[prevJ].x - patternContor[j].x))
			 {
				 int sign = patternContor[prevJ].x - patternContor[j].x > 0 ? -1 : 1;
				 for (int k = patternContor[prevJ].x + sign; k != patternContor[j].x; k += sign)
				 {
					 fillContor.emplace_back(k, patternContor[prevJ].y);
				 }
			 }
		 }
		 fillContor.insert(fillContor.end(), patternContor.begin(), patternContor.end());
		 //cv::fillPoly(test, patternContor,cv::Scalar(255));

		 cv::Rect bbox = cv::boundingRect(patternContor); 
		 
		 if (bbox.width< bbox.height)//vertical
		 {
			 int leftBboxCnt=0;
			 int rightBboxCnt=0; 
			 int leftThre = bbox.x + 3;
			 int rightThre = bbox.x + bbox.width - 3;;
			 for (int i = 0; i < fillContor.size(); i++)
			 {
				 if (fillContor[i].x < leftThre)
				 {
					 leftBboxCnt++;
				 }
				 if (fillContor[i].x > rightThre)
				 {
					 rightBboxCnt++;
				 }
			 } 
			 if (rightBboxCnt < leftBboxCnt)
			 {
				 cv::Rect visiableRect = bbox;
				 bbox.width = bbox.height;
				 patternAndCover.emplace_back(visiableRect,
				  bbox, 
					 std::list<cv::Point>{
					 cv::Point(bbox.x + bbox.width - 1, bbox.y ),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1) });
			 }
			 else
			 {
				 cv::Rect visiableRect = bbox;
				 bbox.x += (bbox.width - bbox.height);
				 bbox.width = bbox.height;
				 patternAndCover.emplace_back(visiableRect,
				  bbox,
					 std::list<cv::Point>{
					 cv::Point(bbox.x, bbox.y),
						 cv::Point(bbox.x, bbox.y + bbox.height - 1) });
			 }
		 }
		 else//horizon
		 {
			 int upBboxCnt=0;
			 int downBboxCnt = 0;
			 int upThre = bbox.y + 3;
			 int downThre = bbox.y + bbox.height - 3;;
			 for (int i = 0; i < fillContor.size(); i++)
			 {
				 if (fillContor[i].y < upThre)
				 {
					 upBboxCnt++;
				 }
				 if (fillContor[i].y > downThre)
				 {
					 downBboxCnt++;
				 }
			 } 
			 if (upBboxCnt > downBboxCnt)
			 {
				 cv::Rect visiableRect = bbox;
				 bbox.height = bbox.width;
				 patternAndCover.emplace_back(visiableRect,
				  bbox,
					 std::list<cv::Point>{
					 cv::Point(bbox.x, bbox.y + bbox.height - 1),
						 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1) });
			 }
			 else
			 {
				 cv::Rect visiableRect = bbox;
				 bbox.y += (bbox.height - bbox.width);
				 bbox.height = bbox.width;
				 patternAndCover.emplace_back(visiableRect,
				  bbox,
					 std::list<cv::Point>{
					 cv::Point(bbox.x, bbox.y),
						 cv::Point(bbox.x + bbox.width - 1, bbox.y  ) });
			 }
		 } 
	 } 

	 cv::Mat test = cv::Mat::zeros(sheepWindowSize, CV_8UC1);
	 for (size_t i = 0; i < oneAnglePatterns.size(); i++)
	 {
		 const auto& patternContor = behindPatternsContor[oneAnglePatterns[i]];

		 //cv::fillPoly(test, patternContor, cv::Scalar(255));
		 cv::Rect bbox = cv::boundingRect(patternContor); 
		 std::vector<cv::Point> bboxCorner = {
			 cv::Point(bbox.x, bbox.y),
			 cv::Point(bbox.x + bbox.width - 1, bbox.y),
			 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1),
			 cv::Point(bbox.x, bbox.y + bbox.height - 1) };

		 std::vector<cv::Point>fillContor;
		 fillContor.reserve(patternContor.size());
		 for (int j = 0; j < patternContor.size(); j++)
		 {
			 int prevJ = j == 0 ? (patternContor.size() - 1) : j - 1;
			 if (patternContor[prevJ].x == patternContor[j].x && 1 < abs(patternContor[prevJ].y - patternContor[j].y))
			 {
				 int sign = patternContor[prevJ].y - patternContor[j].y > 0 ? -1 : 1;
				 for (int k = patternContor[prevJ].y + sign; k != patternContor[j].y; k += sign)
				 {
					 fillContor.emplace_back(patternContor[prevJ].x, k);
				 }
			 }
			 if (patternContor[prevJ].y == patternContor[j].y && 1 < abs(patternContor[prevJ].x - patternContor[j].x))
			 {
				 int sign = patternContor[prevJ].x - patternContor[j].x > 0 ? -1 : 1;
				 for (int k = patternContor[prevJ].x + sign; k != patternContor[j].x; k += sign)
				 {
					 fillContor.emplace_back(k, patternContor[prevJ].y);
				 }
			 }
		 }
		 fillContor.insert(fillContor.end(), patternContor.begin(), patternContor.end());
		 int x1Cnt = 0;
		 int y1Cnt = 0;
		 int x2Cnt = 0;
		 int y2Cnt = 0;
		 int x2 = bbox.x + bbox.width - 1;
		 int y2 = bbox.y + bbox.height - 1;
		 for (const auto&d: fillContor)
		 {
			 if (d.x == bbox.x)x1Cnt++;
			 if (d.y == bbox.y)y1Cnt++;
			 if (d.x == x2)x2Cnt++;
			 if (d.y == y2)y2Cnt++;
		 }
		 int farId = 0;
		 if (x1Cnt > x2Cnt&& y1Cnt > y2Cnt)
		 {
			 farId = 2;
		 }
		 else if (x1Cnt > x2Cnt && y1Cnt < y2Cnt)
		 {
			 farId = 1;
		 }
		 else if (x1Cnt < x2Cnt && y1Cnt > y2Cnt)
		 {
			 farId = 3;
		 }
		 else
		 {
			 farId = 0;
		 }
		 if (farId==0)
		 {
			 cv::Rect visiableRect = bbox;
			 bbox.x = bboxCorner[2].x - standardWidth+1;
			 bbox.y = bboxCorner[2].y - standardHeight+1;
			 bbox.width = standardWidth;
			 bbox.height = standardHeight;
			 patternAndCover.emplace_back(
				 visiableRect,
				 bbox, std::list<cv::Point>{
				 cv::Point(bbox.x, bbox.y),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y),
					 cv::Point(bbox.x, bbox.y + bbox.height - 1)});
		 }
		 else if (farId == 1)
		 {
			 cv::Rect visiableRect = bbox;
			 bbox.y = bboxCorner[3].y - standardHeight + 1;
			 bbox.width = standardWidth;
			 bbox.height = standardHeight;
			 patternAndCover.emplace_back(
				 visiableRect,
				 bbox, std::list<cv::Point>{
				 cv::Point(bbox.x, bbox.y),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1)});
		 }
		 else if (farId == 2)
		 {
			 cv::Rect visiableRect = bbox;
			 bbox.width = standardWidth;
			 bbox.height = standardHeight;
			 patternAndCover.emplace_back(visiableRect,bbox, std::list<cv::Point>{
				 cv::Point(bbox.x + bbox.width - 1, bbox.y),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1),
					 cv::Point(bbox.x, bbox.y + bbox.height - 1)});
		 }
		 else
		 {
			 cv::Rect visiableRect = bbox;
			 bbox.x = bboxCorner[2].x - standardWidth + 1;
			 bbox.width = standardWidth;
			 bbox.height = standardHeight;
			 patternAndCover.emplace_back(visiableRect,bbox, std::list<cv::Point>{
				 cv::Point(bbox.x, bbox.y),
					 cv::Point(bbox.x + bbox.width - 1, bbox.y + bbox.height - 1),
					 cv::Point(bbox.x, bbox.y + bbox.height - 1) });
		 }
	 }

	 return patternAndCover;
 }

 int analysis(const cv::Mat& img,
	 const cv::Rect& rect,
	 cv::Vec3b& fg,
	 cv::Vec3b& bg,
	 cv::Vec3b& line,
	 std::vector<std::pair<cv::Rect, int>>&frontReture,
	 std::vector<std::tuple<cv::Rect, std::list<cv::Point>, int>>&coveredReture,
	 const unsigned int& diff = 1)
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
	 cv::Mat frontMaskNeg = 1 - frontMask;
	 cv::Mat behindMaskNeg = 1 - behindMask;
	 //cv::imwrite("a.bmp", frontMask);
	 //cv::imwrite("b.bmp", behindMask);
	 cv::Mat sheepWinFront, sheepWinBehind;
	 sheepWin.copyTo(sheepWinFront, frontMaskNeg);
	 sheepWin.copyTo(sheepWinBehind, behindMaskNeg);

	 std::vector<std::vector<cv::Point>> contours;
	 cv::findContours(frontMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	 if (contours.size() < 1)return -1;

	 std::vector<cv::Rect>frontPatterns;//return  
	 int minHeight = rect.width*10;
	 int minWidth =  rect.width*10;
	 int maxHeight = 0;
	 int maxWidth = 0;
	 cv::Size maxFrontPatternsSize(0,0);
	 std::vector<cv::Mat>frontPatternsImgs;
	 std::vector<cv::Mat>frontPatternsMask;
	 frontPatternsImgs.reserve(contours.size());
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
				 const cv::Mat& colorLocal = sheepWinFront(region);
				 cv::Mat patternLocal;
				 colorLocal.copyTo(patternLocal); 

				 frontPatternsImgs.emplace_back(patternLocal);
				 frontPatternsMask.emplace_back(frontMaskNeg(region));
				 if (maxFrontPatternsSize.width < patternLocal.cols)maxFrontPatternsSize.width = patternLocal.cols;
				 if (maxFrontPatternsSize.height < patternLocal.rows)maxFrontPatternsSize.height = patternLocal.rows;
			 }
		 }
	 }
	 std::vector<int>frontPatternsLables;//return
	 lableblock.updata(frontPatternsImgs, frontPatternsMask, frontPatternsLables);
	 frontReture.resize(frontPatternsLables.size());//return
	 for (int i = 0; i < frontPatternsLables.size(); i++)
	 {
		 frontReture[i].first = frontPatterns[i];
		 frontReture[i].second = frontPatternsLables[i];
	 }

	 int heightThre = 0.5 * 0.3 * (minHeight + maxHeight);
	 int widthThre = 0.5 * 0.3 * (minWidth + maxWidth);

	 contours.clear();
	 cv::findContours(behindMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); 
	 std::vector<std::vector<cv::Point>>behindPatternsContor;
	 behindPatternsContor.reserve(contours.size());
	 for (int i = 0; i < contours.size(); i++)
	 {
		 cv::Rect region = cv::boundingRect(contours[i]);
		 if (region.width > widthThre && region.height > heightThre)
		 {
			 behindPatternsContor.emplace_back(contours[i]);
		 } 
	 }
	 std::vector<std::tuple<cv::Rect, cv::Rect, std::list<cv::Point>>> covers = patternCover(behindPatternsContor, sheepWin.size(), minHeight, minWidth);
	 std::vector<int>coversLable(covers.size(),-1);
	  

	 std::list<std::pair<int, int>>iou_max;
	 std::set<int>iou_pair_set;
	 for (int i = 0; i < covers.size(); i++)
	 {
		 if (std::get<2>(covers[i]).size() != 3)continue;
		 for (int j = i+1; j < covers.size(); j++)
		 {
			 if (std::get<2>(covers[j]).size() != 3)continue;
			 if (bbOverlap(std::get<1>(covers[i]), std::get<1>(covers[j]))>0.55)
			 {
				 iou_max.emplace_back(i, j);
				 iou_pair_set.insert(i);
				 iou_pair_set.insert(j);
			 }
		 }
	 }
	 CHECK(iou_pair_set.size()==2* iou_max.size());
	 for (auto d: iou_max)
	 {
		 mergeTwoQuars(covers[d.first], covers[d.second]);
	 }
	  coveredReture.clear();
	  coveredReture.reserve(16);
	 for (int coverId = 0; coverId < covers.size(); coverId++)
	 {
		 auto& d = covers[coverId];
		 const auto& visiableBbox = std::get<0>(d);
		 const auto& fullBbox = std::get<1>(d);
		 const auto& coverdCorner = std::get<2>(d);
		 cv::Mat patternLocalVisiable, patternLocal2;
		 int thisLable = -1;
		 if (coverdCorner.size() == 1)
		 {
			 sheepWinBehind(visiableBbox).copyTo(patternLocalVisiable);
			 patternLocalVisiable *= 2;
			 sheepWinBehind(fullBbox).copyTo(patternLocal2);
			 cv::Point patternCenter(fullBbox.width * 0.5, fullBbox.height * 0.5);
			 cv::Point patternCoverdCorner(coverdCorner.begin()->x - fullBbox.x, coverdCorner.begin()->y - fullBbox.y);
			 cv::Mat patternMask = cv::Mat::ones(fullBbox.size(), CV_8UC1);
			 cv::rectangle(patternMask, patternCenter, patternCoverdCorner, cv::Scalar(0), -1);
#if templateMtach>0
			 thisLable = lableblock.checkOrRegister(false, patternLocalVisiable, patternMask, MATCH_THRE_3_4, COLOR_DIFF_THRE_3_4);
#endif // templateMtach>0
#if USE_SIFT>0
			 thisLable = lableblock.checkOrRegister(false, patternLocalVisiable, patternMask, MATCH_THRE_3_4, COLOR_DIFF_THRE_3_4);
#endif // USE_SIFT>0

			 cv::imwrite(std::to_string(thisLable) + " " + std::to_string(coverId) + ".bmp", patternLocalVisiable);
			 
			 LOG(INFO) << thisLable;
		 }
		 if (coverdCorner.size() == 2)
		 {
			 sheepWinBehind(visiableBbox).copyTo(patternLocalVisiable);
			 patternLocalVisiable *= 2;
			 sheepWinBehind(fullBbox).copyTo(patternLocal2);
			 patternLocal2 *= 2;
			 cv::Mat patternMask = cv::Mat::zeros(fullBbox.size(), CV_8UC1);
			 if (visiableBbox.width > visiableBbox.height)
			 {
				 if (visiableBbox.y != fullBbox.y)
				 {
					 cv::rectangle(patternMask, cv::Rect(0, visiableBbox.y - fullBbox.y, visiableBbox.width, visiableBbox.height), cv::Scalar(1), -1);
				 }
				 else
				 {
					 cv::rectangle(patternMask, cv::Rect(0, 0, visiableBbox.width, visiableBbox.height), cv::Scalar(1), -1);
				 }
			 }
			 else
			 {
				 if (visiableBbox.x != fullBbox.x)
				 {
					 cv::rectangle(patternMask, cv::Rect(visiableBbox.x - fullBbox.x, 0, visiableBbox.width, visiableBbox.height), cv::Scalar(1), -1);
				 }
				 else
				 {
					 cv::rectangle(patternMask, cv::Rect(0, 0, visiableBbox.width, visiableBbox.height), cv::Scalar(1), -1);
				 }
			 }
#if templateMtach>0
			 thisLable = lableblock.checkOrRegister(false, patternLocal2, patternMask, MATCH_THRE_2_4, COLOR_DIFF_THRE_2_4);
#endif // templateMtach>0 
			 cv::imwrite(std::to_string(thisLable) + " " + std::to_string(coverId) + ".bmp", patternLocal2);

			 LOG(INFO) << thisLable;
		 }
		 if (coverdCorner.size() == 33)
		 {
			 cv::Mat tt;
			 global.copyTo(tt);
			 cv::rectangle(tt, fullBbox, cv::Scalar(255, 0, 0), 3);
			 for(auto d1: coverdCorner)
				 cv::circle(tt, d1, 3, cv::Scalar(0, 0, 255), -1); 
			 cv::imshow("123",tt);
			 cv::waitKey();
			 continue;
			 sheepWinBehind(visiableBbox).copyTo(patternLocalVisiable);
			 patternLocalVisiable *= 2;
			 sheepWinBehind(fullBbox).copyTo(patternLocal2);
			 patternLocal2 *= 2;
			 cv::Mat patternMask = cv::Mat::zeros(fullBbox.size(), CV_8UC1);
			 cv::Point A(fullBbox.x, fullBbox.y);
			 cv::Point B(fullBbox.x + fullBbox.width - 1, fullBbox.y);
			 cv::Point C(fullBbox.x + fullBbox.width - 1, fullBbox.y + fullBbox.height - 1);
			 cv::Point D(fullBbox.x, fullBbox.y + fullBbox.height - 1);
			 cv::Point a(visiableBbox.x, visiableBbox.y);
			 cv::Point b(visiableBbox.x + visiableBbox.width - 1, visiableBbox.y);
			 cv::Point c(visiableBbox.x + visiableBbox.width - 1, visiableBbox.y + visiableBbox.height - 1);
			 cv::Point d(visiableBbox.x, visiableBbox.y + visiableBbox.height - 1);
			 int finda = a == A ? 1 : 0;
			 int findb = b == B ? 1 : 0;
			 int findc = c == C ? 1 : 0;
			 int findd = d == D ? 1 : 0;
			 CHECK(finda + findb + findc + findd == 1);
			 cv::Rect temp = visiableBbox;
			 if (finda)
			 {
				 temp.x = 0; temp.y = 0;
				 cv::rectangle(patternMask, temp, cv::Scalar(1), -1);
			 }
			 else if (findb)
			 {
				 temp.x = visiableBbox.x - fullBbox.x; temp.y = 0;
				 cv::rectangle(patternMask, temp, cv::Scalar(1), -1);
			 }
			 else if (findc)
			 {
				 temp.x = visiableBbox.x - fullBbox.x; temp.y = visiableBbox.y - fullBbox.y;
				 cv::rectangle(patternMask, temp, cv::Scalar(1), -1);
			 }
			 else
			 {
				 temp.x = 0; temp.y = visiableBbox.y - fullBbox.y;
				 cv::rectangle(patternMask, temp, cv::Scalar(1), -1);
			 }
#if templateMtach>0
			 thisLable = lableblock.checkOrRegister(false, patternLocal2, patternMask, MATCH_THRE_1_4, COLOR_DIFF_THRE_1_4);
#endif // templateMtach>0 
			 cv::Mat patternLocal3;
			 patternLocal2.copyTo(patternLocal3, patternMask);
			 cv::imwrite(std::to_string(thisLable) + " " + std::to_string(coverId) + ".bmp", patternLocal3); 
			 LOG(INFO) << thisLable;
		 }
		 coveredReture.emplace_back(std::make_tuple(fullBbox, coverdCorner, thisLable));
	 }


	 return 0;
 }

int main()
{
	cv::Mat img = cv::imread("C:/Users/Administrator/Downloads/sheep.bmp");
	cv::Rect sheepWin;
	auto grassColor = cv::Vec3b(139, 254, 195);
	auto fgColor = cv::Vec3b(205,255,245);//front
	auto bgColor = cv::Vec3b(123, 153, 147);//behind
	auto lineColor = cv::Vec3b(30,80,60);//line
	if (pickWindowBaseBg(img, grassColor, sheepWin))
	{
		LOG(ERROR) << "pickWindowBaseBg";
	}
	img(sheepWin).copyTo(global);
	std::vector<std::pair<cv::Rect, int>> frontReture;
	std::vector<std::tuple<cv::Rect, std::list<cv::Point>, int>> coveredReture;
	analysis(img,sheepWin, fgColor, bgColor, lineColor, frontReture, coveredReture);
	
	whichCovers(global,frontReture, coveredReture);

	return 0;
}