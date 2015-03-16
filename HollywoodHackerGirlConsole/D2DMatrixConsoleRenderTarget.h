#pragma once
#include "d2dSetup.h"

class D2DMatrixConsoleRenderTarget
{
private:
	float greenValue;
	HWND hWnd;
	HWND hWndConsole;
	Microsoft::WRL::ComPtr<ID3D11Device1> pD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pD3DContext;
	Microsoft::WRL::ComPtr<ID2D1Device> pD2DDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> pD2DSwapChain;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> pD2DBackBuffer;
	Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> pD2DContext;
	Microsoft::WRL::ComPtr<IWICImagingFactory2> pWicImageFactory;
	Microsoft::WRL::ComPtr<ID2D1Effect> bitmapSourceEffect;
	Microsoft::WRL::ComPtr<ID2D1Effect> matrixEffect;

	void CreateRenderTargetWindow(HWND hWndConsole, int width, int height);
	void SetupDirect2D(int width, int height);
	void Present();

public:
	D2DMatrixConsoleRenderTarget(HWND hWndConsole);
	void PostQuit(){ PostQuitMessage(0); }
	void IncrementGreen(){ greenValue -= (1/42.0f); if(greenValue < 0.0f) greenValue = 1.0f; };
	void GiveConsoleFocus(){SetForegroundWindow(hWndConsole);}
	void Draw();
};