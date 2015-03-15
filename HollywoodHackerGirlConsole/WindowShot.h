#pragma once
#include <Windows.h>

struct ScreenShotResult
{
	HBITMAP hBitmap;
	int width, height;
};

void GetWindowSize(HWND hWnd, int &width, int &height);
void TakeScreenShot(HWND hWnd, ScreenShotResult& result);