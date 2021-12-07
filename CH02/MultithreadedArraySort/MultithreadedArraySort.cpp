#include <Windows.h>
#include <iostream>
#include <tchar.h>

#define THREAD_NUMBER		2
#define ELEMENT_NUMBER		200
#define BLOCK_SIZE			ELEMENT_NUMBER / THREAD_NUMBER
#define MAX_VALUE			1000

using namespace std;


typedef struct _tagARRAYOBJECT
{
	int* iArray;
	int iSize;
	int iThreadID;
} ARRAYOBJECT, *PARRAYOBJECT;


DWORD WINAPI ThreadStart(LPVOID lpParameter)
{
	PARRAYOBJECT pObject = (PARRAYOBJECT)lpParameter;
	int iTmp = 0;
	for (int iIndex = 0; iIndex < pObject->iSize; iIndex++)
	{
		for (int iEndIndex = pObject->iSize - 1; iEndIndex > iIndex; iEndIndex--)
		{
			if (pObject->iArray[iEndIndex] < pObject->iArray[iIndex])
			{
				iTmp = pObject->iArray[iEndIndex];
				pObject->iArray[iEndIndex] = pObject->iArray[iIndex];
				pObject->iArray[iIndex] = iTmp;
			}
		}
	}
	return 0;
}

void PrintArray(int* iArray, int iSize)
{
	for (int iIndex = 0; iIndex < iSize; ++iIndex) {
		std::cout << " " << iArray[iIndex];
	}
	std::cout << std::endl;
}

void MergeArray(int* leftArray, int leftArrayLenght, int* rightArray, int rightArrayLenght, int* mergedArray)
{
	int i = 0, j = 0, k = 0;
	while (i < leftArrayLenght && j < rightArrayLenght)
	{
		mergedArray[k++] = (leftArray[i] < rightArray[j])
			? leftArray[i++] : rightArray[j++];
	}

	if (i >= leftArrayLenght)
	{
		while (j < rightArrayLenght) {
			mergedArray[k++] = rightArray[j++];
		}
	}

	if (j >= rightArrayLenght)
	{
		while (i < leftArrayLenght) {
			mergedArray[k++] = leftArray[i++];
		}
	}
}



int _tmain(int argc, TCHAR* argv[])
{
	int iArray1[BLOCK_SIZE];
	int iArray2[BLOCK_SIZE];
	int iArray[ELEMENT_NUMBER];
	for (int iIndex = 0; iIndex < BLOCK_SIZE; ++iIndex) {
		iArray1[iIndex] = rand() % MAX_VALUE;
		iArray2[iIndex] = rand() % MAX_VALUE;
	}

	HANDLE hThreads[THREAD_NUMBER];
	ARRAYOBJECT pObject1 = { &(iArray1[0]), BLOCK_SIZE, 0 };
	hThreads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStart, (LPVOID)&pObject1, 0, NULL);

	ARRAYOBJECT pObject2 = { &(iArray2[0]), BLOCK_SIZE, 0 };
	hThreads[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStart, (LPVOID)&pObject2, 0, NULL);

	cout << "waiting execution..." << endl;
	WaitForMultipleObjects(THREAD_NUMBER, hThreads, TRUE, INFINITE);
	MergeArray(&iArray1[0], BLOCK_SIZE, &iArray2[0], BLOCK_SIZE, &iArray[0]);
	PrintArray(iArray, ELEMENT_NUMBER);
	
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);

	cout << "Array Sorted..." << endl;
	return 0;
}
