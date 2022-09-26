#pragma once
#include<iostream>
#include <vector>
#include <list>
#include <tuple>
#include "opencv2/opencv.hpp"
#include "glog/logging.h"
#include "common.h"
struct Label
{
	Label();
#if USE_SIFT>0
	cv::BFMatcher matcher;
	cv::Ptr<cv::Feature2D>extractor;
	std::vector < std::vector<cv::KeyPoint>> keypointss;
	std::vector<cv::Mat> descriptorss;
#endif // USE_SIFT>0

	

	std::vector<cv::Mat>frontPatternsExtend;
	std::vector<int>labels;
	int newLabel{ 0 };
	cv::Size patternSize;
	void updata(const std::vector<cv::Mat>& newPatterns, const std::vector<cv::Mat>& newPatternMask, std::vector<int>& localLabel);
	int checkOrRegister(const bool & doRegister,const cv::Mat& newPattern, const cv::Mat& mask,const double& thre = MATCH_THRE, const double& color_thre = COLOR_DIFF_THRE_3_4);
};
