// svnplugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "svnplugin.h"
#include "fileutil.h"
#include "uifunc.h"
#include "resource.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <set>

static BOOL setupConfig() {
	if (initialized) return TRUE;

	if (g_hinst == NULL) {
		g_hinst = GetModuleHandle(PLUGIN_NAME);
		if ( g_hinst == NULL ) return FALSE;
	}

	char buf[BUFSIZE];
	DWORD rv = GetModuleFileName( g_hinst, buf, BUFSIZE );
	if (!rv) return FALSE;

	char *pos = strrchr(buf, '.');
	if (pos == NULL) return FALSE;

	pos++;
	*pos++ = 'i';
	*pos++ = 'n';
	*pos++ = 'i';
	*pos = 0;
	g_cfgFilePath = std::string(buf);

	strcpy(g_config.svnCommand, "svn");
	g_config.deleteDir = false;
	g_config.createList = true;
	g_config.includeAddedFiles = true;
	g_config.includeDeletedFiles = true;
	g_config.includeModifiedFiles = true;
	initialized = true;

	FILE *fp = fopen(buf, "r");
	if (fp) {
		fread(&g_config, sizeof(g_config), 1, fp);
		if (ferror(fp)) {
			fclose(fp);
			return FALSE;
		}
		fclose(fp);
	}
	return TRUE;
}

// OpenArchive should perform all necessary operations 
// when an archive is to be opened
SVNPLUGIN_API HANDLE STDCALL
	OpenArchive (
		tOpenArchiveData *ArchiveData
	)
{
	MessageBox(	NULL, "Impossible to open this type of archive", "Error", MB_OK );
	return 0;
}

// WinCmd calls ReadHeaderEx to find out what files are in the archive
// It is called if the supported archive type may contain files >2 GB.
SVNPLUGIN_API int STDCALL
	ReadHeaderEx (
		HANDLE hArcData, 
		tHeaderDataEx *HeaderDataEx
	)
{
	return 0;
}

// WinCmd calls ReadHeader to find out what files are in the archive
SVNPLUGIN_API int STDCALL
	ReadHeader (
		HANDLE hArcData, 
		tHeaderData *HeaderData
	)
{
	return 0;
}

// ProcessFile should unpack the specified file 
// or test the integrity of the archive
SVNPLUGIN_API int STDCALL
	ProcessFile (
		HANDLE hArcData, 
		int Operation, 
		char *DestPath, 
		char *DestName
	)
{
	return 0;
}

// CloseArchive should perform all necessary operations 
// when an archive is about to be closed.
SVNPLUGIN_API int STDCALL
	CloseArchive (
		HANDLE hArcData	
	)
{
	HRESULT rv;
	UCHAR	Ret = E_ECLOSE;

	rv = CloseHandle( hArcData );

	if ( rv == S_OK )
		Ret = 0;

	return ( Ret );
}

// This function allows you to notify user 
// about changing a volume when packing files
SVNPLUGIN_API void STDCALL
	SetChangeVolProc (
		HANDLE hArcData, 
		tChangeVolProc pChangeVolProc1
	)
{

}

// This function allows you to notify user about 
// the progress when you unpack files
SVNPLUGIN_API void STDCALL
	SetProcessDataProc (
		HANDLE hArcData, 
		tProcessDataProc pProcessDataProc
	)
{
	g_ProcessDataProc = pProcessDataProc;
}

// GetPackerCaps tells WinCmd what features your packer plugin supports
SVNPLUGIN_API int STDCALL
	GetPackerCaps ()
{
	return PK_CAPS_NEW | PK_CAPS_OPTIONS;
}

