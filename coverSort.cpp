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
float bbOverlapBase(const cv::Rect& base, const cv::Rect& box2)
{
	if (base.x > box2.x + box2.width) { return 0.0; }
	if (base.y > box2.y + box2.height) { return 0.0; }
	if (base.x + base.width < box2.x) { return 0.0; }
	if (base.y + base.height < box2.y) { return 0.0; }
	float colInt = std::min(base.x + base.width, box2.x + box2.width) - std::max(base.x, box2.x);
	float rowInt = std::min(base.y + base.height, box2.y + box2.height) - std::max(base.y, box2.y);
	float intersection = colInt * rowInt;
	float area1 = base.width * base.height;
	return intersection / area1;
}
bool ptInRect(const cv::Rect& box, const cv::Point& pt)
{
	if (pt.x >= box.x && pt.y >= box.y && pt.x < box.x + box.width && pt.y < box.y + box.height)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void delicateRect(cv::Rect&r,const int&n=3)
{
	r.x -= n;
	r.y -= n;
	r.width += 2*n;
	r.height += 2*n;
}
int whichCovers(const cv::Mat&img,
	const std::vector<std::pair<cv::Rect, int>>&frontReture,
	const std::vector<std::tuple<cv::Rect, std::list<cv::Point>, int>>&coveredReture)
{
	std::vector<cv::Rect>frontDelicateRect(frontReture.size());
	std::vector<cv::Rect>conversDelicateRect(coveredReture.size());
	for (int i = 0; i < frontReture.size(); i++)
	{
		frontDelicateRect[i] = frontReture[i].first;
		delicateRect(frontDelicateRect[i]);
	}
	for (int i = 0; i < coveredReture.size(); i++)
	{
		conversDelicateRect[i] = std::get<0>(coveredReture[i]);
		delicateRect(conversDelicateRect[i]);
	}
	std::vector<std::list<int>>uppers(coveredReture.size());
	for (int i = 0; i < coveredReture.size(); i++)
	{
		const auto&coveredCorner = std::get<1>(coveredReture[i]);
		if (coveredCorner.size()>2)continue;
		for (int j = 0; j < frontReture.size(); j++)
		{
			float iou = bbOverlapBase(std::get<0>(coveredReture[i]), frontReture[j].first);
			if (iou > 0.18 && iou < 0.33 && coveredCorner.size() == 1)
			{
				if (ptInRect(frontDelicateRect[j], *coveredCorner.begin()))
				{
					uppers[i].emplace_back(j);
				}
			}
			else if (iou > 0.4 && iou < 0.6 && std::get<1>(coveredReture[i]).size() == 2)
			{
				const auto& p1 = *coveredCorner.begin();
				const auto& p2 = *(++coveredCorner.begin());
				if (ptInRect(frontDelicateRect[j], p1) && ptInRect(frontDelicateRect[j], p2))
				{
					uppers[i].emplace_back(j);
				}
			}
		}
		for (int j = 0; j < coveredReture.size(); j++)
		{
			if (i == j)continue;
			const auto& coveredCorner2 = std::get<1>(coveredReture[j]);
			if (coveredCorner2.size() > 2)continue;
			const cv::Rect potentialUpper = std::get<0>(coveredReture[j]);
			float iou = bbOverlapBase(std::get<0>(coveredReture[i]), potentialUpper);
			if (iou > 0.18 && iou < 0.33 && coveredCorner.size() == 1)
			{
				if (ptInRect(conversDelicateRect[j], *coveredCorner.begin()))
				{
					uppers[i].emplace_back(frontReture.size()+j);
				}
			}
			else if (iou > 0.4 && iou < 0.6 && std::get<1>(coveredReture[i]).size() == 2)
			{
				const auto& p1 = *coveredCorner.begin();
				const auto& p2 = *(++coveredCorner.begin());
				if (ptInRect(conversDelicateRect[j], p1) && ptInRect(conversDelicateRect[j], p2))
				{
					uppers[i].emplace_back(frontReture.size() + j);
				}
			}
		}

	}
	for (int i = 0; i < uppers.size(); i++)
	{
		
		for (auto d : uppers[i])
		{
			
			if (d< frontReture.size())
			{
				cv::Mat tt;
				img.copyTo(tt);
				cv::rectangle(tt, std::get<0>(coveredReture[i]), cv::Scalar(255, 0, 0), 3);
				cv::rectangle(tt, frontReture[i].first, cv::Scalar(0, 0, 255), 3);
				cv::imshow("123", tt);
				cv::waitKey();
			}
			else
			{
				cv::Mat tt;
				img.copyTo(tt);
				cv::rectangle(tt, std::get<0>(coveredReture[i]), cv::Scalar(255, 0, 0), 3);
				cv::rectangle(tt, std::get<0>(coveredReture[d- frontReture.size()]), cv::Scalar(0, 0, 255), 3);
				cv::imshow("123", tt);
				cv::waitKey();
			}
		}
	}
	
	return 0;
}
