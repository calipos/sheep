#pragma once
#define  MATCH_THRE (0.85)
#define  COLOR_DIFF_THRE (45)//3*25
#define  MATCH_THRE_3_4 (0.6)
#define  MATCH_THRE_2_4 (0.75)
#define  MATCH_THRE_1_4 (0.75)
#define  COLOR_DIFF_THRE_3_4 (75)//3*25
#define  COLOR_DIFF_THRE_2_4 (50)//2*25
#define  COLOR_DIFF_THRE_1_4 (25)//2*25
#define templateMtachMethod   (cv::TemplateMatchModes::TM_CCORR_NORMED)

#define  MATCH_SIFT_THRE (100)
#define  MATCH_SIFT_THRE_3_4 (300)

#define COVER_2_4_IOU_MIN (0.37)//0.5
#define COVER_2_4_IOU_MAX (0.62)//0.5
#define COVER_1_4_IOU_MIN (0.12)//0.25
#define COVER_1_4_IOU_MAX (0.37)//0.25

#define templateMtach 1
#define USE_ORB 0
#define USE_SIFT 0

#define MAX_PATTERN (7)
#define MAX_PATTERN_1 (MAX_PATTERN-1)

#define yes_ (4) 
#define no_ (-1) 

#define SHOW_COVERS 0
#include <iostream>
#include "opencv2/opencv.hpp"
struct Pattern
{
	int id{ -1 };
	int label{-1};
	cv::Rect r;
	int upperCnt{ 0 };
	int lowerCnt{ 0 };
	cv::Vec4i uppers;
	cv::Vec4i lowers;
	void insertUpper(const int& upper)
	{
		uppers[upperCnt++] = upper;
	}
	void insertlower(const int& lower)
	{
		lowers[lowerCnt++] = lower;
	}
	int hasUpper(const Pattern& potentialUpper)const 
	{
		if (upperCnt == 0)return -1; 
		for (size_t i = 0; i < upperCnt; i++)
		{
			if (uppers[i]== potentialUpper.id)
			{
				return i;
			}
		}
		return -1;
	}
	int hasLower(const Pattern& potentialLower)const
	{
		if (lowerCnt == 0)return -1; 
		for (int i = 0; i < lowerCnt; i++)
		{
			if (lowers[i] == potentialLower.id)
			{
				return i;
			}
		}
		return -1;
	}
	int eraseUpper(const int& upperId)
	{
		CHECK(upperCnt>0);
		upperCnt--;
		if (upperId == 3)return 0;
		for (int i = upperId; i < 4; i++)
		{
			uppers[i] = uppers[i+1];
		} 
		return 0;
	}
	int eraseLower(const int& lowerId)
	{
		CHECK(lowerCnt > 0);
		lowerCnt--;
		if (lowerId == 3)return 0;
		for (int i = lowerId; i < 4; i++)
		{
			lowers[i] = lowers[i + 1];
		}
		return 0;
	}
	static friend std::ostream& operator<<(std::ostream& o, const Pattern& p)
	{
		o << "id(" << p.id << "). label(" << p.label << "). upper[";
		for (int i = 0; i < p.upperCnt; i++)
		{
			o << p.uppers[i] << ",";
		}
		o <<  "]. lower[";
		for (int i = 0; i < p.lowerCnt; i++)
		{
			o << p.lowers[i] << ",";
		}
		o << "]";
		return  o;
	}
};
struct ScoreChain
{
	int pickfrontIdx{-100};
	int score{ 0 };
	ScoreChain* nextStep{nullptr};
	int nextStepCnt{0};
};