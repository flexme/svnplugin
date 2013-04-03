// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SVNPLUGIN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SVNPLUGIN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef __cplusplus
extern "C" {
#endif

#include "wcxhead.h"

#ifdef SVNPLUGIN_EXPORTS
#define SVNPLUGIN_API __declspec(dllexport)
#define STDCALL __stdcall
#else
#define SVNPLUGIN_API
#define STDCALL
#endif

// OpenArchive should perform all necessary operations 
// when an archive is to be opened
SVNPLUGIN_API HANDLE STDCALL
	OpenArchive (
		tOpenArchiveData *ArchiveData
	);

// WinCmd calls ReadHeaderEx to find out what files are in the archive
// It is called if the supported archive type may contain files >2 GB.
SVNPLUGIN_API int STDCALL
	ReadHeaderEx (
		HANDLE hArcData, 
		tHeaderDataEx *HeaderDataEx
	);

// WinCmd calls ReadHeader to find out what files are in the archive
SVNPLUGIN_API int STDCALL
	ReadHeader (
		HANDLE hArcData, 
		tHeaderData *HeaderData
	);

// ProcessFile should unpack the specified file 
// or test the integrity of the archive
SVNPLUGIN_API int STDCALL
	ProcessFile (
		HANDLE hArcData, 
		int Operation, 
		char *DestPath, 
		char *DestName
	);

// CloseArchive should perform all necessary operations 
// when an archive is about to be closed.
SVNPLUGIN_API int STDCALL
	CloseArchive (
		HANDLE hArcData	
	);

// This function allows you to notify user 
// about changing a volume when packing files
SVNPLUGIN_API void STDCALL
	SetChangeVolProc (
		HANDLE hArcData, 
		tChangeVolProc pChangeVolProc1
	);

// This function allows you to notify user about 
// the progress when you unpack files
SVNPLUGIN_API void STDCALL
	SetProcessDataProc (
		HANDLE hArcData, 
		tProcessDataProc pProcessDataProc
	);

// GetPackerCaps tells WinCmd what features your packer plugin supports
SVNPLUGIN_API int STDCALL
	GetPackerCaps ();

// PackFiles specifies what should happen when a user creates, 
// or adds files to the archive.
SVNPLUGIN_API int STDCALL 
	PackFiles (
		char *PackedFile, 
		char *SubPath, 
		char *SrcPath, 
		char *AddList, 
		int Flags
	);

// ConfigurePacker gets called when the user clicks the Configure button 
// from within "Pack files..." dialog box in WinCmd
SVNPLUGIN_API void STDCALL
	ConfigurePacker (
		HWND Parent, 
		HINSTANCE DllInstance
	);

#ifdef __cplusplus
}
#endif
