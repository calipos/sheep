#include<iostream>
#include <map>
#include <algorithm>
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
int score(const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22)
{
	return (1 << 5) * layer1.size()
		+ (1 << 4) * layer2_1.size()
		+ (1 << 3) * layer2_11.size()
		+ (1 << 2) * layer3_12.size()
		+ (1 << 1) * layer3_22.size();
}
std::list<std::list<int>> choose(const std::list<int>& ids, const int& chooseNum)
{
	CHECK(chooseNum<= ids.size());
	CHECK(chooseNum > 0 && chooseNum <= 3);
	std::list<std::list<int>>ret1;
	if (chooseNum==1)
	{
		for (const auto&id: ids)
		{
			ret1.emplace_back(std::list<int>{id});
		}
		return ret1;
	}
	if (chooseNum==2)
	{
		for (auto it1 = ids.begin(); it1 != ids.end(); it1++)
		{
			for (auto it2 = it1; it2 != ids.end(); it2++)
			{
				if (it1 == it2)continue;
				ret1.emplace_back(std::list<int>{ *it1, * it2 });
			}
		}
		return ret1;
	}
	if (chooseNum == 3)
	{
		std::list<std::list<int>>sorts;
		std::vector<int>idx(ids.size());
		std::vector<int>A(ids.size());
		auto it = ids.begin();
		for (int i = 0; i < ids.size(); i++)
		{
			idx[i] = i;
			A[i] = *it++;;
		}
		sorts.emplace_back(std::list<int>{idx[0], idx[1], idx[2]});
		for (int i = 0; i < 3; i++)
		{
			for (int j = 3; j < ids.size(); j++)
			{ 
				std::vector<int>temp = idx;
				std::swap(temp[i], temp[j]);
				sorts.emplace_back(std::list<int>{temp[0], temp[1], temp[2]});
			}
		}
		std::list<std::pair<int, int>>swapRead;
		for (int i = 3; i < ids.size(); i++)
		{
			for (int j = i+1; j < ids.size(); j++)
			{ 
				swapRead.emplace_back(std::make_pair(i, j));
			}
		}
		for (auto&d: swapRead)
		{
			{
				std::vector<int>temp = idx;
				std::swap(temp[0], temp[d.first]);
				std::swap(temp[1], temp[d.second]);
				sorts.emplace_back(std::list<int>{temp[0], temp[1], temp[2]});
			}
			{
				std::vector<int>temp = idx;
				std::swap(temp[0], temp[d.first]);
				std::swap(temp[2], temp[d.second]);
				sorts.emplace_back(std::list<int>{temp[0], temp[1], temp[2]});
			}
			{
				std::vector<int>temp = idx;
				std::swap(temp[2], temp[d.first]);
				std::swap(temp[1], temp[d.second]);
				sorts.emplace_back(std::list<int>{temp[0], temp[1], temp[2]});
			}
		}
		for (auto&d: sorts)
		{
			for (auto&d1:d)
			{
				d1 = A[d1];
			}
		} 
		return sorts;
	}
	 
	return std::list<std::list<int>>();
}
int chooseFrontOne(const int&frontId,
	std::vector<Pattern>& layer1,
	std::vector<Pattern>& layer2_1,
	std::vector<Pattern>& layer2_11,
	std::vector<Pattern>& layer3_12,
	std::vector<Pattern>& layer3_22)
{
	//CHECK(frontId>=0 && frontId< layer1.size());
	std::list<int>layer1_ids;
	Pattern choosedFront;
	for (auto it= layer1.begin(); it != layer1.end();)
	{ 
		if (it->id == frontId)
		{
			choosedFront = *it;
			it=layer1.erase(it);
			//break;
		}
		else
		{
			layer1_ids.emplace_back(it->id);
			it++;
		}
	}
	std::list<Pattern>remains; 
	for (int i = 0; i < layer2_1.size(); i++)
	{  
		int upperId = layer2_1[i].hasUpper(choosedFront);
		if (upperId>=0)
		{ 
			layer2_1[i].eraseUpper(upperId);
			layer1_ids.emplace_back(layer2_1[i].id);
			remains.emplace_back(layer2_1[i]);
		}
		else
		{
			remains.emplace_back(layer2_1[i]);
		}
	}
	for (int i = 0; i < layer2_11.size(); i++)
	{
		int upperId = layer2_11[i].hasUpper(choosedFront);
		if (upperId >= 0)
		{
			layer2_11[i].eraseUpper(upperId);
			remains.emplace_back(layer2_11[i]);
		}
		else
		{
			remains.emplace_back(layer2_11[i]);
		}
	}
	for (int i = 0; i < layer3_12.size(); i++)
	{
		int upperId = layer3_12[i].hasUpper(choosedFront);
		if (upperId >= 0)
		{
			layer3_12[i].eraseUpper(upperId);
			remains.emplace_back(layer3_12[i]);
		}
		else
		{
			remains.emplace_back(layer3_12[i]);
		}
	}
	for (int i = 0; i < layer3_22.size(); i++)
	{ 
		remains.emplace_back(layer3_22[i]); 
	}
	std::vector<Pattern>new_layer2_1;
	std::vector<Pattern>new_layer2_11;
	std::vector<Pattern>new_layer3_12;
	std::vector<Pattern>new_layer3_22;
	std::list<int>layer2_ids;
	for (auto&d: remains)
	{
		if (d.upperCnt == 0)
		{
			layer1.emplace_back(d);
		}
		if (d.upperCnt == 1 && layer1_ids.end() != std::find(layer1_ids.begin(), layer1_ids.end(), d.uppers[0]))
		{
			new_layer2_1.emplace_back(d);
			layer2_ids.emplace_back(d.id);
		}
		if (d.upperCnt == 2
			&& layer1_ids.end() != std::find(layer1_ids.begin(), layer1_ids.end(), d.uppers[0])
			&& layer1_ids.end() != std::find(layer1_ids.begin(), layer1_ids.end(), d.uppers[1]))
		{
			new_layer2_11.emplace_back(d);
			layer2_ids.emplace_back(d.id);
		}
	}
	for (auto& d : remains)
	{
		if (d.upperCnt == 2)
		{
			bool find_layer1_a = layer1_ids.end() != std::find(layer1_ids.begin(), layer1_ids.end(), d.uppers[0]);
			bool find_layer1_b = layer1_ids.end() != std::find(layer1_ids.begin(), layer1_ids.end(), d.uppers[1]);
			bool find_layer2_a = layer2_ids.end() != std::find(layer2_ids.begin(), layer2_ids.end(), d.uppers[0]);
			bool find_layer2_b = layer2_ids.end() != std::find(layer2_ids.begin(), layer2_ids.end(), d.uppers[1]);
			if (find_layer2_a && find_layer2_b)
			{
				new_layer3_22.emplace_back(d);
			}
			if (   find_layer1_a && find_layer2_b
				|| find_layer1_b && find_layer2_a)
			{
				new_layer3_12.emplace_back(d);
			}
		} 
	}
	layer2_1 = new_layer2_1;
	layer2_11 = new_layer2_11;
	layer3_12 = new_layer3_12;
	layer3_22 = new_layer3_22;
	return 0;
}
int chooseFrontList(const std::list<int>& frontIds,
	const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22)
{
	std::vector<Pattern>new_layer1 = layer1;
	std::vector<Pattern>new_layer2_1 = layer2_1;
	std::vector<Pattern>new_layer2_11 = layer2_11;
	std::vector<Pattern>new_layer3_12 = layer3_12;
	std::vector<Pattern>new_layer3_22 = layer3_22;
	for (const auto&d: frontIds)
	{
		chooseFrontOne(d, new_layer1, new_layer2_1, new_layer2_11, new_layer3_12, new_layer3_22);
	}

	return score(new_layer1, new_layer2_1, new_layer2_11, new_layer3_12, new_layer3_22);
}
std::list<std::list<int>> getCandidate(const std::vector<int>& currentLabels,
	const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22)
{
	if (currentLabels.size() == MAX_PATTERN)return std::list<std::list<int>>();
	std::list<std::list<int>> candidatePath;
	std::map<int, std::list<int>>labelCluster;
	//if (currentLabels.size()>0)	
	for (int i = 0; i < currentLabels.size(); i++)
	{
		if (labelCluster.count(currentLabels[i]) == 0)
		{
			labelCluster[currentLabels[i]] = std::list<int>();
		}
		labelCluster[currentLabels[i]].emplace_back(-1); 
	}
	for (int i = 0; i < layer1.size(); i++)
	{
		if (labelCluster.count(layer1[i].label)==0)
		{
			labelCluster[layer1[i].label] = std::list<int>();
		}
		labelCluster[layer1[i].label].emplace_back(layer1[i].id);
	}
	for (auto &d: labelCluster)
	{
		if (d.second.size()>=3)
		{
			int existedCnt = 0;
			std::list<int>potentialIdChoose;
			for (auto&id: d.second)
			{
				if (id < 0)existedCnt++;
				else
				{
					potentialIdChoose.emplace_back(id);
				}
			}
			if (3 - existedCnt+ currentLabels.size()<=MAX_PATTERN)
			{
				std::list<std::list<int>> path = choose(potentialIdChoose, 3 - existedCnt);
				candidatePath.insert(candidatePath.end(), path.begin(), path.end());
			}
			
		}
	} 
	return candidatePath;
}

 
int recursive(const std::vector<int>&currentLabels, 
	const std::vector<Pattern>& layer1,
	const std::vector<Pattern>& layer2_1,
	const std::vector<Pattern>& layer2_11,
	const std::vector<Pattern>& layer3_12,
	const std::vector<Pattern>& layer3_22,
	cv::Rect& pickRect)
{
	//look for 3 
	std::list<std::list<int>> candidatePath = getCandidate(currentLabels, layer1, layer2_1, layer2_11, layer3_12, layer3_22);
	if (candidatePath.size() != 0)
	{
		std::list<std::pair<int, int>>scores;
		for (const auto& d : candidatePath)
		{
			int score = chooseFrontList(d, layer1, layer2_1, layer2_11, layer3_12, layer3_22);
			scores.emplace_back(std::make_pair(score, *d.begin()));
		}
		scores.sort([](const auto& a, const auto& b) {return a.first > b.first; });
		pickRect.x = -1;
		int hitLabel = -1;
		for (const auto& d : layer1)
		{
			if (d.id == scores.begin()->second)
			{
				pickRect = d.r;
				hitLabel = d.label;
			}
		}
		return hitLabel;
	}
	else
	{
		std::list<std::pair<int, int>>scores;
		for (const auto&d: layer1)
		{
			std::vector<Pattern>new_layer1 = layer1;
			std::vector<Pattern>new_layer2_1 = layer2_1;
			std::vector<Pattern>new_layer2_11 = layer2_11;
			std::vector<Pattern>new_layer3_12 = layer3_12;
			std::vector<Pattern>new_layer3_22 = layer3_22; 
			chooseFrontOne(d.id, new_layer1, new_layer2_1, new_layer2_11, new_layer3_12, new_layer3_22);
			std::vector<int>  newCurrentLabelsTmp = currentLabels;
			newCurrentLabelsTmp.emplace_back(d.id);
			std::map<int, int>labelClusterTmp;
			for (auto& d : newCurrentLabelsTmp)
			{
				if (labelClusterTmp.count(d)==0)
				{
					labelClusterTmp[d] = 1;
				}
				else
				{
					labelClusterTmp[d]++;
				}
			}
			int littleScore = 0;
			for (auto& d : labelClusterTmp)
			{
				if (d.second >= 2)
				{
					littleScore++;
				}
			}

			//look for 3 
			std::list<std::list<int>> candidatePath2 = getCandidate(newCurrentLabelsTmp, new_layer1, new_layer2_1, new_layer2_11, new_layer3_12, new_layer3_22);
			if (candidatePath2.size() != 0)
			{ 
				for (const auto& d1 : candidatePath)
				{
					int score = chooseFrontList(d1, layer1, layer2_1, layer2_11, layer3_12, layer3_22);
					scores.emplace_back(std::make_pair(score+ littleScore, d.id));
				} 
			}
			else
			{
				scores.emplace_back(std::make_pair(littleScore, d.id));
			}
		}
		scores.sort([](const auto& a, const auto& b) {return a.first > b.first; });
		pickRect.x = -1;
		int hitLabel = -1;
		for (const auto& d : layer1)
		{
			if (d.id == scores.begin()->second)
			{
				pickRect = d.r;
				hitLabel = d.label;
			}
		}
		return hitLabel;
	}

}