#include "stdafx.h"
#include "fileutil.h"
#include <Windows.h>
#include <ShellAPI.h>

void formatPath(std::string& path) {
	for (std::size_t i = 0; i < path.size(); i++) {
		if (path[i] == '/') path[i] = '\\';
	}
}

void deepCopyDirectory(std::string destDir) {
	if (destDir[destDir.size() - 1] == '\\') {
		destDir = destDir.substr(0, destDir.size() - 1);
	}
	int index = 0;
	while (true) {
		index = destDir.find_first_of('\\', index);
		if (index == std::string::npos) {
			break;
		}
		std::string ps = destDir.substr(0, index);
		DWORD attr = GetFileAttributes(ps.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES) {
			// directory not exists
			CreateDirectory(ps.c_str(), NULL);
		}
		index++;
	}
	if (destDir.back() != '\\') {
		DWORD attr = GetFileAttributes(destDir.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES) {
			CreateDirectory(destDir.c_str(), NULL);
		}
	}
}

void copyFile(std::string srcFile, std::string destFile) {
	formatPath(srcFile);
	formatPath(destFile);

	DWORD attr = GetFileAttributes(srcFile.c_str());
	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		std::size_t pos = destFile.find_last_of('\\');
		destFile = destFile.substr(0, pos);
		deepCopyDirectory(destFile);
		SHFILEOPSTRUCT s = {0};
		s.hwnd = g_MainWin;
		s.wFunc = FO_COPY;
		s.pTo = destFile.c_str();
		s.pFrom = srcFile.c_str();
		s.fFlags = FOF_SILENT;
		SHFileOperation(&s);
	} else {
		std::size_t pos = destFile.find_last_of('\\');
		deepCopyDirectory(destFile.substr(0, pos));
		CopyFile(srcFile.c_str(), destFile.c_str(), false);
	}
}

std::string getFirstDir(const std::string& dir) {
	int index1 = dir.find_first_of('/');
	int index2 = dir.find_first_of('\\');
	if (index1 == std::string::npos) index1 = 1 << 30;
	if (index2 == std::string::npos) index2 = 1 << 30;
	int index = index1 < index2 ? index1 : index2;
	if (index == 1 << 30) {
		return dir;
	} else {
		return dir.substr(0, index);
	}
}

std::string getFullPath(const std::string& parentDir, const std::string& name) {
	std::string full = parentDir;
	if (parentDir.back() != '/' && parentDir.back() != '\\') {
		return parentDir + "\\" + name;
	} else {
		return parentDir + name;
	}
}

std::vector<std::string> splitString(const std::string& str) {
	std::vector<std::string> ret;
	char buf[BUFSIZE];
	std::size_t index = 0;
	std::size_t curIndex = 0;
	while (index < str.size()) {
		while (str[index] == ' ' || str[index] == '\t') index++;
		while (index < str.size() && str[index] != ' ' && str[index] != '\t') {
			buf[curIndex++] = str[index++];
		}
		buf[curIndex] = 0;
		ret.push_back(buf);
		curIndex = 0;
	}
	if (curIndex > 0) {
		ret.push_back(buf);
	}
	return ret;
}

bool execute(const char* command, std::string currentDir, DWORD& retCode, std::string& output) {
	HANDLE outWR;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	TCHAR lpTempPathBuffer[BUFSIZE];
	TCHAR szTempFileName[MAX_PATH];
	DWORD ret = GetTempPath(BUFSIZE, lpTempPathBuffer);
	if (ret > BUFSIZE || ret == 0) {
		MessageBox(	NULL, "Error GetTempPath", "Error", MB_OK );
		return false;
	}

	ret = GetTempFileName(lpTempPathBuffer, "svn", 0, szTempFileName);
	if (ret == 0) {
		MessageBox(	NULL, "Error GetTempFileName", "Error", MB_OK );
		return false;
	}

	outWR = CreateFile(szTempFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (outWR == INVALID_HANDLE_VALUE) {
		MessageBox(	NULL, "Error create file", "Error", MB_OK );
		return false;
	}

	PROCESS_INFORMATION processInfomation;
	STARTUPINFO startupInfo;
	memset(&processInfomation, 0, sizeof(processInfomation));
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.hStdError = outWR;
	startupInfo.hStdOutput = outWR;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	startupInfo.wShowWindow = SW_HIDE;

	BOOL result = CreateProcess(
		NULL,
		(char*) command,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		currentDir.c_str(),
		&startupInfo,
		&processInfomation);

	if (!result) {
		MessageBox(	NULL, "Can't CreateProcess", "Error", MB_OK );
		CloseHandle(outWR);
		return false;
	} else {
		WaitForSingleObject(processInfomation.hProcess, INFINITE);
		GetExitCodeProcess(processInfomation.hProcess, &retCode);
		CloseHandle(processInfomation.hProcess);
		CloseHandle(processInfomation.hThread);

		// read from stdout
		if (!CloseHandle(outWR)) {
			MessageBox(	NULL, "Can't CloseHandle", "Error", MB_OK );
			return false;
		}

		outWR = CreateFile(szTempFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (outWR == INVALID_HANDLE_VALUE) {
			MessageBox(	NULL, "Error open file", "Error", MB_OK );
			return false;
		}

		CHAR buf[BUFSIZE];
		DWORD dwRead;
		DWORD totalSize = 0;
		std::vector<char*> allBufs;
		std::vector<DWORD> lens;
		while (true) {
			result = ReadFile(outWR, buf, BUFSIZE, &dwRead, NULL);
			if (!result || dwRead == 0) break;
			totalSize += dwRead;
			char* content = new char[dwRead];
			memcpy(content, buf, dwRead);
			allBufs.push_back(content);
			lens.push_back(dwRead);
		}
		char* content = new char[totalSize + 1];
		char* currentPtr = content;
		for (std::size_t i = 0; i < allBufs.size(); i++) {
			memcpy(currentPtr, allBufs[i], lens[i]);
			currentPtr += lens[i];
		}
		content[totalSize] = 0;
		for (std::size_t i = 0; i < allBufs.size(); i++) {
			delete[] allBufs[i];
		}
		output.swap(std::string(content));
		delete[] content;

		CloseHandle(outWR);
		return true;
	}
}


