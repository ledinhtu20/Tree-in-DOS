#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <list>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#pragma comment(lib, "User32.lib")
using namespace std;

string result;
TCHAR inputPath[MAX_PATH];
bool currentLineBranch[100];
bool fileListingEnabled = false;
int getList(TCHAR* inputPath);

//==============================================
extern "C" __declspec(dllexport) void sendPath(const char* stringPtr) {
	printf("(C) '%s'\n", stringPtr);
	StringCchCopy(inputPath, MAX_PATH, stringPtr);
}

extern "C" __declspec(dllexport) void setFileListingEnabled(const bool val) {
	fileListingEnabled = val;
}

extern "C" __declspec(dllexport) void getTree(char** stringPtr) {
	getList(inputPath);
	*stringPtr = (char*) malloc(sizeof(char) * result.size());
	memset(*stringPtr, 0, sizeof(char) * result.size());
	strcpy(*stringPtr, result.c_str());
}

extern "C" __declspec(dllexport) void clean(char* stringPtr) {
	free(stringPtr);
	result = "";
	result.clear();
}
//==============================================


/* Ham ve nhanh */
void appendBranch(int tabOrder, bool isFolder) {
	int end;
	if (isFolder) {
		end = tabOrder - 1;
	} else {
		end = tabOrder;
	}
	for (int i = 1; i <= end; i++) {
		if (currentLineBranch[i]) {
			result += "|    ";
		} else {
			result += "     ";
		}
	}
}
void getTree(WIN32_FIND_DATA ffd, TCHAR dir[], TCHAR szDir[], HANDLE hFind, int tabOrder) {
	// Mang chua danh sach cac file cua thu muc hien hanh
	list<string> listFile, listFolder;
	bool parentFolder = false, curentFolder = false;
	int next_tabOrder = tabOrder + 1;
		
	//Buoc 1: Tap hop danh sach thu muc va tap tin cua thu muc hien hanh		
	WIN32_FIND_DATA check_ffd = ffd;
	HANDLE check_hFind = hFind;
	check_hFind = FindFirstFile(szDir, &check_ffd);
	do {
		// Loai bo thu muc cha va thu muc hien hanh (. va ..)
		parentFolder = strcmp(ffd.cFileName,"..") == 0;
		curentFolder = strcmp(ffd.cFileName,".") == 0;
		if (!parentFolder && !curentFolder) {
			// Neu la thu muc
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				//folderName.push_back(ffd.cFileName);
				listFolder.push_back(ffd.cFileName);
			} else { // la file
				listFile.push_back(ffd.cFileName);
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);// Neu con file thi tiep tuc
	
	// Buoc 2: In ra file o thu muc hien hanh
	list<string>::const_iterator iterator, end;
	if (fileListingEnabled) { // Neu lenh Tree co tham so /F
		for (iterator = listFile.begin(), end = listFile.end(); iterator != end; ++iterator) {
			currentLineBranch[tabOrder] = !listFolder.empty(); // ko co thuc muc nao thi ko can in nhanh xuong
			appendBranch(tabOrder, false);
			result += iterator->c_str();
			result += "\n";
			
			if (distance(iterator,end) == 1) { //file cuoi cung in 1 dong gian cach
				appendBranch(tabOrder, false);
				result += "\n";
			}
		}
	}

	// Buoc 3: Cu moi thu muc trong danh sach, truy cap danh sach tung thu muc con.
	for (iterator = listFolder.begin(), end = listFolder.end(); iterator != end; ++iterator) {
		currentLineBranch[tabOrder] = true;
		if (distance(iterator,end) == 1) { // thu muc cuoi cung trong thu muc con
			currentLineBranch[tabOrder] = false;
		}
		appendBranch(tabOrder, true);
		
		result += "+----+"; //+--> 
		result += iterator->c_str();
		result += "\n";
		
		TCHAR next_szDir[MAX_PATH], next_dir[MAX_PATH];
		HANDLE next_hFind;
		StringCchCopy(next_szDir, MAX_PATH, dir); // Chep dia chi thu muc hien hanh
		StringCchCat(next_szDir, MAX_PATH, TEXT("\\")); // Gan them ki tu '\'
		StringCchCat(next_szDir, MAX_PATH, iterator->c_str()); // Gan them ten thu muc
		StringCchCopy(next_dir, MAX_PATH, next_szDir); // Chep dia chi thu muc con chuan bi duyet
		StringCchCat(next_szDir, MAX_PATH, TEXT("\\*")); // Them ki tu all files '\*'
		next_hFind = FindFirstFile(next_szDir, &ffd);
		 
		getTree(ffd, next_dir, next_szDir, next_hFind, next_tabOrder); // DE QUY VAO THU MUC CON
		
		if (distance(iterator,end) == 1) { //folder cuoi cung in 1 dong gian cach
			appendBranch(tabOrder, false);
			result += "\n";
		}
	}
}

int getList(TCHAR* inputPath) {
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH], dir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	
	StringCchLength(inputPath, MAX_PATH, &length_of_arg);
	if (length_of_arg > (MAX_PATH - 3)) {
		result += "PATH TOO LONG";
		return (-1);
	}

	StringCchCopy(szDir, MAX_PATH, inputPath);
	StringCchCopy(dir, MAX_PATH, szDir);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));


	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		result = "\nKHONG TIM THAY"; //+=FindFirstFile
		return dwError;
	}

	result += "\n";
	result += dir;
	result += "\n";
	getTree(ffd, dir, szDir, hFind, 1);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		result = "\nKHONG TIM THAY"; //+=FindFirstFile
	}

	FindClose(hFind);
	return dwError;
}



