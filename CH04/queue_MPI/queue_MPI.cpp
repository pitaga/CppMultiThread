#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "CQueue.h"
#include <CommCtrl.h>

#pragma comment(lib, "comctl32.lib")

#define BUFFER_SIZE		4096
#define LABEL_TEXT		100

#define WM_PLOTDATA		WM_USER		+ 1
#define WM_ENDTASK		WM_PLOTDATA	+ 1

#define THREAD_NUMBER	4
#define MAX_MSGCOUNT	10

#define CALCULATION_TIME	1000
#define DRAWING_TIME		2300

#define EVENT_NAME		_T("__t_event__")


typedef struct tagPLOTDATA {
	int value;
	DWORD dwThreadId;
	int iMsgID;
} PLOTDATA, *PPLOTDATA;

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI StartAddress(LPVOID lpParameter);
DWORD WINAPI DrawPlot(LPVOID lpParameter);

int iMessageID = 0;
HANDLE gEvent = NULL;
HANDLE hThreads[THREAD_NUMBER];
HANDLE hThread = NULL;

CRITICAL_SECTION cs;
CQueue<PLOTDATA> queue;


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE preInstance, LPTSTR command, int wndShow)
{
	UNREFERENCED_PARAMETER(preInstance);
	UNREFERENCED_PARAMETER(command);

	InitializeCriticalSection(&cs);

	TCHAR windowClass[] = _T("_ _basic_MPI_wnd_class_ _");

	WNDCLASSEX wndEx = { 0 };
	wndEx.cbSize = sizeof(WNDCLASSEX);
	wndEx.style = CS_HREDRAW | CS_VREDRAW;
	wndEx.lpfnWndProc = WindowProcedure;
	wndEx.cbClsExtra = 0;
	wndEx.cbWndExtra = 0;
	wndEx.hInstance = hInstance;
	wndEx.hIcon = LoadIcon(wndEx.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wndEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndEx.lpszMenuName = NULL;
	wndEx.lpszClassName = windowClass;
	wndEx.hIconSm = LoadIcon(wndEx.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wndEx)) {
		return 1;
	}

	InitCommonControls();

	HWND hWnd = CreateWindow(windowClass, _T("Basic Message Passing Interface"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 200, 200, 440,
		300, NULL, NULL, wndEx.hInstance, NULL);

	if (!hWnd) {
		return NULL;
	}

	HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		BALTIC_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Microsoft Sans Serif"));

	HWND hText = CreateWindow(_T("STATIC"), NULL, WS_CHILD | WS_VISIBLE |
		SS_LEFT | WS_BORDER, 15, 20, 390, 220, hWnd, (HMENU)LABEL_TEXT,
		wndEx.hInstance, NULL);

	SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);

	ShowWindow(hWnd, wndShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(wndEx.lpszClassName, wndEx.hInstance);

	WaitForMultipleObjects(THREAD_NUMBER, hThreads, TRUE, INFINITE);

	for (int iIndex = 0; iIndex < THREAD_NUMBER; iIndex++)
	{
		CloseHandle(hThreads[iIndex]);
	}

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(gEvent);

	DeleteCriticalSection(&cs);

	return (int)msg.wParam;
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		gEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
		for (int iIndex = 0; iIndex < THREAD_NUMBER; ++iIndex) {
			hThreads[iIndex] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartAddress, hwnd, 0, NULL);
		}
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawPlot, hwnd, 0, NULL);
		break;
	}

	case WM_PLOTDATA:
	{
		PPLOTDATA pData = (PPLOTDATA)lParam;
		HWND hLabel = GetDlgItem(hwnd, LABEL_TEXT);
		TCHAR buffer[BUFFER_SIZE];
		GetWindowText(hLabel, buffer, BUFFER_SIZE);
		wsprintf(buffer, _T("%ws\n\n\tMessage has been received. Msg ID:\t%d"), buffer, pData->iMsgID);
		SetWindowText(hLabel, buffer);
		break;
	}

	case WM_ENDTASK:
	{
		HWND label = GetDlgItem(hwnd, LABEL_TEXT);
		TCHAR buffer[BUFFER_SIZE];
		wsprintf(buffer, _T("\n\n\tPlot is drawn. You can close the window now."));
		SetWindowText(label, buffer);
		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	}
	return 0;
}


DWORD WINAPI StartAddress(LPVOID lpParameter)
{
	HWND hwnd = (HWND)lpParameter;
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVENT_NAME);

	if (hEvent != NULL) {
		SetEvent(hEvent);
	}

	CloseHandle(hEvent);

	int iCount = 0;
	while (iCount++ < MAX_MSGCOUNT) {
		// 执行计算
		Sleep(CALCULATION_TIME);
		// 把结果放入 PLOTDATA 结构中
		PPLOTDATA pData = new PLOTDATA();
		pData->value = (rand() % 0xFFFF) - iMessageID;
		pData->dwThreadId = GetCurrentThreadId();
		pData->iMsgID = ++iMessageID;

		EnterCriticalSection(&cs);
		queue.Enqueue(pData);
		LeaveCriticalSection(&cs);

		PostMessage(hwnd, WM_PLOTDATA, 0, (LPARAM)pData);
	}
	return 0L;
}


DWORD WINAPI DrawPlot(LPVOID lpParameter)
{
	HWND hwnd = (HWND)lpParameter;
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, EVENT_NAME);

	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);

	int iCount = 0;
	
	while (iCount < MAX_MSGCOUNT * THREAD_NUMBER) {
		EnterCriticalSection(&cs);
		PPLOTDATA pData = queue.Dequeue();
		LeaveCriticalSection(&cs);

		if (!pData) break;

		// 执行绘制
		Sleep(DRAWING_TIME);

		HANDLE hLabel = GetDlgItem(hwnd, LABEL_TEXT);
		TCHAR buffer[BUFFER_SIZE];
		wsprintf(buffer,
#ifdef _UNICODE
			_T("\n\n\t%ws\t%u\n\t%ws\t%d\n\t%ws\t%d"),
#else
			_T("\n\n\t%s\t%u\n\t%s\t%d\n\t%s\t%d"),
#endif
			_T("Thread ID:"), (DWORD)pData->dwThreadId,
			_T("Current value:"), (int)pData->value,
			_T("Message ID:"), pData->iMsgID);

		SetWindowText((HWND)hLabel, buffer);

		delete pData;
	}

	PostMessage(hwnd, WM_ENDTASK, 0, 0);
	return 0L;
}


