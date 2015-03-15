#include <Windows.h>
#include <ppltasks.h>
#include "D2DMatrixConsoleRenderTarget.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

using namespace concurrency;

int main(int argc, const char* argv)
{
	int result = 0;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	{
		auto systemTask = create_task([]() { return system("launch.bat"); });
		auto hConsole = GetConsoleWindow();

		D2DMatrixConsoleRenderTarget target(hConsole);

		MSG msg; BOOL bRet;
		while((bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{			
			// Translate the message and dispatch it to WindowProc()
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		result = systemTask.get();
	}
	CoUninitialize();
	return result;
}