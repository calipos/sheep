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
Label lableblock;
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

		 cv::fillPoly(test, patternContor, cv::Scalar(255));
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
		 if (farId==0)
		 {
			 cv::Rect visiableRect = bbox;
			 bbox.x = bboxCorner[2].x - standardWidth-1;
			 bbox.y = bboxCorner[2].y - standardHeight-1;
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
			 bbox.y = bboxCorner[3].y - standardHeight - 1;
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
			 bbox.x = bboxCorner[2].x - standardWidth - 1;
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

 int analysis(const cv::Mat&img,const cv::Rect&rect, cv::Vec3b& fg, cv::Vec3b& bg, cv::Vec3b& line, const unsigned int& diff = 1)
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

	 std::vector<cv::Rect>frontPatterns;
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
				 //std::vector<cv::Point> contourLocal(contours[i].size());
				 //for (int j = 0; j < contours[i].size(); j++)
				 //{
					// contourLocal[j] = contours[i][j];
					// contourLocal[j].x -= region.x;
					// contourLocal[j].y -= region.y;
				 //}
				 //const cv::Mat&colorLocal = sheepWin(region);
				 //cv::Mat patternMaskLocal = cv::Mat::zeros(region.size(),CV_8UC1);
				 //cv::fillPoly(patternMaskLocal, contourLocal,cv::Scalar(255));
				 //cv::Mat patternLocal;
				 //colorLocal.copyTo(patternLocal, patternMaskLocal); 
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
	 std::vector<int>frontPatternsLables;
	 lableblock.updata(frontPatternsImgs, frontPatternsMask, frontPatternsLables);
	 
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
	 for (int coverId = 0; coverId < covers.size(); coverId++)
	 {
		 auto& d = covers[coverId]; 
		 const auto& visiableBbox = std::get<0>(d);
		 const auto& fullBbox = std::get<1>(d);
		 const auto& coverdCorner = std::get<2>(d);
		 cv::Mat patternLocalVisiable, patternLocal2; 
		if (coverdCorner.size() == 11)
		{
			sheepWinBehind(visiableBbox).copyTo(patternLocalVisiable);
			patternLocalVisiable *= 2;
			sheepWinBehind(fullBbox).copyTo(patternLocal2);
			cv::Point patternCenter(fullBbox.width * 0.5, fullBbox.height * 0.5);
			cv::Point patternCoverdCorner(coverdCorner.begin()->x - fullBbox.x, coverdCorner.begin()->y - fullBbox.y);
			cv::Mat patternMask = cv::Mat::ones(fullBbox.size(), CV_8UC1);
			cv::rectangle(patternMask, patternCenter, patternCoverdCorner, cv::Scalar(0), -1);
#if templateMtach>0
			int thisLable = lableblock.checkOrRegister(false, patternLocalVisiable, patternMask, MATCH_THRE_3_4);
#endif // templateMtach>0
#if USE_SIFT>0
			int thisLable = lableblock.checkOrRegister(false,patternLocalVisiable, patternMask, MATCH_SIFT_THRE_3_4);
#endif // USE_SIFT>0
			
			cv::imwrite(std::to_string(thisLable)+" "+ std::to_string(coverId)+".bmp", patternLocalVisiable);

			LOG(INFO) << thisLable;
		} 
		if (coverdCorner.size() == 2)
		{
			sheepWinBehind(visiableBbox).copyTo(patternLocalVisiable);
			patternLocalVisiable *= 2;
			sheepWinBehind(fullBbox).copyTo(patternLocal2);
			cv::Point patternCenter(fullBbox.width * 0.5, fullBbox.height * 0.5);
			cv::Point patternCoverdCorner(coverdCorner.begin()->x - fullBbox.x, coverdCorner.begin()->y - fullBbox.y);
			cv::Mat patternMask = cv::Mat::ones(fullBbox.size(), CV_8UC1);
			cv::rectangle(patternMask, patternCenter, patternCoverdCorner, cv::Scalar(0), -1);
		}
	 }


		// } 
		// cv::imshow("123",test);
		// cv::waitKey(0);
	 //}
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

	analysis(img,sheepWin, fgColor, bgColor, lineColor);


	return 0;
}