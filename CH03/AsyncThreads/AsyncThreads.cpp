#include "CThread.h"
#include <windowsx.h>
#include <tchar.h>
#include <time.h>

#define THREADS_NUMBER	4
#define WINDOWS_NUMBER	10
#define SLEEP_TIME		500


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


class Thread : public CThread
{
protected:
	virtual void Run(LPVOID lpParameter = 0) override;
};


void Thread::Run(LPVOID lpParameter)
{
	// 随机种子生成器
	srand((unsigned)time(NULL));
	HWND hwnd = (HWND)GetUserData();
	RECT rect;

	// 获得窗口的尺寸
	BOOL bError = GetClientRect(hwnd, &rect);
	if (!bError) return;

	int iClientX = rect.right - rect.left;
	int iClientY = rect.bottom - rect.top;

	// 如果窗口没有尺寸，不绘图
	if ((!iClientX) || (!iClientY)) return;

	// 获取设备上下文以绘图
	HDC hdc = GetDC(hwnd);

	if (hdc)
	{
		// 绘制 10 个随机图形
		for (int iCount = 0; iCount < WINDOWS_NUMBER; ++iCount)
		{
			// 设置坐标
			int iStartX = (int)(rand() % iClientX);
			int iStopX = (int)(rand() % iClientX);
			int iStartY = (int)(rand() % iClientY);
			int iStopY = (int)(rand() % iClientY);
			
			// 设置颜色
			int iRed = rand() & 255;
			int iGreen = rand() & 255;
			int iBlue = rand() & 255;

			// 创建画刷
			HANDLE hBrush = CreateSolidBrush(GetNearestColor(hdc, RGB(iRed, iGreen, iBlue)));
			HANDLE hbrOld = SelectBrush(hdc, hBrush);
			Rectangle(hdc, min(iStartX, iStopX), max(iStartX, iStopX), min(iStartY, iStopY), max(iStartY, iStopY));

			// 删除画刷
			DeleteBrush(hdc, hbrOld);
		}

		// 释放设备上下文
		ReleaseDC(hwnd, hdc);
	}

	Sleep(SLEEP_TIME);
	return;
}



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPTSTR scCommandLine, int iCmdShow)
{
	WNDCLASSEX wndEx = { 0 };

	wndEx.hInstance = hInstance;
	wndEx.lpszClassName = _T("async_thread");
	wndEx.lpszMenuName = NULL;
	wndEx.lpfnWndProc = WindowProcedure;
	wndEx.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wndEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndEx.cbWndExtra = 0;
	wndEx.cbClsExtra = 0;
	wndEx.cbSize = sizeof(WNDCLASSEX);
	wndEx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	if (!RegisterClassEx(&wndEx)) return 1;

	HWND hwnd = CreateWindow(wndEx.lpszClassName, TEXT("Basic Thread Management"), WS_OVERLAPPEDWINDOW,
		200, 200, 840, 440, HWND_DESKTOP, NULL, wndEx.hInstance, NULL);

	HWND hRects[THREADS_NUMBER];

	hRects[0] = CreateWindow(_T("STATIC"), _T(""), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		20, 20, 180, 350, hwnd, NULL, hInstance, NULL);

	hRects[1] = CreateWindow(_T("STATIC"), _T(""), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		220, 20, 180, 350, hwnd, NULL, hInstance, NULL);

	hRects[2] = CreateWindow(_T("STATIC"), _T(""), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		420, 20, 180, 350, hwnd, NULL, hInstance, NULL);

	hRects[3] = CreateWindow(_T("STATIC"), _T(""), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		620, 20, 180, 350, hwnd, NULL, hInstance, NULL);
	
	UpdateWindow(hwnd);
	ShowWindow(hwnd, SW_SHOW);

	Thread threads[THREADS_NUMBER];
	for (int iIndex = 0; iIndex < THREADS_NUMBER; ++iIndex) {
		threads[iIndex].Create(NULL, STATE_ASYNC | STATE_CONTINUOUS);
		threads[iIndex].SetUserData(hRects[iIndex]);
	}

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)threads);

	MSG msg = { 0 };
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(wndEx.lpszClassName, wndEx.hInstance);
	return 0;
}


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
		Thread* thread = (Thread*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		thread->Alert();
		for (int iIndex = 0; iIndex < THREADS_NUMBER; ++iIndex) {
			thread[iIndex].Join();
		}
		DestroyWindow(hwnd);
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	}

	return 0;
}