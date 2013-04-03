#ifndef __FILE_UTIL_H
#define __FILE_UTIL_H

#include "globalvar.h"
#include <string>
#include <vector>

void formatPath(std::string& path);

void deepCopyDirectory(std::string destDir);

void copyFile(std::string srcFile, std::string destFile);

std::string getFirstDir(const std::string& dir);

std::string getFullPath(const std::string& parentDir, const std::string& name);

std::vector<std::string> splitString(const std::string& str);

bool execute(const char* command, std::string currentDir, DWORD& retCode, std::string& output);

#endif // __FILE_UTIL_H