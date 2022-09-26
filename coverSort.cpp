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
void delicateRect(cv::Rect&r,const int&n=5)
{
	r.x -= n;
	r.y -= n;
	r.width += 2*n;
	r.height += 2*n;
}
int whichCovers(const cv::Mat& img,
	const std::vector<std::pair<cv::Rect, int>>& frontReture,
	const std::vector<std::tuple<cv::Rect, std::list<cv::Point>, int>>& coveredReture,
	std::vector<Pattern>&layer1,
	std::vector<Pattern>& layer2_1,
	std::vector<Pattern>& layer2_11,
	std::vector<Pattern>& layer3_12,
	std::vector<Pattern>& layer3_22)
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
	layer1.clear();
	layer1.resize(frontReture.size());
	for (int i = 0; i < frontReture.size(); i++)
	{
		layer1[i].id = i;
		layer1[i].r = frontReture[i].first;
		layer1[i].label = frontReture[i].second;
	}
	std::vector<Pattern> templayers;
	for (int i = 0; i < coveredReture.size(); i++)
	{
		Pattern temp;
		temp.id = frontReture.size() + i;
		temp.label = std::get<2>(coveredReture[i]);
		temp.r = std::get<0>(coveredReture[i]); 
		const auto&coveredCorner = std::get<1>(coveredReture[i]);
		if (coveredCorner.size()>2)continue;
		for (int j = 0; j < frontReture.size(); j++)
		{
			float iou = bbOverlapBase(std::get<0>(coveredReture[i]), frontReture[j].first);
			if (iou > COVER_1_4_IOU_MIN && iou <= COVER_1_4_IOU_MAX && coveredCorner.size() == 1)
			{
				if (ptInRect(frontDelicateRect[j], *coveredCorner.begin()))
				{
					temp.insertUpper(j);
					layer1[j].insertlower(temp.id);
				}
			}
			else if (iou > COVER_2_4_IOU_MIN && iou <= COVER_2_4_IOU_MAX && std::get<1>(coveredReture[i]).size() == 2)
			{
				const auto& p1 = *coveredCorner.begin();
				const auto& p2 = *(++coveredCorner.begin());
				if (ptInRect(frontDelicateRect[j], p1) && ptInRect(frontDelicateRect[j], p2))
				{
					temp.insertUpper(j);
					layer1[j].insertlower(temp.id);
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
			if (iou > COVER_1_4_IOU_MIN && iou <= COVER_1_4_IOU_MAX && coveredCorner.size() == 1)
			{
				if (ptInRect(conversDelicateRect[j], *coveredCorner.begin()))
				{
					temp.insertUpper(frontReture.size() + j); 
				}
			}
			else if (iou > COVER_2_4_IOU_MIN && iou <= COVER_2_4_IOU_MAX && std::get<1>(coveredReture[i]).size() == 2)
			{
				const auto& p1 = *coveredCorner.begin();
				const auto& p2 = *(++coveredCorner.begin());
				if (ptInRect(conversDelicateRect[j], p1) && ptInRect(conversDelicateRect[j], p2))
				{
					temp.insertUpper(frontReture.size() + j);
				}
			}
			else if (iou > COVER_1_4_IOU_MIN && iou <= COVER_1_4_IOU_MAX && std::get<1>(coveredReture[i]).size() == 2)
			{
				const auto& p1 = *coveredCorner.begin();
				const auto& p2 = *(++coveredCorner.begin());
				bool in1 = ptInRect(conversDelicateRect[j], p1);
				bool in2 = ptInRect(conversDelicateRect[j], p2);
				if (in1^ in2)
				{
					temp.insertUpper(frontReture.size() + j);
				}
			}
		}
		if (temp.upperCnt > 0)templayers.emplace_back(temp); 
	}
	for (int i = 0; i < templayers.size(); i++)
	{
		for (int j = 0; j < templayers.size(); j++)
		{
			if (i == j)continue;
			if (templayers[i].hasUpper(templayers[j])>=0 && 0>templayers[j].hasLower(templayers[i]))
			{
				templayers[j].insertlower(templayers[i].id);
			}
		}
	}



#if SHOW_COVERS>0
				cv::Mat tt;
				img.copyTo(tt);
				cv::rectangle(tt, std::get<0>(coveredReture[i]), cv::Scalar(255, 0, 0), 3);
				cv::rectangle(tt, frontReture[d].first, cv::Scalar(0, 0, 255), 3);
				cv::imshow("123", tt);
				cv::waitKey();
#endif // SHOW_COVERS>0						
#if SHOW_COVERS>0
				cv::Mat tt;
				img.copyTo(tt);
				cv::rectangle(tt, std::get<0>(coveredReture[i]), cv::Scalar(255, 0, 0), 3);
				cv::rectangle(tt, std::get<0>(coveredReture[d - frontReture.size()]), cv::Scalar(0, 0, 255), 3);
				cv::imshow("123", tt);
				cv::waitKey();
#endif // SHOW_COVERS>0
				layer2_1.clear(); layer2_11.clear();
				layer3_12.clear(); layer3_22.clear();
	//for (auto d : layer1)LOG(INFO) << d;
	//for (auto d : templayers)LOG(INFO) << d;
	std::list<int>layer2_1_ids;
	std::list<int>layer2_11_ids;
	for (auto d : templayers)
	{
		if (d.upperCnt==1 && d.uppers[0]< layer1.size())
		{
			layer2_1.emplace_back(d);
			layer2_1_ids.emplace_back(d.id);
		}
		if (d.upperCnt == 2 && d.uppers[0] < layer1.size() && d.uppers[1] < layer1.size())
		{
			layer2_11.emplace_back(d);
			layer2_11_ids.emplace_back(d.id);
		}
	}
	for (auto d : templayers)
	{ 
		if (d.upperCnt == 2 )
		{
			bool isLayer2_1_ids1 = layer2_1_ids.end() != std::find(layer2_1_ids.begin(), layer2_1_ids.end(), d.uppers[0]);
			bool isLayer2_1_ids2 = layer2_1_ids.end() != std::find(layer2_1_ids.begin(), layer2_1_ids.end(), d.uppers[1]);
			bool isLayer2_11_ids1 = layer2_11_ids.end() != std::find(layer2_11_ids.begin(), layer2_11_ids.end(), d.uppers[0]);
			bool isLayer2_11_ids2 = layer2_11_ids.end() != std::find(layer2_11_ids.begin(), layer2_11_ids.end(), d.uppers[1]);
			if (   d.uppers[0] < layer1.size() && isLayer2_1_ids1 
				|| d.uppers[1] < layer1.size() && isLayer2_1_ids2
				|| d.uppers[0] < layer1.size() && isLayer2_11_ids1
				|| d.uppers[1] < layer1.size() && isLayer2_11_ids2)
			{
				layer3_12.emplace_back(d);
			}
			if (isLayer2_1_ids1+ isLayer2_1_ids2 + isLayer2_11_ids1 + isLayer2_11_ids2 ==2)
			{
				layer3_22.emplace_back(d);
			}
		} 
	}
	
	



	return 0;
}
