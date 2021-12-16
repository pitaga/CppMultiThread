#include "CThread.h"


CLock::CLock()
{
	hMutex = CreateMutex(NULL, FALSE, L"_tmp_mutext_lock_");
	WaitForSingleObject(hMutex, INFINITE);
}


CLock::~CLock()
{
	ReleaseMutex(hMutex);
	DeleteObject(hMutex);
}


HANDLE CThread::Create(LPVOID lpParameter, DWORD dwInitialState, DWORD dwCreationFlag)
{
	dwState |= dwInitialState;
	this->lpParameter = lpParameter;

	if (dwState & STATE_ALIVE) return hThread;

	hThread = CreateThread(NULL, 0, StartAddress, this, dwCreationFlag, &dwThreadId);

	dwState |= STATE_ALIVE;
	if (dwState & STATE_CONTINUOUS) {
		hEvent = CreateEvent(NULL, TRUE, FALSE, L"_tmp_event_");
	}

	return hThread;
}


void CThread::Join(DWORD dwWaitInterval)
{
	if (dwState & STATE_BLOCKED) return;

	if (dwState & STATE_READY) return;

	dwState |= STATE_READY;
	WaitForSingleObject(hThread, dwWaitInterval);
	dwState ^= STATE_READY;
}


DWORD CThread::Suspend()
{
	if (dwState & STATE_BLOCKED) return DWORD(-1);

	if (dwState & STATE_READY) return DWORD(-1);

	DWORD dwSuspendCount = SuspendThread(hThread);
	dwState |= STATE_BLOCKED;

	return dwSuspendCount;
}


DWORD CThread::Resume()
{
	if (dwState & STATE_RUNNING) return DWORD(-1);

	DWORD dwSuspendCount = ResumeThread(hThread);
	dwState ^= STATE_BLOCKED;

	return dwSuspendCount;
}


void CThread::SetUserData(void* lpUserData)
{
	this->lpUserData = lpUserData;
}


void* CThread::GetUserData() const
{
	return lpUserData;
}


DWORD CThread::GetAsyncState() const
{
	if (dwState & STATE_ASYNC) {
		return STATE_ASYNC;
	}
	return STATE_SYNC;
}


DWORD CThread::GetState() const
{
	return dwState;
}


void CThread::SetState(DWORD dwNewState)
{
	dwState = 0;
	dwState |= dwNewState;
}


BOOL CThread::Alert()
{
	return SetEvent(hEvent);
}


DWORD __stdcall CThread::StartAddress(LPVOID lpParameter)
{
	CThread* thread = (CThread*)lpParameter;

	if (thread->GetAsyncState() == STATE_SYNC)
	{
		if (thread->dwState == STATE_CONTINUOUS)
		{
			DWORD dwWaitStatus = 0;
			while (TRUE)
			{
				thread->Run();
				dwWaitStatus = WaitForSingleObject(thread->hEvent, 10);
				if (dwWaitStatus == WAIT_OBJECT_0) break;
			}
			return 0;

		}
		thread->Run();
		return 0;
	}

	if (thread->dwState & STATE_CONTINUOUS)
	{
		DWORD dwWaitStatus = 0;
		while (TRUE)
		{
			CLock lock;
			{
				thread->Run();
			}
			dwWaitStatus = WaitForSingleObject(thread->hEvent, 10);

			if (dwWaitStatus == WAIT_OBJECT_0) break;
		}
		return 0;
	}

	CLock lock;
	{
		thread->Run();
	}
	return 0;
}

