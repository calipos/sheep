#include<iostream>
#include <vector>
#include <list>
#include <tuple>
#include "opencv2/opencv.hpp"
#include "glog/logging.h"
#include "common.h"
#include "labelUpdata.h"


Label::Label()
{
#if USE_SIFT>0
	extractor = cv::SIFT::create(50);
	

	//vector<KeyPoint> keypoints_1, keypoints_2;
	//f2d->detect(img_1, keypoints_1);
	//f2d->detect(img_2, keypoints_2);
	////第二步,计算特征向量
	//Mat descriptors_1, descriptors_2;
	//f2d->compute(img_1, keypoints_1, descriptors_1);
	//f2d->compute(img_2, keypoints_2, descriptors_2);
	//// 第三步,用BFMatcher进行匹配特征向量
	//BFMatcher matcher;
	//vector<DMatch> matches;
	//matcher.match(descriptors_1, descriptors_2, matches);
#endif // USE_SIFT>0

}
cv::Mat diffMask(const cv::Mat& a, const cv::Mat& b, const cv::Mat& mask)
{
	cv::Mat diff = cv::Mat::zeros(a.size(),CV_8UC1);
	for (size_t r = 0; r < a.rows; r++)
	{
		for (size_t c = 0; c < a.cols; c++)
		{
			if (mask.ptr<uchar>(r)[c])
			{
				diff.ptr<uchar>(r)[c] = abs(int(a.ptr<uchar>(r)[c]) - int(b.ptr<uchar>(r)[c]));
			}
		}
	}
	return diff;
}
float avgMat(cv::Mat& a)
{
	a.convertTo(a,CV_32FC1);
	cv::reduce(a, a, 0, cv::REDUCE_AVG);
	cv::reduce(a, a, 1, cv::REDUCE_AVG);
	return a.ptr<float>(0)[0];
}
void Label::updata(const std::vector<cv::Mat>& newPatterns, const std::vector<cv::Mat>& newPatternMask, std::vector<int>&localLabel)
{
	localLabel.resize(newPatterns.size(),-1);
	cv::Size maxPatternsSize(0, 0);
	for (size_t i = 0; i < newPatterns.size(); i++)
	{
		if (maxPatternsSize.width < newPatterns[i].cols)maxPatternsSize.width = newPatterns[i].cols;
		if (maxPatternsSize.height < newPatterns[i].rows)maxPatternsSize.height = newPatterns[i].rows;
	}
	cv::Size targetSize; 
	if (frontPatternsExtend.size()>0)
	{
		cv::Size existPatternSize = frontPatternsExtend[0].size();
		if (existPatternSize.width < maxPatternsSize.width || existPatternSize.height < maxPatternsSize.height)
		{
			targetSize= cv::Size(maxPatternsSize.width + LABEL_EXPAND_MARGIN,
				maxPatternsSize.height + LABEL_EXPAND_MARGIN);
			for (size_t i = 0; i < frontPatternsExtend.size(); i++)
			{ 
				cv::Mat temp = cv::Mat::zeros(targetSize, frontPatternsExtend[i].type());
				std::cout<<1<<std::endl;
				frontPatternsExtend[i].copyTo(temp(cv::Rect(cv::Point(LABEL_EXPAND_MARGIN_HALF, LABEL_EXPAND_MARGIN_HALF), frontPatternsExtend[i].size())));
				frontPatternsExtend[i] = temp;
			}
		}
		else
		{
			targetSize = existPatternSize; 
		}
	} 
	else
	{
			targetSize= cv::Size(maxPatternsSize.width + LABEL_EXPAND_MARGIN, maxPatternsSize.height + LABEL_EXPAND_MARGIN);
	}

	for (int newId = 0; newId < newPatterns.size(); newId++)
	{
		const cv::Mat& patternLocal = newPatterns[newId];
		const cv::Mat& patternLocalMask = newPatternMask[newId];
		std::vector<cv::Mat> patternLocal_rgb;
		cv::split(patternLocal, patternLocal_rgb);
#if templateMtach>0
		double maxMatch = -1;
		int maxMatchLabel = -1;
		std::cout<<"this pattern idx = "<< newId << std::endl;
		for (int oldId = 0; oldId < frontPatternsExtend.size()/3; oldId++)
		{
			cv::Mat matchResult0, matchResult1, matchResult2;
			cv::matchTemplate(frontPatternsExtend[3*oldId],   patternLocal_rgb[0], matchResult0, templateMtachMethod, patternLocalMask);
			cv::matchTemplate(frontPatternsExtend[3*oldId+1], patternLocal_rgb[1], matchResult1, templateMtachMethod, patternLocalMask);
			cv::matchTemplate(frontPatternsExtend[3*oldId+2], patternLocal_rgb[2], matchResult2, templateMtachMethod, patternLocalMask);
			cv::Mat matchResult = (matchResult0 + matchResult1 + matchResult2) / 3;
			double maxMatchTemp = 0;
			cv::Point matchPosition;
			cv::minMaxLoc(matchResult, 0, &maxMatchTemp,0,&matchPosition);
			LOG(INFO) << oldId<<"  :  "<<maxMatchTemp;
			if (maxMatchTemp > maxMatch && maxMatchTemp > MATCH_THRE)
			{
				cv::Mat diff0 = diffMask(frontPatternsExtend[3 * oldId + 0](cv::Rect(matchPosition, patternLocal_rgb[0].size())), patternLocal_rgb[0], patternLocalMask);
				cv::Mat diff1 = diffMask(frontPatternsExtend[3 * oldId + 1](cv::Rect(matchPosition, patternLocal_rgb[1].size())), patternLocal_rgb[1], patternLocalMask);
				cv::Mat diff2 = diffMask(frontPatternsExtend[3 * oldId + 2](cv::Rect(matchPosition, patternLocal_rgb[2].size())), patternLocal_rgb[2], patternLocalMask);

				float d0 = avgMat(diff0);
				float d1 = avgMat(diff1);
				float d2 = avgMat(diff2);
				LOG(INFO) << labels[oldId] << "  " << d0 << " " << d1 << " " << d2;
				if (d0 + d1 + d2 < COLOR_DIFF_THRE)
				{
					maxMatch = maxMatchTemp;
					maxMatchLabel = labels[oldId];
				}
			}
		}
		if (maxMatchLabel < 0)
		{
			labels.emplace_back(newLabel);
			cv::Mat temp = cv::Mat::zeros(targetSize, patternLocal.type());
			std::cout << 2 << std::endl;
			patternLocal.copyTo(temp(cv::Rect(cv::Point(LABEL_EXPAND_MARGIN_HALF, LABEL_EXPAND_MARGIN_HALF), patternLocal.size())));
			std::vector<cv::Mat> temp_rgb;
			cv::split(temp, temp_rgb);
			frontPatternsExtend.emplace_back(temp_rgb[0]);
			frontPatternsExtend.emplace_back(temp_rgb[1]);
			frontPatternsExtend.emplace_back(temp_rgb[2]);
			localLabel[newId] = newLabel;
			newLabel++;
		}
		else
		{
			localLabel[newId] = maxMatchLabel;
		}

		cv::imwrite("label_" + std::to_string(localLabel[newId]) + "_" + std::to_string(newId) + ".bmp", patternLocal);
#endif
#if USE_SIFT>0 
		std::vector<cv::KeyPoint> keypoints;
		extractor->detect(patternLocal, keypoints); 
		cv::Mat descriptors;
		extractor->compute(patternLocal, keypoints, descriptors);
		float nearestDistance = std::numeric_limits<float>::max();
		int nearestId = -1;
		for (int oldId = 0; oldId < descriptorss.size(); oldId++)
		{
			std::vector<cv::DMatch> matches;
			matcher.match(descriptorss[oldId], descriptors, matches);
			std::sort(matches.begin(), matches.end(), [](const auto& a, const auto& b) {return a.distance < b.distance; });
			if (matches.size()>0 )
			{
				for (int i = 0; i < matches.size(); i++)
				{
					auto a = keypointss[oldId][matches[i].queryIdx].pt;
					auto b = keypoints[matches[i].trainIdx].pt; 
					if (cv::norm(a-b)<8 && matches[i].distance< nearestDistance)
					{
						nearestDistance = matches[i].distance;
						nearestId = oldId;
						break;
					}
				}
				
			}
			LOG(INFO) << oldId << " " << nearestDistance<<" "<< nearestId;
		}
		LOG(INFO);
		if (nearestId<0 || nearestDistance>MATCH_SIFT_THRE)
		{ 
			keypointss.emplace_back(keypoints);
			descriptorss.emplace_back(descriptors); 
			labels.emplace_back(newLabel); 
			localLabel[newId] = newLabel;
			newLabel++;
		} 
		else
		{
			localLabel[newId] = labels[nearestId];
		}
#endif // USE_SIFT>0


		


		
	}
	patternSize = targetSize;
}


