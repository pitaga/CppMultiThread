#include <Windows.h>
#include <iostream>
#include <tchar.h>


DWORD WINAPI StartAddress(LPVOID lpParameter)
{
	std::cout << "Hello, I am a very simple thread." << std::endl
		<< "I am used to demonstrate thread creation." << std::endl;
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	// �����̡߳�ע�⣬����ֻ������ͬ�Ĵ�����ж��� StartAddress ���ɡ�
	// ��Ϊ����������ʼ�̺߳�ִ�в���������
	HANDLE hThread = CreateThread(NULL, 0, StartAddress, NULL, 0, NULL);

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInformation = { 0 };

#ifdef _DEBUG
	WCHAR path[] = L"../Debug/tmpProcess.exe";
#else
	WCHAR path[] = L"../Release/tmpProcess.exe";
#endif

	BOOL bSuccess = CreateProcess(path, NULL, NULL, NULL, FALSE, 0,
		NULL, NULL, &startupInfo, &processInformation);
	
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	return system("pause");
}