#ifndef __GLOBAL_VAR_H
#define __GLOBAL_VAR_H

#include "wcxhead.h"
#include <Windows.h>
#include <string>

const int BUFSIZE = 1024;
const int OPTION_PAGES = 2;

#define PLUGIN_NAME "svnplugin.wcx"

extern tProcessDataProc g_ProcessDataProc;

struct TDlgDesc { 
	HWND hTabCtrl;
	HWND hCurDlg;  
	HWND hChildDlg[OPTION_PAGES];
	DLGTEMPLATE *ChildDlgRes[OPTION_PAGES];
};

struct Configuration {
	char svnCommand[BUFSIZE];

	bool deleteDir;

	bool createList;

	bool includeAddedFiles;

	bool includeModifiedFiles;

	bool includeDeletedFiles;
};

extern TDlgDesc g_DlgDesc;

extern HINSTANCE		g_hinst;		// handle to dll instance
extern HWND				g_MainWin;

extern Configuration g_config;

extern bool initialized;

extern std::string g_cfgFilePath;

#endif // __GLOBAL_VAR_H