int Label::checkOrRegister(const bool& doRegister, const cv::Mat& newPattern, const cv::Mat& mask, const double& thre, const double& color_thre)
{
	int thisLabel = -1;
	if (newPattern.cols > patternSize.width || newPattern.rows > patternSize.height)
	{
		cv::Size targetSize;
		targetSize.width = std::max(newPattern.cols, patternSize.width) + 2;
		targetSize.height = std::max(newPattern.rows, patternSize.height) + 2;
		for (size_t i = 0; i < frontPatternsExtend.size(); i++)
		{
			cv::Mat temp = cv::Mat::zeros(targetSize, frontPatternsExtend[i].type());
			frontPatternsExtend[i].copyTo(temp(cv::Rect(cv::Point(1, 1), frontPatternsExtend[i].size())));
			frontPatternsExtend[i] = temp;
		}
		patternSize = targetSize;
	}
	const cv::Mat& patternLocal = newPattern;
	const cv::Mat& patternLocalMask = mask;

#if templateMtach>0
	std::vector<cv::Mat> patternLocal_rgb;
	cv::split(patternLocal, patternLocal_rgb);
	double maxMatch = -1;
	int maxMatchLabel = -1;
	for (int oldId = 0; oldId < frontPatternsExtend.size()/3; oldId++)
	{
		cv::Mat matchResult, matchResult0, matchResult1, matchResult2; 

		cv::matchTemplate(frontPatternsExtend[3 * oldId + 0], patternLocal_rgb[0], matchResult0, templateMtachMethod, patternLocalMask);
		cv::matchTemplate(frontPatternsExtend[3 * oldId + 1], patternLocal_rgb[1], matchResult1, templateMtachMethod, patternLocalMask);
		cv::matchTemplate(frontPatternsExtend[3 * oldId + 2], patternLocal_rgb[2], matchResult2, templateMtachMethod, patternLocalMask);

		matchResult = (matchResult0 + matchResult1 + matchResult2) / 3;
		double maxMatchTemp = 0;
		cv::Point matchPosition;
		cv::minMaxLoc(matchResult, 0, &maxMatchTemp,0,&matchPosition);
		LOG(INFO) << maxMatchTemp << "  " << oldId;
		if (maxMatchTemp > maxMatch && maxMatchTemp > thre)
		{
			cv::Mat diff0 = diffMask(frontPatternsExtend[3 * oldId + 0](cv::Rect(matchPosition, patternLocal_rgb[0].size())), patternLocal_rgb[0], patternLocalMask);
			cv::Mat diff1 = diffMask(frontPatternsExtend[3 * oldId + 1](cv::Rect(matchPosition, patternLocal_rgb[1].size())), patternLocal_rgb[1], patternLocalMask);
			cv::Mat diff2 = diffMask(frontPatternsExtend[3 * oldId + 2](cv::Rect(matchPosition, patternLocal_rgb[2].size())), patternLocal_rgb[2], patternLocalMask);
			 
			float d0 = avgMat(diff0);
			float d1 = avgMat(diff1);
			float d2 = avgMat(diff2); 
			LOG(INFO)<<labels[oldId]<<"  " << d0 << " " << d1 << " " << d2;
			if (d0+d1+d2< color_thre)
			{ 
				maxMatch = maxMatchTemp;
				maxMatchLabel = labels[oldId];
			}
		}
	}
	if (doRegister)
	{

		if (maxMatchLabel < 0)
		{
			labels.emplace_back(newLabel);
			cv::Mat temp = cv::Mat::zeros(patternSize, patternLocal.type());
			patternLocal.copyTo(temp(cv::Rect(cv::Point(1, 1), patternLocal.size())));
			std::vector<cv::Mat> temp_rgb;
			cv::split(temp, temp_rgb);
			frontPatternsExtend.emplace_back(temp_rgb[0]);
			frontPatternsExtend.emplace_back(temp_rgb[1]);
			frontPatternsExtend.emplace_back(temp_rgb[2]);
			thisLabel = newLabel;
			newLabel++;
		}
		else
		{
			thisLabel = maxMatchLabel;
		}
	}
	else
	{
		thisLabel = maxMatchLabel;
	}
#endif
#if USE_SIFT>0
	std::vector<cv::KeyPoint> keypoints;
	extractor->detect(patternLocal, keypoints, patternLocalMask);
	cv::Mat descriptors;
	extractor->compute(patternLocal, keypoints, descriptors);
	float nearestDistance = std::numeric_limits<float>::max();
	int nearestId = -1;
	for (int oldId = 0; oldId < descriptorss.size(); oldId++)
	{
		std::vector<cv::DMatch> matches;
		matcher.match(descriptorss[oldId], descriptors, matches);
		std::sort(matches.begin(), matches.end(), [](const auto& a, const auto& b) {return a.distance < b.distance; });
		if (matches.size() > 0)
		{
			for (int i = 0; i < matches.size(); i++)
			{
				auto a = keypointss[oldId][matches[i].queryIdx].pt;
				auto b = keypoints[matches[i].trainIdx].pt;
				if (cv::norm(a - b) < 8 && matches[i].distance < nearestDistance)
				{
					nearestDistance = matches[i].distance;
					nearestId = oldId;
					break;
				}
			}

		}
		LOG(INFO) << oldId << " " << nearestDistance << " " << nearestId;
	}
	if (doRegister)
	{ 
		if (nearestId<0 || nearestDistance>thre)
		{
			keypointss.emplace_back(keypoints);
			descriptorss.emplace_back(descriptors);
			labels.emplace_back(newLabel);
			thisLabel = newLabel;
			newLabel++;
		}
		else
		{
			thisLabel = labels[nearestId];
		}
	}
	else
	{
		if (nearestId>=0 && nearestDistance <= thre)
		{
			thisLabel = labels[nearestId];
		}
	}
#endif 
	return thisLabel;
}
