/*
** Astrolog (Version 8.00) File: wqt.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated elsewhere in this program are Copyright (C) 1991-2026 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** This file declares the small subset of the Win32 API used by Astrolog's
** Windows menu and dialog code (wdriver.cpp, wdialog.cpp), so that code can
** be compiled unchanged in the Qt version (#define QT). The functions are
** implemented on top of Qt 6 Widgets in wqt.cpp. This header is plain C++
** and doesn't include any Qt headers, so the core program doesn't need them.
*/

#ifndef WQT_H
#define WQT_H

#include <stdint.h>
#include <string.h>
#include <wchar.h>

/*
******************************************************************************
** Basic Types.
******************************************************************************
*/

#define FAR
#define NEAR
#define PASCAL
#define CALLBACK
#define WINAPI

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef uint32_t DWORD;
typedef unsigned int UINT;
typedef long LONG;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef unsigned long COLORREF;  // Same as Astrolog's KV type.
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef wchar_t WCHAR;
typedef const wchar_t *LPCWSTR;
typedef void *LPVOID;
typedef long HRESULT;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// Handles are opaque pointers to structures private to wqt.cpp.
typedef struct QtWnd *HWND;
typedef struct QtMenu *HMENU;
typedef struct QtDC *HDC;
typedef struct QtObj *HGDIOBJ;
typedef HGDIOBJ HPEN;
typedef HGDIOBJ HBRUSH;
typedef HGDIOBJ HFONT;
typedef HGDIOBJ HBITMAP;
typedef HGDIOBJ HRGN;
typedef void *HINSTANCE;
typedef void *HACCEL;
typedef void *HANDLE;
typedef void *HGLOBAL;
typedef void *HCURSOR;
typedef void *HICON;

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *LPRECT;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *LPPOINT;

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE;

typedef struct tagSYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME;

typedef int (*DLGPROC)(HWND, UINT, WORD, LONG);
typedef int (*FARPROC)();
typedef void (*TIMERPROC)(HWND, UINT, uintptr_t, DWORD);

