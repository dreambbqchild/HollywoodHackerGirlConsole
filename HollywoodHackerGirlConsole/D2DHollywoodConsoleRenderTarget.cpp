#include "D2DHollywoodConsoleRenderTarget.h"
#include "D2DHollywoodEffect.h"
#include "WindowShot.h"

using namespace Microsoft::WRL;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	D2DHollywoodConsoleRenderTarget* target = (D2DHollywoodConsoleRenderTarget*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	if(uMsg == WM_CREATE || target)
	{
		switch(uMsg)
		{
			case WM_CREATE:
				target = (D2DHollywoodConsoleRenderTarget*)((LPCREATESTRUCT)lParam)->lpCreateParams;
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)target); 
				break;
			case WM_TIMER:
				target->IncrementGreen();
				InvalidateRect(hWnd, NULL, FALSE);
				break;
			case WM_SETFOCUS:
				target->GiveConsoleFocus();
				break;
			case WM_PAINT:
				target->Draw();
				break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

D3D_FEATURE_LEVEL featureLevels[] =
{
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1
};

D2DHollywoodConsoleRenderTarget::D2DHollywoodConsoleRenderTarget(HWND hWndConsole)
	: hWndConsole(hWndConsole), offsetValue(1.0f)
{
	int width, height;
	GetWindowSize(hWndConsole, width, height);
	CreateRenderTargetWindow(hWndConsole, width, height);
	SetupDirect2D(width, height);
}

void D2DHollywoodConsoleRenderTarget::CreateRenderTargetWindow(HWND hWndConsole, int width, int height)
{  
	// set default style
	auto style = WS_POPUP | WS_MINIMIZEBOX;

	const char CLASS_NAME[]  = "D2DHollywoodConsoleRenderTarget";
	WNDCLASSA wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

	// form for the image
	hWnd = CreateWindowExA(0, CLASS_NAME, NULL, style, 0, 0, width, height, NULL, NULL, GetModuleHandle(0), this);
	SetTimer(hWnd, 1, 24, (TIMERPROC) NULL);

	//Overlap Window
	RECT rect;
	GetWindowRect(hWndConsole, &rect);
	SetWindowPos(hWnd, nullptr, rect.top, rect.left, 0, 0, SWP_NOSIZE);

	SetWindowLong(hWnd, GWL_EXSTYLE, 0);
	ShowWindow(hWnd, SW_SHOW);

	SetWindowPos(hWndConsole, nullptr, -(width + 60), 0, 0, 0, SWP_NOSIZE);
}

void D2DHollywoodConsoleRenderTarget::SetupDirect2D(int width, int height)
{
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
 
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, &pD2DFactory);
	D2DHollywoodEffect::Register(pD2DFactory.Get());
	
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
 
	// Create Direct3D device and context
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	D3D_FEATURE_LEVEL returnedFeatureLevel;
 
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device, &returnedFeatureLevel, &context);

	device.As(&pD3DDevice);
	context.As(&pD3DContext);

	ComPtr<IDXGIDevice> dxgiDevice;
	pD3DDevice.As(&dxgiDevice);

	pD2DFactory->CreateDevice(dxgiDevice.Get(), &pD2DDevice);
	pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pD2DContext);

	//This could be in a resize method but I'm not caring about that right now.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);
 
	// Get the DXGI factory instance
	ComPtr<IDXGIFactory2> dxgiFactory;
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	dxgiFactory->CreateSwapChainForHwnd(pD3DDevice.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &pD2DSwapChain);

	ComPtr<IDXGISurface> dxgiBackBuffer;
	pD2DSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));

	FLOAT dpiX, dpiY;
	pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
 
	// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);
 
	pD2DContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, &pD2DBackBuffer);

	pD2DContext->SetTarget(pD2DBackBuffer.Get());

	//FINALLY!!! SETUP THE EFFECTS!!!
    pD2DContext->CreateEffect(CLSID_D2D1BitmapSource, &bitmapSourceEffect);
	pD2DContext->CreateEffect(CLSID_D2DHollywoodEffect, &hollywoodEffect);
	
	D2D_POINT_2F bounds = D2D1::Point2F(width, height);
	hollywoodEffect->SetValue(D2DHollywoodEffect::BOUNDS, bounds);
	hollywoodEffect->SetInputEffect(0, bitmapSourceEffect.Get());
}

void D2DHollywoodConsoleRenderTarget::Draw()
{
	ScreenShotResult result;
	TakeScreenShot(hWndConsole, result);

	hollywoodEffect->SetValue(D2DHollywoodEffect::OFFSET_VALUE, offsetValue);

	ComPtr<IWICBitmap> bitmapWindow;
	pWicImageFactory->CreateBitmapFromHBITMAP(result.hBitmap, nullptr, WICBitmapIgnoreAlpha, &bitmapWindow);
	DeleteObject(result.hBitmap);
    bitmapSourceEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, bitmapWindow.Get());

	pD2DContext->BeginDraw();
    pD2DContext->Clear( D2D1::ColorF(D2D1::ColorF::SkyBlue) );
	pD2DContext->DrawImage(hollywoodEffect.Get());
	pD2DContext->EndDraw();
	
	Present();
}

void D2DHollywoodConsoleRenderTarget::Present()
{
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;
 
	pD2DSwapChain->Present1(1, 0, &parameters);
}