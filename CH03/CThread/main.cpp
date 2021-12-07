#include "CThread.h"


class Thread: public CThread
{
protected:
	virtual void Run(LPVOID lpParameter = 0) override;
};


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	}
}


void Thread::Run(LPVOID lpParameter /*= 0*/)
{
	WNDCLASSEX wnd = { 0 };

	wnd.cbClsExtra = 0;
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.cbWndExtra = 0;
	wnd.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hInstance = GetModuleHandle(NULL);
	wnd.lpfnWndProc = WindowProcedure;
	wnd.lpszClassName = (LPCTSTR)this->lpParameter;
	wnd.lpszMenuName = NULL;
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	if (!RegisterClassEx(&wnd)) return;

	HWND hwnd = CreateWindow(wnd.lpszClassName, L"Base Thread Management", WS_OVERLAPPEDWINDOW,
		200, 200, 800, 600, HWND_DESKTOP, NULL, wnd.hInstance, NULL);
	
	UpdateWindow(hwnd);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = { 0 };
	while (GetMessage(&msg, hwnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(wnd.lpszClassName, wnd.hInstance);
}


int main()
{
	Thread thread1;
	thread1.Create((LPVOID)L"WND_CLASS_1");

	Thread thread2;
	thread2.Create((LPVOID)L"WND_CLASS_2");

	thread1.Join();
	thread2.Join();

	return 0;
}