#define MAKEINTRESOURCE(i) ((LPCSTR)(intptr_t)(i))
#define MakeProcInstance(pfn, hinst) ((DLGPROC)(pfn))
#define FreeProcInstance(pfn)
#define LOWORD(l) ((WORD)((uintptr_t)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((uintptr_t)(l) >> 16) & 0xffff))
#define MAKEWPARAM(l, h) ((WPARAM)(((DWORD)(WORD)(l)) | ((DWORD)(WORD)(h)) << 16))
#define MAKELPARAM(l, h) ((LPARAM)MAKEWPARAM(l, h))
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | \
  (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(kv) ((BYTE)(kv))
#define GetGValue(kv) ((BYTE)((kv) >> 8))
#define GetBValue(kv) ((BYTE)((kv) >> 16))
#define FAILED(hr) ((HRESULT)(hr) < 0)
#if UINTPTR_MAX > 0xffffffffu
#define szArchCore "64 bit"
#else
#define szArchCore "32 bit"
#endif
#define _unlink unlink
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif


/*
******************************************************************************
** Constants.
******************************************************************************
*/

// Window messages.
#define WM_CREATE        0x0001
#define WM_SIZE          0x0005
#define WM_PAINT         0x000F
#define WM_CLOSE         0x0010
#define WM_ERASEBKGND    0x0014
#define WM_GETMINMAXINFO 0x0024
#define WM_KEYDOWN       0x0100
#define WM_CHAR          0x0102
#define WM_INITDIALOG    0x0110
#define WM_COMMAND       0x0111
#define WM_SYSCOMMAND    0x0112
#define WM_TIMER         0x0113
#define WM_HSCROLL       0x0114
#define WM_VSCROLL       0x0115
#define WM_MOUSEMOVE     0x0200
#define WM_LBUTTONDOWN   0x0201
#define WM_RBUTTONDOWN   0x0204
#define WM_RBUTTONUP     0x0205

// Control notifications.
#define BN_CLICKED       0
#define CBN_SELCHANGE    1
#define CBN_EDITCHANGE   5
#define LBN_SELCHANGE    1
#define LBN_DBLCLK       2
#define EN_CHANGE        0x0300

// Combobox and listbox messages.
#define CB_GETEDITSEL    0x0140
#define CB_ADDSTRING     0x0143
#define CB_DELETESTRING  0x0144
#define CB_GETCOUNT      0x0146
#define CB_GETCURSEL     0x0147
#define CB_GETLBTEXT     0x0148
#define CB_INSERTSTRING  0x014A
#define CB_RESETCONTENT  0x014B
#define CB_FINDSTRING    0x014C
#define CB_SELECTSTRING  0x014D
#define CB_SETCURSEL     0x014E
#define CB_ERR           (-1)
#define LB_ADDSTRING     0x0180
#define LB_INSERTSTRING  0x0181
#define LB_DELETESTRING  0x0182
#define LB_RESETCONTENT  0x0184
#define LB_SETCURSEL     0x0186
#define LB_GETCURSEL     0x0188
#define LB_GETTEXT       0x0189
#define LB_GETCOUNT      0x018B
#define LB_GETITEMDATA   0x0199
#define LB_SETITEMDATA   0x019A
#define LB_INITSTORAGE   0x01A8
#define LB_ERR           (-1)
#define EM_SETSEL        0x00B1
#define EM_LIMITTEXT     0x00C5

// Dialog results and button states.
#define IDOK             1
#define IDCANCEL         2
#define IDABORT          3
#define IDRETRY          4
#define IDIGNORE         5
#define IDYES            6
#define IDNO             7
#ifndef IDC_STATIC
#define IDC_STATIC       (-1)
#endif
#define BST_UNCHECKED    0
#define BST_CHECKED      1

// Menu flags.
#define MF_STRING        0x0000
#define MF_ENABLED       0x0000
#define MF_UNCHECKED     0x0000
#define MF_BYCOMMAND     0x0000
#define MF_GRAYED        0x0001
#define MF_DISABLED      0x0002
#define MF_CHECKED       0x0008
#define MF_BYPOSITION    0x0400
#define MF_SEPARATOR     0x0800
#define TPM_RIGHTBUTTON  0x0002

// Message box flags.
#define MB_OK              0x0000
#define MB_OKCANCEL        0x0001
#define MB_YESNO           0x0004
#define MB_ICONSTOP        0x0010
#define MB_ICONQUESTION    0x0020
#define MB_ICONEXCLAMATION 0x0030
#define MB_ICONINFORMATION 0x0040

// Show window and window position flags.
#define SW_HIDE          0
#define SW_NORMAL        1
#define SW_SHOWNORMAL    1
#define SW_SHOW          5
#define HWND_NOTOPMOST   ((HWND)(intptr_t)-2)
#define RDW_INVALIDATE   0x0001

// Scrollbars.
#define SB_HORZ          0
#define SB_VERT          1
#define SB_BOTH          3
#define SB_LINEUP        0
#define SB_LINELEFT      0
#define SB_LINEDOWN      1
#define SB_LINERIGHT     1
#define SB_PAGEUP        2
#define SB_PAGELEFT      2
#define SB_PAGEDOWN      3
#define SB_PAGERIGHT     3
#define SB_THUMBPOSITION 4
#define SB_THUMBTRACK    5
#define SB_TOP           6
#define SB_LEFT          6
#define SB_BOTTOM        7
#define SB_RIGHT         7

// Keys and mouse buttons.
#define MK_LBUTTON       0x0001
#define MK_RBUTTON       0x0002
#define MK_SHIFT         0x0004
#define MK_CONTROL       0x0008
#define VK_SHIFT         0x10
#define VK_CONTROL       0x11
#define VK_MENU          0x12
#define VK_F1            0x70
#define VK_F12           0x7B

// Cursors.
#define IDC_ARROW        ((LPCSTR)(intptr_t)32512)
#define IDC_WAIT         ((LPCSTR)(intptr_t)32514)

// GDI.
#define TRANSPARENT      1
#define OPAQUE           2
#define MM_TEXT          1
#define MM_ANISOTROPIC   8
#define HORZRES          8
#define VERTRES          10
#define LOGPIXELSX       88
#define LOGPIXELSY       90
#define NULL_BRUSH       5
#define NULL_PEN         8
#define BLACKNESS        0x00000042
#define WHITENESS        0x00FF0062
#define PATCOPY          0x00F00021
#define SRCCOPY          0x00CC0020

// Common dialog flags.
#define OFN_OVERWRITEPROMPT 0x00000002
#define OFN_HIDEREADONLY    0x00000004
#define OFN_PATHMUSTEXIST   0x00000800
#define OFN_FILEMUSTEXIST   0x00001000
#define PD_NOSELECTION      0x00000004
#define PD_NOPAGENUMS       0x00000008
#define PD_PRINTSETUP       0x00000040
#define PD_RETURNDC         0x00000100
#define PD_USEDEVMODECOPIES 0x00040000
#define CC_RGBINIT          0x00000001
#define CC_FULLOPEN         0x00000002


/*
******************************************************************************
** Common Dialog Structures.
******************************************************************************
*/

// Field order matches Win32 so the existing initializers in xdata.cpp work.

typedef struct tagOFN {
  DWORD lStructSize;
  HWND hwndOwner;
  HINSTANCE hInstance;
  LPCSTR lpstrFilter;
  LPSTR lpstrCustomFilter;
  DWORD nMaxCustFilter;
  DWORD nFilterIndex;
  LPSTR lpstrFile;
  DWORD nMaxFile;
  LPSTR lpstrFileTitle;
  DWORD nMaxFileTitle;
  LPCSTR lpstrInitialDir;
  LPCSTR lpstrTitle;
  DWORD Flags;
  WORD nFileOffset;
  WORD nFileExtension;
  LPCSTR lpstrDefExt;
  LPARAM lCustData;
  void *lpfnHook;
  LPCSTR lpTemplateName;
} OPENFILENAME, *LPOPENFILENAME;

typedef struct tagPD {
  DWORD lStructSize;
  HWND hwndOwner;
  HGLOBAL hDevMode;
  HGLOBAL hDevNames;
  HDC hDC;
  DWORD Flags;
  WORD nFromPage;
  WORD nToPage;
  WORD nMinPage;
  WORD nMaxPage;
  WORD nCopies;
  HINSTANCE hInstance;
  LPARAM lCustData;
  void *lpfnPrintHook;
  void *lpfnSetupHook;
  LPCSTR lpPrintTemplateName;
  LPCSTR lpSetupTemplateName;
  HGLOBAL hPrintTemplate;
  HGLOBAL hSetupTemplate;
} PRINTDLG, *LPPRINTDLG;

typedef struct tagCHOOSECOLOR {
  DWORD lStructSize;
  HWND hwndOwner;
  HWND hInstance;
  COLORREF rgbResult;
  COLORREF *lpCustColors;
  DWORD Flags;
  LPARAM lCustData;
  void *lpfnHook;
  LPCSTR lpTemplateName;
} CHOOSECOLOR, *LPCHOOSECOLOR;


/*
******************************************************************************
** Win32 Subset Functions (implemented in wqt.cpp).
******************************************************************************
*/

// Messages and windows.
extern LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM);
extern BOOL PostMessage(HWND, UINT, WPARAM, LPARAM);
extern BOOL SetWindowText(HWND, LPCSTR);
extern BOOL GetWindowRect(HWND, LPRECT);
extern BOOL GetClientRect(HWND, LPRECT);
extern BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT);
extern BOOL MoveWindow(HWND, int, int, int, int, BOOL);
extern BOOL ClientToScreen(HWND, LPPOINT);
extern BOOL ScreenToClient(HWND, LPPOINT);
extern BOOL GetCursorPos(LPPOINT);
extern HWND GetDesktopWindow(void);
extern HWND GetTopWindow(HWND);
extern int SetScrollPos(HWND, int, int, BOOL);
extern BOOL SetScrollRange(HWND, int, int, int, BOOL);
extern BOOL ShowScrollBar(HWND, int, BOOL);
extern BOOL RedrawWindow(HWND, const RECT *, HRGN, UINT);
extern BOOL InvalidateRect(HWND, const RECT *, BOOL);
extern BOOL ShowWindow(HWND, int);
extern BOOL EnableWindow(HWND, BOOL);
extern HWND SetFocus(HWND);
extern BOOL DestroyWindow(HWND);
extern short GetKeyState(int);
extern HCURSOR SetCursor(HCURSOR);
extern HCURSOR LoadCursor(HINSTANCE, LPCSTR);
extern int ShowCursor(BOOL);
extern uintptr_t SetTimer(HWND, uintptr_t, UINT, TIMERPROC);
extern BOOL KillTimer(HWND, uintptr_t);
extern int MessageBox(HWND, LPCSTR, LPCSTR, UINT);
extern BOOL MessageBeep(UINT);
extern void PostQuitMessage(int);

