#include "stdafx.h"
#include "uifunc.h"
#include "resource.h"
#include "globalvar.h"
#include <commctrl.h>
#include <CommDlg.h>

void CheckButton( HWND hDlg, UINT ItemCode )
{
	HWND hItem =  GetDlgItem( 
		hDlg, 
		ItemCode
		);

	SendMessage(  
		hItem, 
		BM_SETCHECK,
		BST_CHECKED, 
		0
		);
}

void UncheckButton( HWND hDlg, UINT ItemCode )
{
	HWND hItem =  GetDlgItem( 
		hDlg, 
		ItemCode
		);

	SendMessage(  
		hItem, 
		BM_SETCHECK,
		BST_UNCHECKED, 
		0
		);
}

void EnableControl( HWND hDlg, UINT ItemCode )
{
	HWND hItem =  GetDlgItem( 
		hDlg, 
		ItemCode
		);

	EnableWindow( hItem, TRUE );
}

void DisableControl( HWND hDlg, UINT ItemCode )
{
	HWND hItem =  GetDlgItem( 
		hDlg, 
		ItemCode
		);

	EnableWindow( hItem, FALSE );
}

DLGTEMPLATE * DoLockDlgRes( LPCTSTR lpszResName ) 
{ 
	HRSRC hrsrc		= FindResource( g_hinst, lpszResName, RT_DIALOG ); 
	HGLOBAL hglb	= LoadResource( g_hinst, hrsrc ); 

	return (DLGTEMPLATE *) LockResource( hglb ); 
}

BOOL CALLBACK ChildDialogProc(
	HWND hChildDlg, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
	)
{
	switch ( message ) {
		case WM_INITDIALOG:
			RECT rcTab, rc;

			GetWindowRect( g_DlgDesc.hTabCtrl, &rcTab );
			TabCtrl_AdjustRect( g_DlgDesc.hTabCtrl, FALSE, &rcTab );
			CopyRect( &rc, &rcTab );
			OffsetRect( &rc, -rc.left, -rc.top );

			SetWindowPos(
				hChildDlg, 
				HWND_TOP, 
				rcTab.left, 
				rcTab.top, 
				rc.right, 
				rc.bottom, 
				0
				); 
			return TRUE;

		case WM_COMMAND:
			HWND	hItem;
			LRESULT	rv;

			switch (wParam) {
			case IDC_CREATEFILELIST:
				hItem = GetDlgItem( hChildDlg, IDC_CREATEFILELIST);
				rv = SendMessage( hItem, BM_GETCHECK, 0, 0 );

				if (rv == BST_CHECKED) {
					EnableControl(g_DlgDesc.hChildDlg[0], IDC_ADDEDFILES);
					EnableControl(g_DlgDesc.hChildDlg[0], IDC_MODIFIEDFILES);
					EnableControl(g_DlgDesc.hChildDlg[0], IDC_REMOVEDFILES);
				} else {
					DisableControl(g_DlgDesc.hChildDlg[0], IDC_ADDEDFILES);
					DisableControl(g_DlgDesc.hChildDlg[0], IDC_MODIFIEDFILES);
					DisableControl(g_DlgDesc.hChildDlg[0], IDC_REMOVEDFILES);
				}
				break;

			case IDC_BROWSE:
				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL;
				char szFile[BUFSIZE];
				szFile[0] = 0;
				ofn.hwndOwner = hChildDlg;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = BUFSIZE;
				ofn.lpstrTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.lpstrFilter = "Exe(*.exe)\0*.exe;\0\0";
				ofn.nFilterIndex = 1;
				hItem = GetDlgItem( hChildDlg, IDC_SVNPATHEDIT);
				if (GetOpenFileName(&ofn)) {
					SendMessage(
						hItem,
						WM_SETTEXT,
						strlen(szFile),
						(LPARAM) szFile);
				}
				break;
			}
			return TRUE;
	}
	return FALSE;
}

