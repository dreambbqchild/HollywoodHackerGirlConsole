#include <Windows.h>
#include <ppltasks.h>
#include "D2DHollywoodConsoleRenderTarget.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

using namespace concurrency;

int main(int argc, const char* argv[])
{
	int result = 0;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	{
		const char* systemCall = "cmd";
		if (argc > 1)
			systemCall = argv[1];

		auto postQuit = false;
		auto systemTask = create_task([&postQuit, &systemCall]() { auto result = system(systemCall); postQuit = true; return result; });
		auto hConsole = GetConsoleWindow();
		ShowWindow(hConsole, SW_MAXIMIZE);

		D2DHollywoodConsoleRenderTarget target(hConsole);

		MSG msg; BOOL bRet;
		while((bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{			
			if(postQuit)
				target.PostQuit();

			// Translate the message and dispatch it to WindowProc()
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		result = systemTask.get();
	}
	CoUninitialize();
	return result;
}