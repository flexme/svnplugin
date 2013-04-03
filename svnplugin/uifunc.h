#ifndef __UI_FUNC_H
#define __UI_FUNC_H

#include <Windows.h>

DLGTEMPLATE * DoLockDlgRes( 
	LPCTSTR lpszResName 
	);

BOOL CALLBACK ChildDialogProc(
	HWND hChildDlg, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
	);

void OnInitOptionDialog( 
	HWND hDlg 
	);

BOOL CALLBACK OptionsDialogProc(
	HWND hDlg, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
	);

VOID OnSelChanged( 
	HWND hDlg 
	);

void EnableControl( 
	HWND hDlg, 
	UINT ItemCode 
	);

void DisableControl( 
	HWND hDlg, 
	UINT ItemCode 
	);

void CheckButton( 
	HWND hDlg, 
	UINT ItemCode 
	);

void UncheckButton( 
	HWND hDlg, 
	UINT ItemCode 
	);

#endif