void OnInitOptionDialog( HWND hDlg )
{
	TCITEM	Attributes; 

	// Create Tab Control
	g_DlgDesc.hTabCtrl = GetDlgItem( hDlg, IDC_TAB1 );

	Attributes.mask = TCIF_TEXT | TCIF_IMAGE; 
	Attributes.iImage = -1; 
	Attributes.pszText = "Basic";
	TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 0, &Attributes ); 
	Attributes.pszText = "About"; 
	TabCtrl_InsertItem( g_DlgDesc.hTabCtrl, 1, &Attributes ); 

	g_DlgDesc.ChildDlgRes[0] = 
		DoLockDlgRes( MAKEINTRESOURCE( IDD_BASICDLG ) ); 
	g_DlgDesc.hChildDlg[0] = CreateDialogIndirect(g_hinst, g_DlgDesc.ChildDlgRes[0], hDlg, ChildDialogProc);

	g_DlgDesc.ChildDlgRes[1] = 
		DoLockDlgRes( MAKEINTRESOURCE( IDD_ABOUTDLG ) ); 
	g_DlgDesc.hChildDlg[1] = CreateDialogIndirect(g_hinst, g_DlgDesc.ChildDlgRes[1], hDlg, ChildDialogProc);

	// setup default parameters
	if (g_config.deleteDir) {
		CheckButton(g_DlgDesc.hChildDlg[0], IDC_DELETEDIRECTORY);
	}
	if (g_config.createList) {
		CheckButton(g_DlgDesc.hChildDlg[0], IDC_CREATEFILELIST);
	} else {
		DisableControl(g_DlgDesc.hChildDlg[0], IDC_ADDEDFILES);
		DisableControl(g_DlgDesc.hChildDlg[0], IDC_MODIFIEDFILES);
		DisableControl(g_DlgDesc.hChildDlg[0], IDC_REMOVEDFILES);
	}
	if (g_config.includeAddedFiles) {
		CheckButton(g_DlgDesc.hChildDlg[0], IDC_ADDEDFILES);
	}
	if (g_config.includeModifiedFiles) {
		CheckButton(g_DlgDesc.hChildDlg[0], IDC_MODIFIEDFILES);
	}
	if (g_config.includeDeletedFiles) {
		CheckButton(g_DlgDesc.hChildDlg[0], IDC_REMOVEDFILES);
	}

	HWND hItem = GetDlgItem(g_DlgDesc.hChildDlg[0], IDC_SVNPATHEDIT);
	SendMessage(
		hItem,
		WM_SETTEXT,
		strlen(g_config.svnCommand),
		(LPARAM) g_config.svnCommand);
}

static void GetOptionsFromWindow() {
	HWND hItem = GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_DELETEDIRECTORY
		);
	LRESULT rv = SendMessage(hItem, BM_GETCHECK, 0, 0);
	g_config.deleteDir = rv == BST_CHECKED ? true : false;

	hItem = GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_CREATEFILELIST
		);
	rv = SendMessage(hItem, BM_GETCHECK, 0, 0);
	g_config.createList = rv == BST_CHECKED ? true : false;

	hItem = GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_ADDEDFILES
		);
	rv = SendMessage(hItem, BM_GETCHECK, 0, 0);
	g_config.includeAddedFiles = rv == BST_CHECKED ? true : false;

	hItem = GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_MODIFIEDFILES
		);
	rv = SendMessage(hItem, BM_GETCHECK, 0, 0);
	g_config.includeModifiedFiles = rv == BST_CHECKED ? true : false;

	hItem = GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_REMOVEDFILES
		);
	rv = SendMessage(hItem, BM_GETCHECK, 0, 0);
	g_config.includeDeletedFiles = rv == BST_CHECKED ? true : false;

	hItem =  GetDlgItem( 
		g_DlgDesc.hChildDlg[0], 
		IDC_SVNPATHEDIT
		);
	SendMessage(  
		hItem, 
		WM_GETTEXT,
		(WPARAM)sizeof(g_config.svnCommand), 
		(LPARAM) g_config.svnCommand
		);
}

BOOL CALLBACK OptionsDialogProc(
	HWND hDlg, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
	)
{
	switch (message) {
	case WM_INITDIALOG:

		g_MainWin = hDlg;
		OnInitOptionDialog( hDlg );
		OnSelChanged( hDlg );
		return TRUE;

	case WM_MOVE:
		for ( int i = 0; i < OPTION_PAGES; i++ )
		{
			SendMessage( g_DlgDesc.hChildDlg[i], WM_INITDIALOG, 0, 0 );
		}

		SendMessage( g_MainWin, WM_SETFOCUS, 0, 0 );
		break;

	case WM_NOTIFY:
		NMHDR *p;
		p = (NMHDR*) lParam;

		switch ( p->code ) 
		{ 
		case TCN_SELCHANGE:

			OnSelChanged( hDlg );
			SendMessage( g_MainWin, WM_SETFOCUS, 0, 0 );
			return TRUE;
		} 
		break; 

	case WM_COMMAND:
		switch ( wParam ) {
		case IDOK:
			{
				GetOptionsFromWindow();
				FILE *fp = fopen(g_cfgFilePath.c_str(), "w");
				if (fp) {
					fwrite(&g_config, sizeof(g_config), 1, fp);
					if (ferror(fp)) {
						fclose(fp);
						return FALSE;
					}
					fclose(fp);
				}
			}

		case IDCANCEL:
			EndDialog( hDlg, wParam );
			return TRUE;
		}
	}

	return FALSE;
}

VOID OnSelChanged( HWND hDlg ) 
{ 
	UINT CurPageIndex = TabCtrl_GetCurSel( g_DlgDesc.hTabCtrl ); 

	if ( g_DlgDesc.hCurDlg != NULL ) 
		ShowWindow( g_DlgDesc.hCurDlg, SW_HIDE ); 

	// Create the new child dialog box. 
	g_DlgDesc.hCurDlg = g_DlgDesc.hChildDlg[CurPageIndex];
	ShowWindow( g_DlgDesc.hCurDlg, SW_SHOW );

} 
