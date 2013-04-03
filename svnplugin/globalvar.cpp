#include "stdafx.h"
#include "globalvar.h"

tProcessDataProc g_ProcessDataProc;

TDlgDesc g_DlgDesc;
HINSTANCE		g_hinst;
HWND				g_MainWin;
Configuration g_config;
bool initialized = false;
std::string g_cfgFilePath;
