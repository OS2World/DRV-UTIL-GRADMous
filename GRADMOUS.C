/****************************************************************************

    PROGRAM: gradmous.c

    VERSION: 1.00
    DATE: 25. Feb. 2002
    LAST UPDATE: 9. May. 2002

    PURPOSE: The fullscreen GRADD driver for WinOS/2 (IFGDI2VM.DRV)
             has the following bug.
             The speed of the mous is horizontally not the same as vertically.
             This program installs a hook and filters the MouseMove Messages.
             And sets the Cursor to the propper position.


    FUNCTIONS:

        WinMain()
        MouseHookProc( int, WPARAM, LPARAM);
        CmdLineCheck (LPSTR);
        WndMain(HWND, WORD, WORD, LONG);
        AboutDlgProc () 

    COMMENTS:

****************************************************************************/


#include <windows.h>
#include <dir.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "aboutdlg.h"

#define VERSION "1.0.0"
#define PROGRAM "GRADMOUS.EXE"
#define COMMENT "Corrects Mouse Speed\n in Y-Direction"

// Prototypes

int      PASCAL   WinMain( HANDLE, HANDLE, LPSTR, int);
DWORD    CALLBACK MouseHookProc( int, WPARAM, LPARAM);
void              CmdLineCheck (LPSTR);
LONG FAR PASCAL   WndMain(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL   AboutDlgProc (HWND, WORD, WORD, LONG);

typedef MOUSEHOOKSTRUCT _far *LPMOUSEHOOKSTRUCT;

HTASK   hMyTask;
HHOOK   hMouseHook;
FARPROC lpMouseHookProc;
BOOL    bActive = TRUE;
BOOL    bRunning = FALSE;
POINT   ptLast, ptNew;
char    achDisplayDriverName[MAXPATH];
HANDLE  hDisplayDriver;
int     nLength;
int     nScreenHeight;
BOOL    bQuiet = FALSE;
BOOL    bVerbose = FALSE;
BOOL    bHidden = FALSE;
LPSTR   lpExec;
HMENU   hSysMenu;
HANDLE  hInstance;
HICON   hIcoEna, hIcoDis, hIcoMain;
WNDCLASS wc;
FARPROC lpitAbout;

#pragma argsused
int PASCAL WinMain( HANDLE hInst, HANDLE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   HWND     hwMain;
   FILE * screen;
   void far *fbuffer;

   hInstance = hInst;
   CmdLineCheck (lpCmdLine);
   hDisplayDriver = GetModuleHandle ("DISPLAY");
   if (hDisplayDriver == NULL) {
      return 0;
   } // endif
   nLength = GetModuleFileName (hDisplayDriver, achDisplayDriverName, MAXPATH);
   if (stricmp (&achDisplayDriverName[nLength - 12], "ifgdi2vm.drv") != 0) {
      if (bVerbose)
         MessageBox (NULL,&achDisplayDriverName[nLength - 12], "Wrong Display Driver", MB_OK | MB_ICONSTOP);
      return 0;
   } else {
   } // endif

   if ( hPrevInst) { 
      // On second call terminate
      GetInstanceData( hPrevInst, (NPSTR)&hMyTask, sizeof( hMyTask));
      PostAppMessage( hMyTask, WM_QUIT, 0, 0); // terminate old instance
      if (!bQuiet)
         MessageBox( NULL, "Previous Instance terminated", "Info", MB_OK);
      return 0; 
   } // endif

   hIcoEna = LoadIcon (hInst, "ICON_1");
   hIcoDis = LoadIcon (hInst, "ICON_2");
   if (bActive) {
      hIcoMain = hIcoEna;
   } else {
      hIcoMain = hIcoDis;
   } // endif

   wc.lpszClassName="GraddMouse";
   wc.hInstance=hInst;
   wc.lpfnWndProc=(WNDPROC)WndMain;
   wc.hCursor=LoadCursor(0,IDC_ARROW);
   wc.hbrBackground=GetStockObject (GetSysColor(COLOR_BACKGROUND));
   wc.style= 0;
   wc.hIcon = hIcoMain; 
   wc.lpszMenuName=NULL;
   wc.cbClsExtra=0;
   wc.cbWndExtra=0;
   // register window, terminate program if error
   if (!RegisterClass(&wc)) {
      MessageBox (NULL, "RegisterClass failed", "Error", MB_OK | MB_ICONSTOP);
      return 255;
   } // endif

   // create application window if required
   if (!bHidden) {
      hwMain=CreateWindow("GraddMouse",
             "Gradd Mouse",
             WS_SYSMENU | WS_OVERLAPPEDWINDOW | WS_ICONIC,
             CW_USEDEFAULT,
             0,
             CW_USEDEFAULT,
             0,
             0,
             0,
             hInst,
             NULL);
      if (!hwMain) {
         MessageBox (NULL, "CreateWindow failed", "error", MB_OK | MB_ICONSTOP);
         return 255; // error during creation => terminate program
      } // endif
      // show created main window
      lpitAbout = MakeProcInstance (AboutDlgProc, hInst);
      ShowWindow(hwMain,SW_MINIMIZE); //idCmdShow);
   } // endif
   hMyTask= GetCurrentTask(); // Save TaskID for Termination
   lpMouseHookProc= MakeProcInstance( (FARPROC)&MouseHookProc, hInst);
   if ( lpMouseHookProc) {
      // Install  Filter
      hMouseHook = SetWindowsHookEx( WH_MOUSE, lpMouseHookProc, hInst, NULL);
   } // endif
   if (lpExec != NULL) {
      char achTemp[256];
      int n;
      n = WinExec (lpExec, SW_SHOW);
      if ((n < 32) && (!bQuiet)) {
         switch (n) {
            case 0:
               sprintf (achTemp, "%d: Not enough memory", n);
            break;
            case 2:
               sprintf (achTemp, "%d: File not found", n);
            break;
            case 3:
               sprintf (achTemp, "%d: Path not found", n);
            break;
            case 11:
               sprintf (achTemp, "%d: Invalid EXE", n);
            break;
            case 14:
               sprintf (achTemp, "%d: Unknown EXE-type", n);
            break;
            default:
               sprintf (achTemp, "%d: Other Error", n);
            break;
         } // endswitch
         MessageBox (0, achTemp, "WinExec Error", MB_OK |MB_ICONSTOP);
      } // endif
   } // endif

   if ( hMouseHook) {
      // Stay in task if installation was successfull
      MSG msg;
      nScreenHeight = GetSystemMetrics (SM_CYSCREEN);
      GetCursorPos (&ptLast);
      while ( GetMessage( &msg, NULL, NULL, NULL)) {
         /* bis WM_Quit */
         TranslateMessage( &msg);
         DispatchMessage( &msg);
      } // endwhile
      UnhookWindowsHookEx( hMouseHook); // Remove filter
   } else {
      // Installation failed
      if (!bQuiet)
         MessageBox( NULL, "Can't initialize ... ", "Error", MB_OK);
      if ( lpMouseHookProc) {
         // Free instance address if required
         FreeProcInstance( lpMouseHookProc);
      } // endif
   } // endif
   return 0;
} // end WinMain


/* System Filter for Mousmessages */
DWORD CALLBACK MouseHookProc( int Code, WPARAM wParam, LPARAM lParam) {
   LPMOUSEHOOKSTRUCT lpMHS= (LPMOUSEHOOKSTRUCT)lParam;
   if (bActive) { 
      if (( Code >= 0) && (( wParam == WM_MOUSEMOVE) || ( wParam == WM_NCMOUSEMOVE))){
         if (bRunning) {
            // ignore Mousemove
            return (1);
         } // endif
         if (ptLast.y != lpMHS->pt.y) {
            /* Only if a move in y-direction */
            bRunning = TRUE;
            ptNew.x = lpMHS->pt.x;
            if ((lpMHS->pt.y == nScreenHeight - 1) && (ptLast.y == nScreenHeight -2)) {
               ptNew.y = nScreenHeight -1;
            } else { 
               ptNew.y = (ptLast.y + lpMHS->pt.y) / 2; 
            } // endif
            SetCursorPos (ptNew.x, ptNew.y);  // New Position
            ptLast.x = ptNew.x;
            ptLast.y = ptNew.y;
            bRunning = FALSE;
            return (1);
         } // endif 
      } // endif
   } // endif
   return ( CallNextHookEx( hMouseHook, Code, wParam, lParam));
} // end MouseHookProc


void CmdLineCheck (LPSTR lpCmdLine) {
   char *chBuf, *chD, *chH, *chQ, *chV, *chX;

   chBuf = malloc (_fstrlen (lpCmdLine) + 1);
   _fstrcpy (chBuf, lpCmdLine);
   strupr (chBuf);
   chQ = strstr (chBuf, "/Q");  // Quiet
   chV = strstr (chBuf, "/V");  // Verbose
   chX = strstr (chBuf, "/X");  // eXecute
   chD = strstr (chBuf, "/D");  // Disable
   chH = strstr (chBuf, "/H");  // Hide

   if (chX != NULL) {
      lpExec = (LPSTR)&lpCmdLine [_fstrlen(lpCmdLine) - strlen(chX) + 2];
      while ((lpExec[0] == ' ') && (lpExec[0] != 0)) {
         lpExec = &lpExec[1];
      } // endwhile
      if (lpExec[0] == 0) {
         lpExec = NULL;
      } // endif
      if ((chQ != NULL) && (chX > chQ)) {
         bQuiet = TRUE;
      } else {
         bQuiet = FALSE;
      } // endif
      if ((chV != NULL) && (chX > chV)) {
         bVerbose = TRUE;
      } else {
         bVerbose = FALSE;
      } // endif
      if ((chD != NULL) && (chX > chD)) {
         bActive = FALSE;
      } else {
         bActive = TRUE;
      } // endif
      if ((chH != NULL) && (chX > chD)) {
         bHidden = TRUE;
      } else {
         bHidden = FALSE;
      } // endif
   } else {
      lpExec = NULL;
      if (chQ != NULL) {
         bQuiet = TRUE;
      } else {
         bQuiet = FALSE;
      } // endif
      if (chV != NULL) {
         bVerbose = TRUE;
      } else {
         bVerbose = FALSE;
      } // endif
      if (chD != NULL) {
         bActive = FALSE;
      } else {
         bActive = TRUE;
      } // endif
      if (chH != NULL) {
         bHidden = TRUE;
      } else {
         bHidden = FALSE;
      } // endif

   } // endif
   if (chBuf != NULL) {
      free (chBuf);
   } // endif
} // end CmdLineCheck 


LONG FAR PASCAL WndMain(HWND hw,WORD iMsg,WORD uP1,LONG lP2) {
   switch (iMsg) {
      case WM_CREATE:
         hSysMenu = GetSystemMenu (hw, 0);
         DeleteMenu (hSysMenu, SC_RESTORE, MF_BYCOMMAND);
         DeleteMenu (hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
         DeleteMenu (hSysMenu, SC_MINIMIZE, MF_BYCOMMAND);
         DeleteMenu (hSysMenu, SC_SIZE, MF_BYCOMMAND);
         InsertMenu (hSysMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
         if (bActive) {
            InsertMenu (hSysMenu, -1, MF_BYPOSITION | MF_STRING, 0xE000, (LPSTR) "&Disable");
         } else {
            InsertMenu (hSysMenu, -1, MF_BYPOSITION | MF_STRING, 0xE000, (LPSTR) "&Enable");
         } // endif
         InsertMenu (hSysMenu, -1, MF_BYPOSITION | MF_STRING, 0xE010, (LPSTR) "&About...");
         DrawMenuBar (hSysMenu);
      break;
      case WM_DESTROY:
         PostQuitMessage(0);
      break;
      case WM_SYSCOMMAND:
         switch (uP1 & 0xFFF0){
            case SC_RESTORE:
            case SC_MAXIMIZE:
               return 0;
            case SC_KEYMENU:
            break;
            case 0xE000:
               if (bActive) {
                  bActive = FALSE;
                  hIcoMain = hIcoDis;
                  ModifyMenu (GetSystemMenu (hw,0), 0xE000, MF_BYCOMMAND | MF_STRING, 0xE000, "&Enable");
               } else {
                  bActive = TRUE;
                  hIcoMain = hIcoEna;
                  ModifyMenu (GetSystemMenu (hw,0), 0xE000, MF_BYCOMMAND | MF_STRING, 0xE000, "&Disable");
               } // endif
               SetClassWord (hw, GCW_HICON, hIcoMain);
               InvalidateRect (hw, NULL, 1);
            break;
            case 0xE010:
               // About 
               DialogBox (hInstance, MAKEINTRESOURCE (IDD_ABOUT), hw, lpitAbout);
            break;
         } // endswitch
      break;
   } // endswitch
   return DefWindowProc (hw,iMsg, uP1, lP2);
} // end WndMain

#pragma argsused
int FAR PASCAL AboutDlgProc (HWND hdlg, WORD iMsg,WORD uP1,LONG lP2) {
   switch (iMsg) {
      case WM_INITDIALOG:
         SetDlgItemText (hdlg, IDD_VERSION, VERSION);
         SetDlgItemText (hdlg, IDD_PROGRAM, PROGRAM);
         SetDlgItemText (hdlg, IDD_COMMENT, COMMENT);
         SetFocus (GetDlgItem (hdlg, IDOK));
         ShowWindow (hdlg, SW_SHOW);
         return FALSE;
      case WM_COMMAND:
         EndDialog (hdlg, TRUE);
         return TRUE;
   } // endswitch
   return FALSE;
} // end AboutDlgProc