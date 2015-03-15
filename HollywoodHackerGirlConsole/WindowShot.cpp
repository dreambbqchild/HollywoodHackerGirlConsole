#include "WindowShot.h"

void GetWindowSize(HWND hWnd, int &width, int &height)
{
	RECT rect;
	if(GetWindowRect(hWnd, &rect))
	{
	  width = rect.right - rect.left;
	  height = rect.bottom - rect.top;
	}
}

void TakeScreenShot(HWND hWnd, ScreenShotResult& result)
{
	HDC hdc = GetWindowDC(hWnd);
	HBITMAP hbitmap = 0;
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			GetWindowSize(hWnd, result.width, result.height);

			result.hBitmap = CreateCompatibleBitmap(hdc, result.width, result.height);
			if (result.hBitmap)
			{
				SelectObject(hdcMem, result.hBitmap);
				PrintWindow(hWnd, hdcMem, 0);
			}
			DeleteObject(hdcMem);
		}
		ReleaseDC(hWnd, hdc);
	}

	//auto hDCWnd = GetDC(hWnd);
	//auto hDCMem = CreateCompatibleDC(hDCWnd);
	//int width = 0, height = 0;
	//GetWindowSize(hWnd, width, height);

	//auto hBitmap = CreateCompatibleBitmap(hDCWnd, width, height);

	//SelectObject(hDCMem, hBitmap);
	//auto result = BitBlt(hDCMem, 0, 0, width, height, hDCWnd, 0, 0, SRCCOPY);

	//SaveHBITMAP(hDCMem, hBitmap);

	//ReleaseDC(hWnd, hDCWnd);
	//DeleteDC(hDCMem);
	//DeleteObject(hBitmap);

	//return result;
}