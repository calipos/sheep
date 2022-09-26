#include<iostream>
#include <map>
#include <vector>
#include <list>
#include <tuple>
#include <iterator>
#include "opencv2/opencv.hpp"
#include "glog/logging.h"
#include "common.h"
#ifdef _MSC_VER
#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif // !GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif 

int chooseOne(const int&frontId,
	std::vector<Pattern>& layer1,
	std::vector<Pattern>& layer2_1,
	std::vector<Pattern>& layer2_11,
	std::vector<Pattern>& layer3_12,
	std::vector<Pattern>& layer3_22)
{
	CHECK(frontId>=0 && frontId< layer1.size());
	for (auto it= layer1.begin(); it != layer1.end();it++)
	{
		if (it- layer1.begin() == frontId)
		{
			layer1.erase(it);
			break;
		}
	}
	std::vector<int>layer2_1_discard;
	for (int i = 0; i < layer2_1.size(); i++)
	{
		int upperId = layer2_1[i].hasUpper(layer1[frontId]);
		if (upperId>=0)
		{ 
			layer2_1[i].eraseUpper(upperId);
			if (layer2_1.size()==0)
			{
				layer2_1_discard.emplace_back(i);
				layer1.emplace_back(layer2_1[i]);
			}
		}
	}
}

std::vector<int> pickChoose(const std::vector<int>& currentLabels,
	const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22)
{
	if (currentLabels.size() == MAX_PATTERN)return std::vector<int>();
	std::list<int> choose; 
	std::map<int, std::list<int>>labelCluster;
	if (currentLabels.size()>0)
	{
		for (int i = 0; i < currentLabels.size(); i++)
		{
			if (labelCluster.count(currentLabels[i]) == 0)
			{
				labelCluster[currentLabels[i]] = std::list<int>();
			}
			labelCluster[currentLabels[i]].emplace_back(-1); 
		}
	}
	for (int i = 0; i < layer1.size(); i++)
	{
		if (labelCluster.count(layer1[i].label)==0)
		{
			labelCluster[layer1[i].label] = std::list<int>();
		}
		labelCluster[layer1[i].label].emplace_back(i);
	}

	for (const auto&d: labelCluster)
	{
		if (d.second.size()>=3)
		{
			for (const auto& d1 : d.second)
			{
				if (d1>=0)
				{
					choose.emplace_back(d1);
				} 
			}
		}
	}

	std::vector<std::vector<int>> newCurrentLabels(choose.size());
	int idx = 0;
	for (auto&d: choose)
	{ 
		newCurrentLabels[idx] = currentLabels;
		newCurrentLabels[idx].emplace_back(d);
		idx++;
	}
	return ret;
}

std::vector< std::vector<int >> getPotentialChoose(const std::vector<int>& currentLabels,
	const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22,
	int&score)
{



}
bool recursive(const std::vector<int>&currentLabels, 
	const std::vector<Pattern>& layer1,
	std::vector<Pattern>& layer2_1,
	std::vector<Pattern>& layer2_11,
	std::vector<Pattern>& layer3_12,
	std::vector<Pattern>& layer3_22)
{



}