// PackFiles specifies what should happen when a user creates, 
// or adds files to the archive.
SVNPLUGIN_API int STDCALL 
	PackFiles (
		char *PackedFile, 
		char *SubPath, 
		char *SrcPath, 
		char *AddList, 
		int Flags
	)
{
	if (!setupConfig()) {
		MessageBox(NULL, "Error getting configuration", "Error", MB_OK);
		return 0;
	}

	std::string destFile(PackedFile);
	for (std::size_t i = 0; i < destFile.size(); i++) {
		if (destFile[i] == '/') destFile[i] = '\\';
	}
	std::string destDir = destFile.substr(0, destFile.find_last_of('\\'));

	int index = 0;
	std::set<std::string> dirs;
	while (AddList[index]) {
		std::string name(&AddList[index]);
		if (name[name.size() - 1] == '/' || name[name.size() - 1] == '\\') {
			// directory
			dirs.insert(getFirstDir(name));
		}

		while (AddList[index]) index++;
		index++;
	}

	std::vector<std::string> addedFiles;
	std::vector<std::string> updatedFiles;
	std::vector<std::string> deletedFiles;

	char buf[BUFSIZE];

	std::string svnCmd(g_config.svnCommand);

	for (std::set<std::string>::iterator it = dirs.begin(); it != dirs.end(); it++) {
		std::string output;
		DWORD retCode;
		std::string fullPath = getFullPath(SrcPath, *it);

		if (!execute((svnCmd + " info").c_str(), fullPath.c_str(), retCode, output)) {
			MessageBox( NULL, "Can't execute svn command", "Error", MB_OK);
			return 0;
		} else if (retCode != 0) {
			std::ostringstream os;
			os << fullPath << "is not a SVN directory";
			MessageBox(NULL, os.str().c_str(), "Error", MB_OK);
			continue;
		}

		if (!execute((svnCmd + " st").c_str(), fullPath.c_str(), retCode, output)) {
			MessageBox( NULL, "Can't execute svn command", "Error", MB_OK);
			return 0;
		} else {
			index = 0;
			int curindex = 0;
			std::vector<std::string> contents;
			for (std::size_t i = 0; i < output.size(); i++) {
				if (output[i] == '\n' || output[i] == '\r') {
					if (curindex > 0) {
						buf[curindex] = 0;
						contents.push_back(buf);
					}
					curindex = 0;
				} else {
					buf[curindex++] = output[i];
				}
			}
			if (curindex > 0) {
				buf[curindex] = 0;
				contents.push_back(buf);
			}

			for (std::size_t i = 0; i < contents.size(); i++) {
				std::vector<std::string> parts = splitString(contents[i]);
				if (parts.size() == 2) {
					if (parts[0] == "A" || parts[0] == "?") {
						std::string fn = getFullPath(*it, parts[1]);
						DWORD attr = GetFileAttributes(getFullPath(SrcPath, fn).c_str());
						addedFiles.push_back(fn);
					} else if (parts[0] == "D" || parts[0] == "!") {
						deletedFiles.push_back(getFullPath(*it, parts[1]));
					} else if (parts[0] == "M") {
						std::string fn = getFullPath(*it, parts[1]);
						DWORD attr = GetFileAttributes(getFullPath(SrcPath, fn).c_str());
						if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
							updatedFiles.push_back(fn);
						}
					}
				}
			}
		}
	}
	
	for (std::size_t i = 0; i < addedFiles.size(); i++) {
		std::string src = getFullPath(SrcPath, addedFiles[i]);

		DWORD attr = GetFileAttributes(src.c_str());
		if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
			WIN32_FILE_ATTRIBUTE_DATA fileInfo;
			GetFileAttributesEx(src.c_str(), GetFileExInfoStandard, (void*) &fileInfo);
			g_ProcessDataProc((char*) src.c_str(), fileInfo.nFileSizeLow);
		}
		copyFile(src, getFullPath(destDir, addedFiles[i]));
	}
	for (std::size_t i = 0; i < updatedFiles.size(); i++) {
		std::string src = getFullPath(SrcPath, updatedFiles[i]);
		copyFile(src, getFullPath(destDir, updatedFiles[i]));
	}

	if (g_config.createList) {
		std::string destFile = getFullPath(destDir, "list.txt");
		FILE* fp = fopen(destFile.c_str(), "w");
		if (!fp) return 0;
		if (g_config.includeAddedFiles && addedFiles.size() > 0) {
			fprintf(fp, "Added Files:\n");
			for (std::vector<std::string>::iterator it = addedFiles.begin(); it != addedFiles.end(); it++) {
				fprintf(fp, "    %s\n", it->c_str());
			}
			fprintf(fp, "\n");
		}
		if (g_config.includeModifiedFiles && updatedFiles.size() > 0) {
			fprintf(fp, "Modified Files:\n");
			for (std::vector<std::string>::iterator it = updatedFiles.begin(); it != updatedFiles.end(); it++) {
				fprintf(fp, "    %s\n", it->c_str());
			}
			fprintf(fp, "\n");
		}
		if (g_config.includeDeletedFiles && deletedFiles.size() > 0) {
			fprintf(fp, "Deleted Files:\n");
			for (std::vector<std::string>::iterator it = deletedFiles.begin(); it != deletedFiles.end(); it++) {
				fprintf(fp, "    %s\n", it->c_str());
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
	return 0;
}

// ConfigurePacker gets called when the user clicks the Configure button 
// from within "Pack files..." dialog box in WinCmd
SVNPLUGIN_API void STDCALL
	ConfigurePacker (
		HWND Parent, 
		HINSTANCE DllInstance
	)
{
	g_hinst = DllInstance;

	if (!setupConfig()) {
		MessageBox(NULL, "Error getting configuration", "Error", MB_OK);
		return;
	}

	DialogBox(DllInstance, MAKEINTRESOURCE(IDD_OPTIONSDLG), Parent, (DLGPROC) OptionsDialogProc);
}
