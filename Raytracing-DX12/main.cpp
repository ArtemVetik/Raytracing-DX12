#include <Windows.h>
#include <crtdbg.h>
#include <sstream>

#include "../RenderEngine/IRenderEngine.h"
#include "../RenderEngine/Window.h"
#include "../RenderEngine/Timer.h"
#include "../Core/InputSystem/InputManager.h"

using namespace RaytracingDX12;
using namespace EduEngine;

void UpdateWindowTitle(HWND window, int rFps, float rMspf)
{
	std::wstringstream out;
	out.precision(6);

	out << "Raytracing - DX12 (" << " fps: " << rFps << " frame time: " << rMspf << " ms)";

	SetWindowText(window, out.str().c_str());
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	FILE* fp;

	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

	Window window(hInstance, 1280, 720);
	window.Initialize();

	Timer timer(window.GetMainWindow(), L"Raytracing-DX12!");

	InputManager::GetInstance().Initialize(hInstance, window.GetMainWindow());

	auto renderEngine = IRenderEngine::Create(window);

	MSG msg = { 0 };
	int fps;
	float mspf;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.UpdateTimer();

			InputManager::GetInstance().Update();

			if (!window.IsPaused())
			{
				if (timer.UpdateTitleBarStats(fps, mspf))
					UpdateWindowTitle(window.GetMainWindow(), fps, mspf);

				renderEngine->Update(timer);
				renderEngine->Render();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}