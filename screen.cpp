#include"screen.h"

#ifdef _MSC_VER
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
//#define _AFXDLL//为了方便是用mfc类  
#include<afxwin.h>  
#include<chrono>
#include<thread>
#include<iostream>
#include<string>
#include<conio.h>
#include<windows.h>
#include <string>
#include<opencv2/opencv.hpp>



std::string ws2s(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char* _Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	std::string result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

static LPVOID    screenCaptureData = NULL;
static HBITMAP hBitmap;
static HDC hDDC;
static HDC hCDC;
static int nWidth;
static int nHeight;
int initGDI(int* screenHeight, int* screenWidth)
{
	nWidth = GetSystemMetrics(SM_CXSCREEN);//得到屏幕的分辨率的x  
	nHeight = GetSystemMetrics(SM_CYSCREEN);//得到屏幕分辨率的y  
	screenCaptureData = new char[nWidth * nHeight * 4];
	memset(screenCaptureData, 0, nWidth);
	// Get desktop DC, create a compatible dc, create a comaptible bitmap and select into compatible dc.  
	hDDC = GetDC(GetDesktopWindow());//得到屏幕的dc  
	hCDC = CreateCompatibleDC(hDDC);//  
	hBitmap = CreateCompatibleBitmap(hDDC, nWidth, nHeight);//得到位图  
	SelectObject(hCDC, hBitmap); //好像总得这么写。  
	std::cout << "screen : [" << nHeight << " , " << nWidth << "]" << std::endl;

	*screenHeight = nHeight;
	*screenWidth = nWidth;

	return 0;
}
int screenCapture(void* cvMatPtr)
//cv::Mat gdiScreenCapture()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	BitBlt(hCDC, 0, 0, nWidth, nHeight, hDDC, 0, 0, SRCCOPY);
	GetBitmapBits(hBitmap, nWidth * nHeight * 4, screenCaptureData);//得到位图的数据，并存到screenCaptureData数组中。  
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	cv::Mat img = cv::Mat(cv::Size(nWidth, nHeight), CV_8UC4);//创建一个rgba格式的IplImage,内容为空  
	memcpy(img.data, screenCaptureData, nWidth * nHeight * 4);//这样比较浪费时间，但写的方便。这里必须得是*4，上面的链接写的是*3，这是不对的。  
	cv::Mat& img2 = *(cv::Mat*)cvMatPtr;
	cv::cvtColor(img, img2, cv::COLOR_BGRA2BGR);
	return 0;
}

int keyInApi(int key0, int key1)
{
	//keybd_event(VK_LWIN, 0, 0, 0);
	//keybd_event('M', 0, 0, 0);
	//keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
	//keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
	if (key1 >= 0)
	{
		keybd_event(key0, 0, 0, 0);
		keybd_event(key1, 0, 0, 0);
		Sleep(500);
		keybd_event(key1, 0, 2, 0);
		keybd_event(key0, 0, 2, 0);
		//
	}
	else
	{
		keybd_event(key0, 0, 0, 0);
		keybd_event(key0, 0, 2, 0);
		Sleep(100);
	}
	return 0;
}
int keyInLongTimeApi(int key0, int key1, int milliseconds)
{
	if (key1 >= 0)
	{
		keybd_event(key0, 0, 0, 0);
		keybd_event(key1, 0, 0, 0);
		Sleep(milliseconds);
		keybd_event(key1, 0, 2, 0);
		keybd_event(key0, 0, 2, 0);
		//
	}
	else
	{
		keybd_event(key0, 0, 0, 0);
		std::this_thread::sleep_for(std::chrono::seconds(1000));
		keybd_event(key0, 0, 2, 0);
	}
	return 0;
}
int commentInApi(const char* comment_)
{
	std::string comment(comment_);
	if (comment.length() > 900)
	{
		std::cout << "comment too long" << std::endl;
		return -1;
	}
	HWND hWnd = NULL;
	if (OpenClipboard(hWnd))//打开剪切板
	{
		EmptyClipboard();//清空剪切板
		HANDLE hHandle = GlobalAlloc(GMEM_FIXED, 1000);//分配内存
		char* pData = (char*)GlobalLock(hHandle);//锁定内存，返回申请内存的首地址
		strcpy(pData, comment.data());
		SetClipboardData(CF_TEXT, hHandle);//设置剪切板数据
		GlobalUnlock(hHandle);//解除锁定
		CloseClipboard();//关闭剪切板
		keyInApi(VK_LCONTROL, 'V');
	}
	else
	{
		std::cout << "OpenClipboard err" << std::endl;
		return -1;
	}
	return 0;
}
int commentInApi2(const char* comment_)
{
	std::string comment(comment_);
	if (comment.length() > 900)
	{
		std::cout << "comment too long" << std::endl;
		return -1;
	}
	HWND hWnd = NULL;
	if (OpenClipboard(hWnd))//打开剪切板
	{
		EmptyClipboard();//清空剪切板
		HANDLE hHandle = GlobalAlloc(GMEM_FIXED, 1000);//分配内存
		char* pData = (char*)GlobalLock(hHandle);//锁定内存，返回申请内存的首地址
		strcpy(pData, comment.data());
		SetClipboardData(CF_TEXT, hHandle);//设置剪切板数据
		GlobalUnlock(hHandle);//解除锁定
		CloseClipboard();//关闭剪切板 
	}
	else
	{
		std::cout << "OpenClipboard err" << std::endl;
		return -1;
	}
	return 0;
}
int commentInApi3(void* str)
{
	std::string& ss = *(std::string*)str;
	HWND hWnd = NULL;
	if (OpenClipboard(hWnd))//打开剪切板
	{
		UINT uiFormat = (sizeof(TCHAR) == sizeof(WCHAR)) ? CF_UNICODETEXT : CF_TEXT;
		HGLOBAL hMem = GetClipboardData(uiFormat);
		if (hMem != NULL)
		{
			//获取UNICODE的字符串。
			LPCTSTR lpStr = (LPCTSTR)GlobalLock(hMem);
			if (lpStr != NULL)
			{
				std::wstring texts;
				texts = lpStr;
				ss = ws2s(texts);
			}
			GlobalUnlock(hMem);
		}
		CloseClipboard();//关闭剪切板 
	}
	else
	{
		std::cout << "OpenClipboard err" << std::endl;
		return -1;
	}
	return 0;
}
int setMousePos(float x, float y)
{
	std::cout << "setMousePos : [" << int(x) << " , " << int(y) << "]" << std::endl;
	SetCursorPos(int(x), int(y));
	return 0;
}

int setMouseLeftChick(int milliseconds)
{
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	return 0;
}
int setMouseRighttChick(int milliseconds)
{
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	return 0;
}

int leftDrag(int startX, int startY, int endX, int endY)
{
	setMousePos(startX, startY);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	setMousePos(endX, endY);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return 0;
}