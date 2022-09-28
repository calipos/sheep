#pragma once
#define _AFXDLL
__declspec(dllexport) int initGDI(int* screenHeight, int* screenWidth);
__declspec(dllexport) int screenCapture(void* cvMatPtr);
__declspec(dllexport) int keyInApi(int key0, int key1 = -1);
__declspec(dllexport) int keyInLongTimeApi(int key0, int key1 = -1, int milliseconds = 500);
__declspec(dllexport) int commentInApi(const char* comment_);
__declspec(dllexport) int commentInApi2(const char* comment_);
__declspec(dllexport) int commentInApi3(void* str);
__declspec(dllexport) int setMousePos(float x, float y);
__declspec(dllexport) int setMouseLeftChick(int milliseconds = 100);
__declspec(dllexport) int setMouseRighttChick(int milliseconds = 100);
__declspec(dllexport) int leftDrag(int startX, int startY, int endX, int endY);