// Menus.
extern HMENU LoadMenu(HINSTANCE, LPCSTR);
extern HMENU GetSubMenu(HMENU, int);
extern BOOL DestroyMenu(HMENU);
extern BOOL DrawMenuBar(HWND);
extern DWORD CheckMenuItem(HMENU, UINT, UINT);
extern BOOL CheckMenuRadioItem(HMENU, UINT, UINT, UINT, UINT);
extern BOOL EnableMenuItem(HMENU, UINT, UINT);
extern BOOL ModifyMenu(HMENU, UINT, UINT, uintptr_t, LPCSTR);
extern BOOL DeleteMenu(HMENU, UINT, UINT);
extern int GetMenuString(HMENU, UINT, LPSTR, int, UINT);
extern BOOL TrackPopupMenu(HMENU, UINT, int, int, int, HWND, const RECT *);

// Dialogs.
extern intptr_t DialogBox(HINSTANCE, LPCSTR, HWND, DLGPROC);
extern BOOL EndDialog(HWND, intptr_t);
extern HWND GetDlgItem(HWND, int);
extern BOOL SetDlgItemText(HWND, int, LPCSTR);
extern UINT GetDlgItemText(HWND, int, LPSTR, int);
extern BOOL SetDlgItemInt(HWND, int, UINT, BOOL);
extern BOOL CheckDlgButton(HWND, int, UINT);
extern UINT IsDlgButtonChecked(HWND, int);
extern BOOL CheckRadioButton(HWND, int, int, int);
extern LRESULT SendDlgItemMessage(HWND, int, UINT, WPARAM, LPARAM);

