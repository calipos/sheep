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
std::list<std::list<int>> choose(const std::list<int>& a, const int& chooseNum)
{
	CHECK(chooseNum<=a.size());
	CHECK(chooseNum > 0 && chooseNum <= 3);
	std::list<std::list<int>>ret1;
	if (chooseNum==1)
	{
		for (const auto&d:a)
		{
			ret1.emplace_back(std::list<int>{d});
		}
		return ret1;
	}
	if (chooseNum==2)
	{
		for (auto it1 = a.begin(); it1 != a.end(); it1++)
		{
			for (auto it2 = it1; it2 != a.end(); it2++)
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
		std::vector<int>idx(a.size());
		std::vector<int>A(a.size());
		auto it = a.begin();
		for (int i = 0; i < a.size(); i++)
		{
			idx[i] = i;
			A[i] = *it++;;
		}
		sorts.emplace_back(std::list<int>{idx[0], idx[1], idx[2]});
		for (int i = 0; i < 3; i++)
		{
			for (int j = 3; j < a.size(); j++)
			{ 
				std::vector<int>temp = idx;
				std::swap(temp[i], temp[j]);
				sorts.emplace_back(std::list<int>{temp[0], temp[1], temp[2]});
			}
		}
		std::list<std::pair<int, int>>swapRead;
		for (int i = 3; i < a.size(); i++)
		{
			for (int j = i+1; j < a.size(); j++)
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
	CHECK(frontId>=0 && frontId< layer1.size());
	std::list<int>layer1_ids;
	Pattern* choosedFront = nullptr;
	for (auto it= layer1.begin(); it != layer1.end();it++)
	{ 
		if (it->id == frontId)
		{
			choosedFront = &(*it);
			//layer1.erase(it);
			//break;
		}
		else
		{
			layer1_ids.emplace_back(it->id);
		}
	}
	CHECK(choosedFront != nullptr);;
	std::list<Pattern>remains; 
	for (int i = 0; i < layer2_1.size(); i++)
	{  
		int upperId = layer2_1[i].hasUpper(*choosedFront);
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
		int upperId = layer2_11[i].hasUpper(*choosedFront);
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
		int upperId = layer3_12[i].hasUpper(*choosedFront);
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
int chooseFrontlIST(const std::list<int>& frontIds,
	std::vector<Pattern>& layer1,
	std::vector<Pattern>& layer2_1,
	std::vector<Pattern>& layer2_11,
	std::vector<Pattern>& layer3_12,
	std::vector<Pattern>& layer3_22)
{

	return 0;
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
	if (currentLabels.size()>0)	
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
		labelCluster[layer1[i].label].emplace_back(i);
	}
	for (auto &d: labelCluster)
	{
		if (d.second.size()>=3)
		{
			int existedCnt = 0;
			std::list<int>potentialChoose;
			for (auto&d1: d.second)
			{
				if (d1 < 0)existedCnt++;
				else
				{
					potentialChoose.emplace_back(d1);
				}
			}
			std::list<std::list<int>> path = choose(potentialChoose,3- existedCnt);
			candidatePath.insert(candidatePath.end(), path.begin(), path.end());
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
	ScoreChain& scoreChain)
{
	std::list<std::list<int>> candidatePath = getCandidate(currentLabels, layer1, layer2_1, layer2_11, layer3_12, layer3_22);
	if (candidatePath.size()==0)
	{
		return 0;
	}
	//scoreChain.nextStep = new ScoreChain[candidate.size()];
	//scoreChain.nextStepCnt = candidate.size();
	//int candidateIdx = 0;
	//for (auto &d: candidate)
	//{
	//	std::vector<int>newCurrentLabels = currentLabels;
	//	std::vector<Pattern>new_layer1 = layer1;
	//	std::vector<Pattern>new_layer2_1 = layer2_1;
	//	std::vector<Pattern>new_layer2_11 = layer2_11;
	//	std::vector<Pattern>new_layer3_12 = layer3_12;
	//	std::vector<Pattern>new_layer3_22 = layer3_22;
	//	chooseFrontOne(d,
	//		new_layer1,
	//		new_layer2_1,
	//		new_layer2_11,
	//		new_layer3_12, 
	//		new_layer3_22);
	//	newCurrentLabels.emplace_back(layer1[d].label);
	//	std::map<int, int>collection;
	//	for (auto &d1: newCurrentLabels)
	//	{
	//		if (collection.count(d1)==0)
	//		{
	//			collection[d1] = 1;
	//		}
	//		else
	//		{
	//			collection[d1]++;
	//		}
	//	}
	//	int yes_label = -1;
	//	for (auto&d1: collection)
	//	{
	//		if (d1.second>=3)
	//		{
	//			yes_label = d1.first;
	//			break;
	//		}
	//	}
	//	if (yes_label<0)
	//	{
	//		scoreChain.nextStep->score = no_;
	//	}
	//	else
	//	{
	//		scoreChain.nextStep->score = yes_;
	//		for (auto iter = newCurrentLabels.begin(); iter != newCurrentLabels.end(); )
	//		{
	//			if (*iter == yes_label)
	//				iter = newCurrentLabels.erase(iter);//当删除时erase函数自动指向下一个位置，就不需要进行++
	//			else
	//				iter++;    //当没有进行删除的时候，迭代器++
	//		}
	//	}
	//	recursive(
	//		newCurrentLabels,
	//		new_layer1,
	//		new_layer2_1,
	//		new_layer2_11,
	//		new_layer3_12,
	//		new_layer3_22,
	//		*scoreChain.nextStep);
	//	candidateIdx++;
	//}
	return 0;

}