// Common dialogs.
extern BOOL GetOpenFileName(LPOPENFILENAME);
extern BOOL GetSaveFileName(LPOPENFILENAME);
extern BOOL ChooseColor(LPCHOOSECOLOR);
extern BOOL PrintDlg(LPPRINTDLG);

// GDI subset (text output to the window).
extern HDC GetDC(HWND);
extern int ReleaseDC(HWND, HDC);
extern int GetDeviceCaps(HDC, int);
extern BOOL TextOut(HDC, int, int, LPCSTR, int);
extern COLORREF SetTextColor(HDC, COLORREF);
extern int SetBkMode(HDC, int);
extern HGDIOBJ SelectObject(HDC, HGDIOBJ);
extern BOOL DeleteObject(HGDIOBJ);
extern HGDIOBJ GetStockObject(int);
extern int SetMapMode(HDC, int);
extern BOOL SetWindowOrgEx(HDC, int, int, LPPOINT);
extern BOOL SetWindowExtEx(HDC, int, int, SIZE *);
extern BOOL SetViewportOrgEx(HDC, int, int, LPPOINT);
extern BOOL SetViewportExtEx(HDC, int, int, SIZE *);

// System.
extern DWORD GetModuleFileName(HINSTANCE, LPSTR, DWORD);
extern UINT WinExec(LPCSTR, UINT);
extern HINSTANCE ShellExecute(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int);
extern UINT GetWindowsDirectory(LPSTR, UINT);
extern HANDLE CreateMutex(void *, BOOL, LPCSTR);
extern DWORD WaitForSingleObject(HANDLE, DWORD);
extern BOOL ReleaseMutex(HANDLE);
extern BOOL CloseHandle(HANDLE);


/*
******************************************************************************
** Qt Version Specific Routines (implemented in wqt.cpp).
******************************************************************************
*/

// The Qt versions of FRedraw(), FFilePaste(), DlgPrint(), WinClearScreen(),
// DrawGlyph(), and ResizeWindowToChart() are also in wqt.cpp, replacing the
// GDI based ones.

// Set while FActionX() draws a chart for the window (into gi.bmp), as
// opposed to a chart file. TrueType fonts are only drawn in this case.
extern int fQtScreenChart;

// Draw one character of a TrueType font into the window chart bitmap.
extern void QtDrawFontChar(int wch, int x, int y, int fi, int nHeight,
  int fCenter);

#endif // WQT_H

/* wqt.h */
