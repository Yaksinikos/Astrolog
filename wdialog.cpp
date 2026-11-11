/*
** Astrolog (Version 8.00) File: wdialog.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2026 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** The main ephemeris databases and calculation routines are from the
** library SWISS EPHEMERIS and are programmed and copyright 1997-2008 by
** Astrodienst AG. Use of that source code is subject to license for Swiss
** Ephemeris Free Edition at https://www.astro.com/swisseph/swephinfo_e.htm.
** This copyright notice must not be changed or removed by any user of this
** program.
**
** Additional ephemeris databases and formulas are from the calculation
** routines in the program PLACALC and are programmed and Copyright (C)
** 1989,1991,1993 by Astrodienst AG and Alois Treindl (alois@astro.ch). The
** use of that source code is subject to regulations made by Astrodienst
** Zurich, and the code is not in the public domain. This copyright notice
** must not be changed or removed by any user of this program.
**
** The original planetary calculation routines used in this program have
** been copyrighted and the initial core of this program was mostly a
** conversion to C of the routines created by James Neely as listed in
** 'Manual of Computer Programming for Astrologers', by Michael Erlewine,
** available from Matrix Software.
**
** Atlas composed using data from https://www.geonames.org/ licensed under a
** Creative Commons Attribution 4.0 License. Time zone changes composed using
** public domain TZ database: https://data.iana.org/time-zones/tz-link.html
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby (brianw@sounds.wa.com).
**
** More formally: This program is free software; you can redistribute it
** and/or modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version. This program is
** distributed in the hope that it will be useful and inspiring, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details, a copy of which is in the
** LICENSE.HTM file included with Astrolog, and at http://www.gnu.org
**
** Initial programming 8/28-30/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 5/28/2026.
*/

#include "astrolog.h"


#ifdef WINUI
/*
******************************************************************************
** Dialog Utility Functions.
******************************************************************************
*/

// Set the contents of the given edit control in a dialog to a string.

void SetEditSz(HWND hdlg, int id, CONST char *sz)
{
  char sz2[cchSzDef];
  CONST char *pch;

  if (sz == NULL)
    sz = "";
  while (*sz == ' ')    // Strip off any extra leading spaces.
    sz++;
  pch = ConvertSzToLatin(sz, sz2, cchSzDef);
  SetEdit(id, pch);
}


// Set the contents of the given edit control in a dialog to a floating point
// value, with at most 'n' significant fractional digits.

void SetEditR(HWND hdlg, int id, real r, int n)
{
  char sz[cchSzDef];

  FormatR(sz, r, n);
  SetEdit(id, sz);
}


// Set the contents of four combo controls and their dropdowns in a dialog
// indicating month, day, year, and time fields to the given values.

void SetEditMDYT(HWND hdlg, int idMon, int idDay, int idYea, int idTim,
  int mon, int day, int yea, real tim)
{
  char sz[cchSzDef];
  int i;

  ClearCombo(idMon);
  ClearCombo(idDay);
  ClearCombo(idYea);
  ClearCombo(idTim);
  if (!FValidMon(mon))
    mon = 1;
  sprintf(sz, "%.3s", szMonth[mon]);
  SetEdit(idMon, sz);
  for (i = 1; i <= cSign; i++)
    SetCombo(idMon, szMonth[i]);
  if (!FValidDay(day, mon, yea))
    day = 1;
  SetEditN(idDay, day);
  for (i = 0; i <= 25; i += 5) {
    sprintf(sz, "%d", Max(i, 1)); SetCombo(idDay, sz);
  }
  SetEditN(idYea, yea);
  for (i = 2020; i <= 2030; i++) {
    sprintf(sz, "%d", i); SetCombo(idYea, sz);
  }
  sprintf(sz, "%s", SzTim(tim));
  SetEditSz(hdlg, idTim, sz);
  SetCombo(idTim, "Midnight");
  SetCombo(idTim, (char *)(us.fEuroTime ? "6:00" : "6:00am"));
  SetCombo(idTim, "Noon");
  SetCombo(idTim, (char *)(us.fEuroTime ? "18:00" : "6:00pm"));
}


// Set the contents of four combo controls in a dialog indicating daylight,
// time zone, longitude, and latitude fields to the given values.

void SetEditSZOA(HWND hdlg, int idDst, int idZon, int idLon, int idLat,
  real dst, real zon, real lon, real lat)
{
  char sz[cchSzDef];
  int i, nSav;

  ClearCombo(idDst);
  ClearCombo(idZon);
  ClearCombo(idLon);
  ClearCombo(idLat);
  if (dst == 0.0 || dst == 1.0 || dst == dstAuto)
    sprintf(sz, "%s",
      dst == 0.0 ? "No" : (dst == 1.0 ? "Yes" : "Autodetect"));
  else
    sprintf(sz, "%s", SzZone(dst));
  SetEdit(idDst, sz);
  SetCombo(idDst, "No"); SetCombo(idDst, "Yes");
  sprintf(sz, "%s", SzZone(zon));
  SetEdit(idZon, (char *)(sz[0] == '+' ? &sz[1] : sz));

  // For the time zone dropdown, fill it out with all abbreviations of three
  // letters that don't reference daylight or war time.
  for (i = 0; i < cZone; i++) {
    if (szZon[i][1] && szZon[i][1] != 'D' && szZon[i][1] != 'W' &&
      szZon[i][2] && szZon[i][2] != 'D') {
      if (rZon[i] != zonLMT && rZon[i] != zonLAT)
        sprintf(sz, "%s %s", SzZone(rZon[i]), szZon[i]);
      else
        sprintf(sz, "%s", SzZone(rZon[i]));
      SetCombo(idZon, sz);
    }
  }
  nSav = us.fAnsiChar; us.fAnsiChar = fFalse;
  sprintf(sz, "%s", SzLocation(lon, lat));
  us.fAnsiChar = nSav;
  sz[is.ichLocSplit] = chNull;
  SetEditSz(hdlg, idLon, &sz[0]);
  SetCombo(idLon, "122W20"); SetCombo(idLon, "0E00");
  SetEditSz(hdlg, idLat, &sz[is.ichLocSplit+1]);
  SetCombo(idLat, "47N36"); SetCombo(idLat, "0S00");
}


// Set the contents of a combo control in a dialog indicating a color field
// field to the given value, and fill its dropdown with the color list.

void SetEditColor(HWND hdlg, int id, KI ki, int nExtra)
{
  int i, iMax = cColor2 + (nExtra > 0)*(nExtra + 1);

  SetEdit(id, SzColor(ki));
  for (i = 0; i < iMax; i++) {
    SetCombo(id, szColor[i]);
    if (i == kWhite)
      i += (cColor2 - cColor) + (nExtra >= 2)*nExtra;
  }
}


// Return the contents of a dialog edit control as an integer value.

int GetEditN(HWND hdlg, int id)
{
  char sz[cchSzMax];

  GetEdit(id, sz);
  return NFromSz(sz);
}


// Return the contents of a dialog edit control as a floating point value.

real GetEditR(HWND hdlg, int id)
{
  char sz[cchSzMax];

  GetEdit(id, sz);
  return RFromSz(sz);
}


// Set the contents of the given listbox entry in a dialog to a string.

int SetListSz(HWND hdlg, int id, CONST char *sz)
{
  char sz2[cchSzMax];
  CONST char *pch;

  while (*sz == ' ')    // Strip off any extra leading spaces.
    sz++;
  pch = ConvertSzToLatin(sz, sz2, cchSzMax);
  return SetList(id, pch);
}


// Return which radio button in a radio button group is set.

int GetRadio(HWND hdlg, int id1, int id2)
{
  int i;

  for (i = id1; i <= id2; i++)
    if (GetCheck(i))
      return i - id1;
  return 0;
}


// Bring up an error box indicating an illegal value for a dialog field.

void ErrorEnsure(int n, CONST char *sz)
{
  char szT[cchSzDef];

  sprintf(szT, "The value %d is not a valid %s setting.", n, sz);
  PrintWarning(szT);
}


#ifdef ATLAS
// Shared WM_COMMAND processor for atlas and time change related button
// commands. Called from DlgInfo() and DlgDefault().

flag FDlgInfoAtlas(HWND hdlg, WORD wParam, flag fDefault)
{
  CI ci;
  char sz[cchSzMax];
  int dcDst, dcZon, dcLon, dcLat, deLoc, ilist, i, j;
  real lon, lat;

  // Variables different between the Chart Info and Defaults dialogs.
  dcDst = (fDefault ? dcDeDst : dcInDst);
  dcZon = (fDefault ? dcDeZon : dcInZon);
  dcLon = (fDefault ? dcDeLon : dcInLon);
  dcLat = (fDefault ? dcDeLat : dcInLat);
  deLoc = (fDefault ? deDeLoc : deInLoc);
  ilist = (fDefault ? 12 : 14);

  // "Lookup City" button:
  if (wParam == dbInCity) {
    GetEdit(deLoc, sz);
    ClearList(dlIn);
    if (!DisplayAtlasLookup(sz, (size_t)hdlg, &ilist))
      PrintWarning("Couldn't get atlas data!");

  // "Nearby Cities" button:
  } else if (wParam == dbInCoor) {
    GetEdit(dcLon, sz); lon = RParseSz(sz, pmLon);
    GetEdit(dcLat, sz); lat = RParseSz(sz, pmLat);
    ClearList(dlIn);
    if (!DisplayAtlasNearby(lon, lat, (size_t)hdlg, &ilist, fFalse))
      PrintWarning("Couldn't get atlas data!");

  // "Time Changes" button:
  } else if (wParam == dbInChan) {
    GetEdit(deLoc, sz);
    if (!DisplayAtlasLookup(sz, 0, &i)) {
      PrintWarning("Please have a valid city in the 'Location' field first.");
      return fTrue;
    }
    ClearList(dlIn);
    ci = ciMain;
    if (!fDefault) {
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
    }
    if (!DisplayTimezoneChanges(is.rgae[i].izn, (size_t)hdlg, &ci))
      PrintWarning("Couldn't get time zone data!");

  // "Apply Info" button:
  } else if (wParam == dbInAppl) {
    j = (int)SendDlgItemMessage(hdlg, dlIn, LB_GETCURSEL, 0, 0);
    i = (int)SendDlgItemMessage(hdlg, dlIn, LB_GETITEMDATA, j, 0);
    if (i < 0) {
      GetEdit(deLoc, sz);
      if (!DisplayAtlasLookup(sz, 0, &i)) {
        PrintWarning("Please have a valid city selected in 'Atlas Lookups', "
          "or a valid city already in the 'Location' field.");
        return fTrue;
      }
    }
    SetEditSz(hdlg, deLoc, SzCity(i));
    ci = ciMain;
    if (!fDefault) {
      GetEdit(dcInMon, sz); ci.mon = NParseSz(sz, pmMon);
      GetEdit(dcInDay, sz); ci.day = NParseSz(sz, pmDay);
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
      GetEdit(dcInTim, sz); ci.tim = RParseSz(sz, pmTim);
    }
    if (!DisplayTimezoneChanges(is.rgae[i].izn, 0, &ci))
      PrintWarning("Couldn't get time zone data!");
    SetEditSZOA(hdlg, dcDst, dcZon, dcLon, dcLat,
      ci.dst, ci.zon, is.rgae[i].lon, is.rgae[i].lat);
  }
  return fFalse;
}
#endif


// Bring up the Windows color picker dialog. Return the color selected.

KV KvDialog()
{
  chc.rgbResult = KvFromKi(gi.kiOn);
  chc.lpCustColors = (dword *)rgbbmp;
  ChooseColor(&chc);
  return chc.rgbResult;
}


/*
******************************************************************************
** Windows Dialogs.
******************************************************************************
*/

// Bring up the Windows standard file open dialog, and process the command
// file specified if any. This is called from the File \ Open Chart and
// File / Open Chart #2 menu commands.

flag API DlgOpenChart()
{
  char sz[cchSzDef];
  CI ciT;
  HCURSOR hcurPrev = NULL;

  // Setup dialog title and settings and get the name of a file from it.
  if (us.fNoRead) {
    PrintWarning("File input is disabled.");
    return fFalse;
  }
  if (wi.nDlgChart > 0) {
    sprintf(sz, "Open Chart #%d", wi.nDlgChart);
    if (wi.nDlgChart < 2)
      sz[10] = chNull;
    ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0"
      "Astrological Exchange Files (*.aaf)\0*.aaf\0"
      "Quick*Chart Files (*.qck)\0*.qck\0"
      "Astrodatabank Files (*.xml)\0*.xml\0"
      "iCalendar Files (*.ics)\0*.ics\0"
      "Text Files [Solar Fire] (*.txt)\0*.txt\0"
      "All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.as");
  } else {
    sprintf(sz, "Open %s", wi.nDlgChart == 0 ? "Background" : "World Map");
    ofn.lpstrFilter = "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.bmp");
  }
  ofn.lpstrTitle = sz;
  if (!GetOpenFileName((LPOPENFILENAME)&ofn))
    return fFalse;

  // Process the given file based on what open command is being run.
  HourglassOn;
  if (wi.nDlgChart <= 0) {
    FLoadBmp(ofn.lpstrFileTitle,
      wi.nDlgChart == 0 ? &gi.bmpBack : &gi.bmpWorld, fFalse);
    HourglassOff;
    wi.fRedraw = fTrue;
    return fTrue;
  }

  ciT = ciCore;
  FInputData(ofn.lpstrFileTitle);
  if (wi.nDlgChart <= 1)
    cp1 = cp0;
  else {
    *rgpci[wi.nDlgChart] = ciCore;
    *rgpcp[wi.nDlgChart] = cp0;
    ciCore = ciT;
  }
  HourglassOff;

  wi.fCast = fTrue;
  return fTrue;
}


// Bring up the Windows standard file save dialog, get the name of a file from
// the user, and save to it either right away or set variables to ensure it
// will be done later. This is called from all six File / Save menu commands:
// Save Info, Positions, Text, Bitmap, Picture, PostScript, and Wireframe.

flag API DlgSaveChart()
{
  char *pch;

  // Setup dialog title and settings and get the name of a file from it.
  if (us.fNoWrite) {
    PrintWarning("File output is disabled.");
    return fFalse;
  }
  ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0All Files (*.*)\0*.*\0";
  ofn.lpstrDefExt = "as";
  sprintf(szFileName, "*.as");
  switch (wi.wCmd) {
  case cmdSaveChart:
    ofn.lpstrTitle = "Save Chart Info";
    break;
  case cmdSavePositions:
    ofn.lpstrTitle = "Save Chart Positions";
    break;
  case cmdSaveSettings:
    ofn.lpstrTitle = "Save Program Settings";
    sprintf(szFileName, "%s", DEFAULT_INFOFILE);
    break;
  case cmdSaveList:
    ofn.lpstrTitle = "Save Chart List";
    break;
  case cmdSaveAAF:
    ofn.lpstrTitle = "Save Chart Exchange Format";
    ofn.lpstrFilter =
      "Astrological Exchange Files (*.aaf)\0*.aaf\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "aaf";
    sprintf(szFileName, "*.aaf");
    break;
  case cmdSaveQuick:
    ofn.lpstrTitle = "Save Chart Quick*Chart Format";
    ofn.lpstrFilter =
      "Quick*Chart Files (*.qck)\0*.qck\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "qck";
    sprintf(szFileName, "*.qck");
    break;
  case cmdSaveCalendar:
    ofn.lpstrTitle = "Save Chart Calendar Format";
    ofn.lpstrFilter =
      "iCalendar Files (*.ics)\0*.ics\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "ics";
    sprintf(szFileName, "*.ics");
    break;
  case cmdSaveText:
    if (!us.fTextHTML) {
      ofn.lpstrTitle = "Export Chart Text";
      ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "txt";
      sprintf(szFileName, "*.txt");
    } else {
      ofn.lpstrTitle = "Export Chart HTML Text";
      ofn.lpstrFilter = "HTML Files (*.htm)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "htm";
      sprintf(szFileName, "*.htm");
    }
    break;
  case cmdSaveBitmap:
    if (gs.chBmpMode != 'P') {
      ofn.lpstrTitle = "Export Chart Bitmap";
      ofn.lpstrFilter =
        "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "bmp";
      sprintf(szFileName, "*.bmp");
    } else {
      ofn.lpstrTitle = "Export Chart PNG Bitmap";
      ofn.lpstrFilter =
        "Portable Network Graphics (*.png)\0*.png\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "png";
      sprintf(szFileName, "*.png");
    }
    break;
  case cmdSavePicture:
    ofn.lpstrTitle = "Export Chart Metafile";
    ofn.lpstrFilter =
      "Windows Metafiles (*.wmf)\0*.wmf\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "wmf";
    sprintf(szFileName, "*.wmf");
    break;
  case cmdSavePS:
    ofn.lpstrTitle = "Export Chart PostScript";
    ofn.lpstrFilter =
      "PostScript Text (*.eps)\0*.eps\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "eps";
    sprintf(szFileName, "*.eps");
    break;
  case cmdSaveSVG:
    ofn.lpstrTitle = "Export Chart SVG";
    ofn.lpstrFilter =
      "Scalable Vector Graphics (*.svg)\0*.svg\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "svg";
    sprintf(szFileName, "*.svg");
    break;
  case cmdSaveWire:
    ofn.lpstrTitle = "Export Chart Wireframe";
    ofn.lpstrFilter =
      "Daedalus Wireframes (*.dw)\0*.dw\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "dw";
    sprintf(szFileName, "*.dw");
    break;
  }
  if (wi.wCmd != cmdSaveWallTile && wi.wCmd != cmdSaveWallCenter &&
    wi.wCmd != cmdSaveWallStretch && wi.wCmd != cmdSaveWallFit &&
    wi.wCmd != cmdSaveWallFill) {
    if (!GetSaveFileName((LPOPENFILENAME)&ofn))
      return fFalse;
  } else {
    if (ofn.lpstrFile == NULL || ofn.lpstrFile == szFileName)
      ofn.lpstrFile = (char *)PAllocate(cchSzMax, "filename");
    if (ofn.lpstrFile == NULL)
      return fFalse;
    GetWindowsDirectory(ofn.lpstrFile, cchSzMax);
    pch = ofn.lpstrFile + CchSz(ofn.lpstrFile);
    sprintf(pch, "%c%s%s", '\\', szAppName, ".bmp");
  }

  // Saving chart info, position, or setting command files can be done here.
  // Saving actual chart output isn't done until the next redraw.
  FCloneSz(ofn.lpstrFile, &is.szFileOut);
  FCloneSz(ofn.lpstrFile, &gi.szFileOut);
  switch (wi.wCmd) {
  case cmdSaveChart:
    us.nWriteFormat = 0;
    return FOutputData();
  case cmdSavePositions:
    us.nWriteFormat = '0';
    return FOutputData();
  case cmdSaveList:
    us.nWriteFormat = 'l';
    return FOutputData();
  case cmdSaveText:
    FCloneSz(ofn.lpstrFile, &is.szFileScreen);
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;
  case cmdSaveBitmap:
  case cmdSaveWallTile:
  case cmdSaveWallCenter:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    gs.ft = ftBmp;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePicture:
    gs.ft = ftWmf;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePS:
    gs.ft = ftPS;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveSVG:
    gs.ft = ftSVG;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveWire:
    gs.ft = ftWire;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveSettings:
    return FOutputSettings();
  case cmdSaveAAF:
    return FOutputAAFFile();
  case cmdSaveQuick:
    return FOutputQuickFile();
  case cmdSaveCalendar:
    return FOutputCalendarFile();
  }
  return fTrue;
}


#ifdef WSETUP
// Bring up the Windows standard folder selection dialog, and open all the
// Astrolog files within that folder into the chart list. Called from the
// File / Other Formats / Open Charts in Folder command.

flag API DlgOpenDir()
{
  BROWSEINFO bi;
  LPITEMIDLIST pidl;
  char szDir[cchSzMax], szFile[cchSzMax];

  sprintf(szFile, "Select directory containing Astrolog *.%s chart files:",
    !us.fWriteOld ? "as" : "*");
  bi.hwndOwner = wi.hwnd;
  bi.pidlRoot  = NULL;
  bi.pszDisplayName = NULL;
  bi.lpszTitle = szFile;
  bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
  bi.lpfn      = NULL;
  bi.lParam    = NULL;

  pidl = SHBrowseForFolder(&bi);
  if (pidl == NULL)
    return fFalse;
  SHGetPathFromIDList(pidl, szDir);

  OpenDir(szDir);
  wi.fCast = fTrue;
  return fTrue;
}
#endif


#ifdef WIN
// Bring up the Windows standard print dialog, receive any printing settings
// from the user, and proceed to print the current graphics or text chart as
// displayed in the window. Called from the File / Print command.

flag API DlgPrint()
{
  DLGPROC lpAbortDlg;
  ABORTPROC lpAbortProc;
  HDC hdc;
  LPDEVMODE lpDevMode = NULL;
  LPDEVNAMES lpDevNames;
  LPSTR lpszDriverName;
  LPSTR lpszDeviceName;
  LPSTR lpszPortName;
  DOCINFO doci;
  int xPrint, yPrint;
  int xClient, yClient;
  flag fInverse;

  // Bring up the Windows print dialog.
  wi.fNoUpdate = fFalse;
  if (!PrintDlg((LPPRINTDLG)&prd))
    return fTrue;

  // Get the printer DC.
  if (prd.hDC)
    hdc = prd.hDC;
  else {
    // If the dialog didn't just return the DC, try to make one manually.
    if (!prd.hDevNames)
      return fFalse;
    lpDevNames = (LPDEVNAMES)GlobalLock(prd.hDevNames);
    lpszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
    lpszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
    lpszPortName = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
    GlobalUnlock(prd.hDevNames);
    if (prd.hDevMode)
      lpDevMode = (LPDEVMODE)GlobalLock(prd.hDevMode);
    hdc = CreateDC(lpszDriverName, lpszDeviceName, lpszPortName,
      /*(LPSTR)*/lpDevMode);
    if (prd.hDevMode && lpDevMode)
      GlobalUnlock(prd.hDevMode);
  }
  if (prd.hDevNames) {
    GlobalFree(prd.hDevNames);
    prd.hDevNames = (HGLOBAL)NULL;
  }
  if (prd.hDevMode) {
    GlobalFree(prd.hDevMode);
    prd.hDevMode = (HGLOBAL)NULL;
  }

  // Setup the Abort dialog and start the print job.
  lpAbortDlg = (DLGPROC)MakeProcInstance((FARPROC)DlgAbort, wi.hinst);
  lpAbortProc = (ABORTPROC)MakeProcInstance((FARPROC)DlgAbortProc, wi.hinst);
  SetAbortProc(hdc, lpAbortProc);
  doci.cbSize = sizeof(DOCINFO);
  doci.lpszDocName = szAppName;
  doci.lpszOutput = NULL;
  if (StartDoc(hdc, &doci) < 0) {
    FreeProcInstance(lpAbortDlg);
    FreeProcInstance(lpAbortProc);
    DeleteDC(hdc);
    return fFalse;
  }
  wi.fAbort = FALSE;
  wi.hwndAbort = CreateDialog(wi.hinst, MAKEINTRESOURCE(dlgAbort), wi.hwnd,
    lpAbortDlg);
  if (!wi.hwndAbort)
    return fFalse;
  ShowWindow(wi.hwndAbort, SW_NORMAL);
  EnableWindow(wi.hwnd, fFalse);
  StartPage(hdc);

  // Scale the chart to the page.
  if (us.fGraphics) {
    gs.xWin *= METAMUL; gs.yWin *= METAMUL; gs.nScale *= METAMUL;
    fInverse = gs.fInverse;
    if (us.fSmartSave)
      gs.fInverse = fTrue;
  }
  SetMapMode(hdc, MM_ANISOTROPIC);       // So SetViewportExt can be called.
  xPrint = GetDeviceCaps(hdc, HORZRES);
  yPrint = GetDeviceCaps(hdc, VERTRES);
  SetViewportOrg(hdc, 0, 0);
  SetViewportExt(hdc, xPrint, yPrint);
  xClient = wi.xClient; yClient = wi.yClient;
  wi.xClient = gs.xWin;
  wi.yClient = NMultDiv(wi.xClient, yPrint, xPrint);
  if (gs.yWin > wi.yClient) {
    wi.yClient = gs.yWin;
    wi.xClient = NMultDiv(wi.yClient, xPrint, yPrint);
  }
  wi.hdcPrint = hdc;

  FRedraw();    // Actually go "draw" the chart to the printer.

  // Restore globals that were temporarily changed to print above.
  wi.hdcPrint = hdcNil;
  wi.xClient = xClient; wi.yClient = yClient;
  if (us.fGraphics) {
    gs.xWin /= METAMUL; gs.yWin /= METAMUL; gs.nScale /= METAMUL;
    gs.fInverse = fInverse;
  }

  // Finalize and cleanup everything.
  if (!wi.fAbort) {
    EndPage(hdc);
    EndDoc(hdc);
  }
  EnableWindow(wi.hwnd, fTrue);
  DestroyWindow(wi.hwndAbort);
  FreeProcInstance(lpAbortDlg);
  FreeProcInstance(lpAbortProc);
  DeleteDC(hdc);
  return fTrue;
}


// Message loop function for the printing abort dialog. Loops until printing
// is completed or the user hits cancel, returning which result.

flag API DlgAbortProc(HDC hdc, int nCode)
{
  MSG msg;

  if (wi.hwndAbort == (HWND)NULL)
    return fTrue;
  while (!wi.fAbort && PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
    if (!IsDialogMessage(wi.hwndAbort, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  return !wi.fAbort;
}


// Processing function for the printing abort modeless dialog, as brought up
// temporarily when printing via the File / Print menu command.

BOOL API DlgAbort(HWND hdlg, uint message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hdlg, IDCANCEL));
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      wi.fAbort = fTrue;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


#endif // WIN (DlgPrint, DlgAbortProc, DlgAbort)

// Processing function for the file settings dialog, as brought up with the
// File / File Settings menu command.

flag API DlgFile(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, nwx;
  real rI;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxFi_YOO, us.fSmartSave);
    SetCheck(dxFi_kh, us.fTextHTML);
    SetCheck(dxFi_Wb, wi.fBmpWindow);
    SetCheck(dxFi_Xbp, gs.chBmpMode == 'P');
    SetCheck(dxFi_Xp0, !gs.fPSComplete);
    SetCheck(dxFi_Wt, wi.fNoPopup);
    SetCheck(dxFi_Yo, us.fWriteOld);
    SetCheck(dxFi_YXf, gs.nFontAll > 0);
    SetEditN(deFi_Wx, wi.nAntialias);
    SetEditN(deFi_YXx, gs.nThickAdjust);
    SetCheck(dxFi_XI0, !gs.fBackDraw);
    for (i = 25; i <= 100; i += 25) {
      sprintf(sz, "%d", i);
      SetCombo(dcFi_XI1, sz);
    }
    SetEditR(hdlg, dcFi_XI1, gs.rBackPct, -3);
    SetEdit(deFi_Y5i, us.szADB);
    SetEdit(deFi_YXp0_x, SzLength(gs.xInch));
    SetEdit(deFi_YXp0_y, SzLength(gs.yInch));
    SetRadio(gs.nOrient == 0 ? dr03 : (gs.nOrient > 0 ? dr01 : dr02),
      dr01, dr03);
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      nwx = GetEditN(hdlg, deFi_Wx);
      rI = GetEditR(hdlg, dcFi_XI1);
      EnsureN(nwx, FValidAntialias(nwx), "antialias");
      EnsureR(rI, FValidBackPct(rI), "background transparency");
      us.fSmartSave = GetCheck(dxFi_YOO);
      us.fTextHTML = GetCheck(dxFi_kh);
      wi.fBmpWindow = GetCheck(dxFi_Wb);
      if (GetCheck(dxFi_Xbp))
        gs.chBmpMode = 'P';
      else if (gs.chBmpMode == 'P')
        gs.chBmpMode = 'B';
      gs.fPSComplete = !GetCheck(dxFi_Xp0);
      wi.fNoPopup = GetCheck(dxFi_Wt);
      us.fWriteOld = GetCheck(dxFi_Yo);
      gs.nFontAll = GetCheck(dxFi_YXf) * gi.nFontPrev;
      gs.nFontTxt = gs.nFontAll / 0x100000;
      gs.nFontSig = (gs.nFontAll / 0x10000) % 0x10;
      gs.nFontHou = (gs.nFontAll / 0x1000) % 0x10;
      gs.nFontObj = (gs.nFontAll / 0x100) % 0x10;
      gs.nFontAsp = (gs.nFontAll / 0x10) % 0x10;
      gs.nFontNak = gs.nFontAll % 0x10;
      if (nwx <= 0)
        gs.fAntialias = fFalse;
      else
        wi.nAntialias = nwx;
      gs.nThickAdjust = GetEditN(hdlg, deFi_YXx);
      gs.fBackDraw = !GetCheck(dxFi_XI0);
      gs.rBackPct = rI;
      GetEdit(deFi_Y5i, sz); FCloneSz(sz, &us.szADB);
      GetEdit(deFi_YXp0_x, sz); gs.xInch = RParseSz(sz, pmLength);
      GetEdit(deFi_YXp0_y, sz); gs.yInch = RParseSz(sz, pmLength);
      gs.nOrient = GetCheck(dr03) ? 0 : (GetCheck(dr01) ? 1 : -1);
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the chart list dialog, as brought up with the
// Info / Chart List / View Chart List menu command.

flag API DlgList(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  DLGPROC dlgproc;
  char sz[cchSzLine], sz1[cchSzDef], sz2[cchSzDef];
  int i, i2, j, nSav;
  flag fSav;
  CI ciT, *pci;
  HCURSOR hcurPrev = NULL;
  static flag fFilter = fFalse;
#ifdef EXPRESS
  CP cpT;
  flag fRet;
#endif

  switch (message) {
  case WM_INITDIALOG:
    HourglassOn;
    SendDlgItemMessage(hdlg, dlLi, LB_INITSTORAGE, is.cci, is.cci*cchSzDef);
    fFilter = (wParam == (WORD)-2 && lParam == -2);
    if (fFilter) {
      GetEdit(deLi_n, sz1);
      GetEdit(deLi_l, sz2);
    }
    for (i = 0, i2 = 0; i < is.cci; i++) {
      pci = &is.rgci[i];
      if (fFilter) {
        if (*sz1) {
          for (j = 0; pci->nam[j]; j++)
            if (FEqSzSubI(sz1, &pci->nam[j]))
              break;
          if (!pci->nam[j])
            continue;
        }
        if (*sz2) {
          for (j = 0; pci->loc[j]; j++)
            if (FEqSzSubI(sz2, &pci->loc[j]))
              break;
          if (!pci->loc[j])
            continue;
        }
#ifdef EXPRESS
        // May want to skip current chart if AstroExpression says to do so.
        if (!us.fExpOff && FSzSet(us.szExpListF)) {
          cpT = cp0; ciT = ciCore;
          ciCore = *pci;
          CastChart(-1);
          ExpSetN(iLetterZ, i);
          fRet = !NParseExpression(us.szExpListF);
          ciCore = ciT;
          cp0 = cpT;
          if (fRet)
            continue;
        }
#endif
      }
      j = DayOfWeek(pci->mon, pci->day, pci->yea);
      nSav = us.fAnsiChar; us.fAnsiChar = 2;
      fSav = us.fGraphics; us.fGraphics = fTrue;
      sprintf(sz, "%.3s %s %s (%cT Zone %s) %s %s%s%s", szDay[j],
        SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
        ChDst(pci->dst), SzZone(pci->zon), SzLocation(pci->lon, pci->lat),
        pci->nam, FSzSet(pci->nam) && FSzSet(pci->loc) ? "; " : "",
        pci->loc);
      us.fAnsiChar = nSav; us.fGraphics = fSav;
      SetListN(dlLi, sz, i, j);
      i2++;
    }
    if (i2 <= 0) {
      SetListN(dlLi, "(No charts in list)", -1, j);
    }
    sprintf(sz, "List size: %d", i2);
    SetDlgItemText(hdlg, ds01, sz);
    HourglassOff;
    if (fFilter || (wParam == (WORD)-1 && lParam == -1))
      return fFalse;
    SetRadio(dr01, dr01, dr05);
    SetRadio(dr06, dr06, dr11);
    SetFocus(GetDlgItem(hdlg, dbLi_sl));
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbLi_sl) {
      HourglassOn;
      i = GetRadio(hdlg, dr01, dr05);
      FSortCIList(i);
      HourglassOff;
    }
    if (wParam == dbLi_da)
      is.cci = 0;
    if (wParam == dbLi_da || wParam == dbLi_sl ||
      wParam == dbLi_f || wParam == dbLi_fr) {
      ClearList(dlLi);
      i = wParam != dbLi_f ? -1 : -2;
      DlgList(hdlg, WM_INITDIALOG, i, i);
    }
    if (wParam == IDOK || wParam == dbLi_st || wParam == dbLi_cf ||
      wParam == dbLi_ec || wParam == dbLi_dc) {
      j = (int)SendDlgItemMessage(hdlg, dlLi, LB_GETCURSEL, 0, 0);
      i = (int)SendDlgItemMessage(hdlg, dlLi, LB_GETITEMDATA, j, 0);
      if (i < 0 && wParam != IDOK && wParam != dbLi_cf) {
        PrintWarning(
          "Can't do operation because no chart in list is selected.");
        return fTrue;
      }
      if (wParam == dbLi_ec) {
        wi.nDlgChart = -i;
        WiDoDialog(DlgInfo, dlgInfo);
        if (wi.nDlgChart == -i)
          return fTrue;
      } else if (wParam == dbLi_dc) {
        CopyRgb((pbyte)&is.rgci[i+1], (pbyte)&is.rgci[i],
          (is.cci-1-i)*sizeof(CI));
        is.cci--;
      } else if (wParam == dbLi_cf) {
        i = GetRadio(hdlg, dr06, dr11) + 1;
        FAppendCIList(rgpci[i]);
        j = is.cci-1;
      } else if (i >= 0) {
        ciT = is.rgci[i];
        is.iciCur = i;
        wi.fCast = fTrue;
        i = GetRadio(hdlg, dr06, dr11) + 1;
        *rgpci[i] = ciT;
        if (i == 1)
          ciCore = ciT;
        if (wParam != IDOK)
          return fTrue;
      }
      if (wParam != IDOK) {
        ClearList(dlLi);
        DlgList(hdlg, WM_INITDIALOG, -1, -1);
        if (j >= is.cci)
          j = is.cci-1;
        SendDlgItemMessage(hdlg, dlLi, LB_SETCURSEL, j, 0);
      } else if (fFilter && i < 0) {
        // Only permanently filter on OK if no chart is selected.
        GetEdit(deLi_n, sz1);
        GetEdit(deLi_l, sz2);
        FilterCIList(sz1, sz2);
      }
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      fFilter = fFalse;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the command switch entry dialog, as brought up with
// the Edit / Enter Command Line menu command.

flag API DlgCommand(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzLine];

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxCo_e, !us.fExpOff);
    SetFocus(GetDlgItem(hdlg, deCo));
#ifndef EXPRESS
    ShowWindow(GetDlgItem(hdlg, dxCo_e), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      us.fExpOff = !GetCheck(dxCo_e);
      GetDlgItemText(hdlg, deCo, sz, cchSzLine);  // Longer cchSzLine string.
      FProcessCommandLine(sz);
      wi.fCast = wi.fMenuAll = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the color customization dialog, as brought up with
// the View / Set Colors menu command.

flag API DlgColor(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k, l;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cColor; i++) {
      j = ikPalette[i];
      SetEditColor(hdlg, dck00 + i, j <= 0 ? kMainA[-j] : kRainbowA[j], 0);
    }
    for (i = 0; i < cElem; i++)
      SetEditColor(hdlg, dce0 + i, kElemA[i], 0);
    for (i = 1; i <= cRay; i++)
      SetEditColor(hdlg, dcr1 - 1 + i, kRayA[i], 0);
    SetEditColor(hdlg, dca01, gi.kiPen, 0);
    SetEditColor(hdlg, dca02, gs.kiDeca, 4);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (k = 0; k <= 1; k++) {
        for (i = 0; i < cColor; i++) {
          GetEdit(dck00 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k) {
            j = ikPalette[i];
            if (j <= 0)
              kMainA[-j] = l;
            else
              kRainbowA[j] = l;
          } else
            EnsureN(l, FValidColorA(l), "palette color");
        }
        for (i = 0; i < cElem; i++) {
          GetEdit(dce0 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kElemA[i] = l;
          else
            EnsureN(l, FValidColorA(l), "element color");
        }
        for (i = 1; i <= cRay; i++) {
          GetEdit(dcr1 - 1 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kRayA[i] = l;
          else
            EnsureN(l, FValidColorA(l), "ray color");
        }
        for (i = 0; i <= 1; i++) {
          GetEdit(dca01 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k) {
            if (i <= 0)
              gi.kiPen = l;
            else
              gs.kiDeca = l;
          } else
            EnsureN(l, FValidColorA(l) || (i == 1 && l == kMax), "color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the chart info entry dialog, as brought up by both
// the Info / Set Chart Info and Info / Set Chart #2 menu commands.

flag API DlgInfo(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    if (wi.nDlgChart >= 1)
      ci = *rgpci[wi.nDlgChart];
    else
      ci = is.rgci[-wi.nDlgChart];
    if (wi.nDlgChart != 1) {
      if (wi.nDlgChart > 1)
        sprintf(sz, "Set Chart #%d Info", wi.nDlgChart);
      else
        sprintf(sz, "Set Chart List #%d Info", 1-wi.nDlgChart);
      SetWindowText(hdlg, sz);
    }
    SetEditSz(hdlg, deInNam, ci.nam);
    SetEditSz(hdlg, deInLoc, ci.loc);
LInit:
    SetEditMDYT(hdlg, dcInMon, dcInDay, dcInYea, dcInTim,
      ci.mon, ci.day, ci.yea, ci.tim);
    SetEditSZOA(hdlg, dcInDst, dcInZon, dcInLon, dcInLat,
      DstReal(ci.dst), ci.zon, ci.lon, ci.lat);
    SetFocus(GetDlgItem(hdlg, dcInMon));
#ifndef ATLAS
    ShowWindow(GetDlgItem(hdlg, dbInCity), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInCoor), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInChan), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInAppl), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dlIn), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbInNow || wParam == dbInSet) {
#ifdef TIME
      if (wParam == dbInNow) {
        GetTimeNow(&ci.mon, &ci.day, &ci.yea, &ci.tim, ciDefa.dst, ciDefa.zon);
        ci.dst = ciDefa.dst; ci.zon = ciDefa.zon;
        ci.lon = ciDefa.lon; ci.lat = ciDefa.lat;
      } else
#endif
        ci = ciSave;
      goto LInit;
    }
#ifdef ATLAS
    if (FDlgInfoAtlas(hdlg, wParam, fFalse))
      break;
#endif
    if (wParam == IDOK) {
      GetEdit(dcInMon, sz); ci.mon = NParseSz(sz, pmMon);
      GetEdit(dcInDay, sz); ci.day = NParseSz(sz, pmDay);
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
      GetEdit(dcInTim, sz); ci.tim = RParseSz(sz, pmTim);
      GetEdit(dcInDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcInZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcInLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcInLat, sz); ci.lat = RParseSz(sz, pmLat);
      EnsureN(ci.mon, FValidMon(ci.mon), "month");
      EnsureN(ci.yea, FValidYea(ci.yea), "year");
      EnsureN(ci.day, FValidDay(ci.day, ci.mon, ci.yea), "day");
      EnsureR(ci.tim, FValidTim(ci.tim), "time");
      EnsureR(ci.dst, FValidDst(ci.dst), "daylight saving");
      EnsureR(ci.zon, FValidZon(ci.zon), "time zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      GetEdit(deInNam, sz); ci.nam = SzClone(sz);
      GetEdit(deInLoc, sz); ci.loc = SzClone(sz);
      if (wi.nDlgChart >= 1) {
        *rgpci[wi.nDlgChart] = ci;
        if (wi.nDlgChart == 1)
          ciCore = ci;
      } else {
        is.rgci[-wi.nDlgChart] = ci;
        wi.nDlgChart = 1;
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the default chart info dialog, as brought up with
// the Info / Default Chart Info menu command.

flag API DlgDefault(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    SetEditSZOA(hdlg, dcDeDst, dcDeZon, dcDeLon, dcDeLat,
      ciDefa.dst, ciDefa.zon, ciDefa.lon, ciDefa.lat);
    SetCombo(dcDeDst, "Autodetect");
    SetEditN(dcDeCor, (int)us.lTimeAddition);
    SetCombo(dcDeCor, "60"); SetCombo(dcDeCor, "0"); SetCombo(dcDeCor, "-60");
    SetEditSz(hdlg, dcDeElv, SzElevation(us.elvDef));
    SetCombo(dcDeElv, "0m"); SetCombo(dcDeElv, "1000ft");
    SetEditSz(hdlg, dcDeTmp, SzTemperature(us.tmpDef));
    SetCombo(dcDeTmp, "0C"); SetCombo(dcDeTmp, "32F");
    SetEditSz(hdlg, deDeNam, ciDefa.nam);
    SetEditSz(hdlg, deDeLoc, ciDefa.loc);
    SetFocus(GetDlgItem(hdlg, dcDeDst));
#ifndef ATLAS
    ShowWindow(GetDlgItem(hdlg, dbInCity), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInCoor), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInChan), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInAppl), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dlIn), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      ci = ciDefa;
      GetEdit(dcDeDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcDeZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcDeLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcDeLat, sz); ci.lat = RParseSz(sz, pmLat);
      GetEdit(dcDeCor, sz); us.lTimeAddition = NFromSz(sz);
      GetEdit(dcDeElv, sz); us.elvDef = RParseSz(sz, pmElv);
      GetEdit(dcDeTmp, sz); us.tmpDef = RParseSz(sz, pmTmp);
      EnsureR(ci.dst, FValidDst(ci.dst), "daylight saving");
      EnsureR(ci.zon, FValidZon(ci.zon), "time zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      ciDefa = ci;
      GetEdit(deDeNam, sz); ciDefa.nam = SzClone(sz);
      GetEdit(deDeLoc, sz); ciDefa.loc = SzClone(sz);
      wi.fCast = fTrue;
    }
#ifdef ATLAS
    if (FDlgInfoAtlas(hdlg, wParam, fTrue))
      break;
#endif
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the charts #3 through #6 info entry dialog, as
// brought up by the Info / Charts #3 through #6 menu command.

flag API DlgInfoAll(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  DLGPROC dlgproc;
  char sz[cchSzMax];
  int i, n, nSav;
  flag fSav;
  CI *pci;

  switch (message) {
  case WM_INITDIALOG:
    nSav = us.fAnsiChar; us.fAnsiChar = 2;
    fSav = us.fGraphics; us.fGraphics = fTrue;
    for (i = 1; i <= cRing; i++) {
      pci = rgpci[i];
      n = DayOfWeek(pci->mon, pci->day, pci->yea);
      sprintf(sz, "%.3s %s %s (%cT Zone %s) %s", szDay[n],
        SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
        ChDst(pci->dst), SzZone(pci->zon), SzLocation(pci->lon, pci->lat));
      SetDlgItemText(hdlg, ds01 + (i-1)*2, sz);
      sprintf(sz, "%s%s%s", pci->nam, FSzSet(pci->nam) && FSzSet(pci->loc) ?
        "; " : "", pci->loc);
      SetDlgItemText(hdlg, ds02 + (i-1)*2, sz);
    }
    us.fAnsiChar = nSav; us.fGraphics = fSav;
    if (wParam == (WORD)-1 && lParam == -1)
      return fFalse;
    i = us.nRel;
    if (i > rcDual)
      i = 0;
    else if (i < rcHexaWheel)
      i = rcDual;
    SetRadio(dr01-i, dr01, dr06);
    for (i = 2; i <= 5; i++)
      SetCheck(dx02 - 2 + i, rgfProg[i]);
    return fTrue;

  case WM_COMMAND:
    if (FBetween(wParam, dbIa_o1, dbIa_o6)) {
      wi.nDlgChart = wParam - dbIa_o1 + 1;
      DlgOpenChart();
      DlgInfoAll(hdlg, WM_INITDIALOG, -1, -1);
    } else if (FBetween(wParam, dbIa_i1, dbIa_i6)) {
      wi.nDlgChart = wParam - dbIa_i1 + 1;
      WiDoDialog(DlgInfo, dlgInfo);
      DlgInfoAll(hdlg, WM_INITDIALOG, -1, -1);
    }
    if (wParam == IDOK) {
      i = GetRadio(hdlg, dr01, dr06) + 1;
      SetRel(-(i-1));
      for (i = 2; i <= 5; i++)
        rgfProg[i] = GetCheck(dx02 - 2 + i);
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the aspect settings dialog, as brought up with the
// Setting / Aspect Settings menu command.

flag API DlgAspect(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 1; i <= cAspect; i++) {
      SetCheck(dxa01 - 1 + i, ignorea[i]);
      SetEditR(hdlg, deo01 - 1 + i, rAspOrb[i], -6);
      SetEditR(hdlg, dea01 - 1 + i, rAspAngle[i], -6);
      SetEditR(hdlg, dei01 - 1 + i, rAspInf[i], 2);
      SetEditColor(hdlg, dck01 - 1 + i, kAspA[i], 0);
    }
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbAs_RA0:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fTrue);
      break;
    case dbAs_RA1:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fFalse);
      break;
    case dbAs_RA:
      for (i = 1; i <= 5; i++)
        SetCheck(dxa01 - 1 + i, !GetCheck(dxa01 - 1 + i));
      break;
    }

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 1; i <= cAspect; i++) {
          r = GetEditR(hdlg, deo01 - 1 + i);
          if (j)
            rAspOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb");
          r = GetEditR(hdlg, dea01 - 1 + i);
          if (j)
            rAspAngle[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "angle");
          if (j) {
            r = GetEditR(hdlg, dei01 - 1 + i);
            rAspInf[i] = r;
          }
          GetEdit(dck01 - 1 + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kAspA[i] = k;
          else
            EnsureN(k, FValidColorA(k), "color");
        }
      }
      for (i = 1; i <= cAspect; i++)
        ignorea[i] = GetCheck(dxa01 - 1 + i);
      AdjustAspectCount();
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the object settings dialog, as brought up with the
// Setting / Object Settings menu command.

flag API DlgObject(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i <= oCore; i++) {
      SetEditR(hdlg, deo01 + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 + i, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 + i, kObjU[i], 1);
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 0; i <= oCore; i++) {
          r = GetEditR(hdlg, deo01 + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
          r = GetEditR(hdlg, dea01 + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
          r = GetEditR(hdlg, dei01 + i);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, FValidColor2A(k), "color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the cusp and Uranian object settings dialog, as
// brought up with the Setting / More Object Settings menu command.

flag API DlgObject2(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i0, i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i0 = oAsc; i0 <= dwarfHi+1; i0++) {
      i = (i0 <= dwarfHi ? i0 : starLo);
      SetEditR(hdlg, deo01 - oAsc + i0, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 - oAsc + i0, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 - oAsc + i0, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 - oAsc + i0, kObjU[i], 1 + (i == starLo));
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i0 = oAsc; i0 <= dwarfHi+1; i0++) {
          i = (i0 <= dwarfHi ? i0 : starLo);
          r = GetEditR(hdlg, deo01 - oAsc + i0);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, FBetween(r, -rDegMax, rDegMax), "max orb");
          r = GetEditR(hdlg, dea01 - oAsc + i0);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, FBetween(r, -rDegMax, rDegMax), "orb addition");
          r = GetEditR(hdlg, dei01 - oAsc + i0);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 - oAsc + i0, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, i < starLo ? FValidColor2A(k) : FValidColorSA(k),
              "color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the planetary moons object settings dialog, as
// brought up with the Setting / Planetary Moons / Moon Object Settings menu
// command.

flag API DlgObjectM(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = moonsLo; i <= cobHi; i++) {
      SetEditR(hdlg, deo01 - moonsLo + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 - moonsLo + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 - moonsLo + i, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 - moonsLo + i, kObjU[i], 3);
    }
    SetCheck(dxMo_Ym, us.fMoonMove);
    SetCheck(dxMo_80, us.fMoonChartSep);
    SetCheck(dxMo_X8, gs.fMoonWheel);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = moonsLo; i <= cobHi; i++) {
          r = GetEditR(hdlg, deo01 - moonsLo + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
          r = GetEditR(hdlg, dea01 - moonsLo + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
          r = GetEditR(hdlg, dei01 - moonsLo + i);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 - moonsLo + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, FValidColorMA(k), "color");
        }
      }
      us.fMoonMove = GetCheck(dxMo_Ym);
      us.fMoonChartSep = GetCheck(dxMo_80);
      gs.fMoonWheel = GetCheck(dxMo_X8);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the custom objects dialog, as brought up with the
// Setting / Planetary Moons / Object Customization menu command.

flag API DlgCustom(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j;
#ifdef SWISS
  char *pch, *pchEnd;
  int k, l;
#ifdef JPLWEB
  real rT;
#endif
#endif // SWISS

  switch (message) {
  case WM_INITDIALOG:
    for (i = custLo; i <= custHi; i++) {
      SetEdit(den01 - custLo + i, szObjDisp[i]);
#ifdef SWISS
      j = rgTypSwiss[i - custLo];
      k = rgObjSwiss[i - custLo];
      if (j != 2)
        sprintf(sz, "%s%d", j <= 0 ? "h" :
          (j == 1 ? "" : (j == 3 ? "m" : (j == 4 ? "j" : "A"))), k);
      else
        sprintf(sz, k < cobLo ? "%.3s" : "%.4s", szObjName[k]);
      for (pch = sz; *pch; pch++)
        ;
      j = rgPntSwiss[i - custLo];
      k = rgFlgSwiss[i - custLo];
      if (j > 0 || k > 0)
        *pch++ = ' ';
      if (j > 0)
        *pch++ = (j == 1 ? 'n' : (j == 2 ? 's' : (j == 3 ? 'p' : 'a')));
      if (k &  1) *pch++ = 'H';
      if (k &  2) *pch++ = 'S';
      if (k &  4) *pch++ = 'B';
      if (k &  8) *pch++ = 'N';
      if (k & 16) *pch++ = 'T';
      if (k & 32) *pch++ = 'V';
      *pch = chNull;
      SetEdit(ded01 - custLo + i, sz);
#endif
    }
#ifndef SWISS
    ShowWindow(GetDlgItem(hdlg, dbCu_l), SW_HIDE);
    for (i = custLo; i <= custHi; i++)
      ShowWindow(GetDlgItem(hdlg, ded01 - custLo + i), SW_HIDE);
#endif
    return fTrue;

  case WM_COMMAND:
#ifdef SWISS
    if (wParam == dbCu_l)
      for (i = custLo; i <= custHi; i++) {
        GetEdit(den01 - custLo + i, sz);
        if (*sz && !FEqSz(sz, szObjUnknown))
          continue;
        GetEdit(ded01 - custLo + i, sz);
        for (pch = sz; *pch; pch++)
          ;
        pchEnd = pch;
        for (pch--; pch > sz && *pch >= 'A'; pch--)
          ;
        if (pch >= sz && *pch < '0')
          *pch = chNull;
        pch = sz;
        k = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
          (*pch == 'A' ? 5 : (FNumCh(*pch) ? 1 : 2)))));
        if (k == 0 || k >= 3)
          pch++;
        l = (k == 2 ? NParseSz(pch, pmObject) : NFromSz(pch));
        switch (k) {
        case 0:
          SwissGetObjName(sz, -l);
          break;
        case 1:
          SwissGetObjName(sz, l);
          break;
        case 2:
          sprintf(sz, "%s", FValidObj(l) ? szObjName[l] : szObjUnknown);
          break;
        case 3:
          l = ObjMoons(l);
          sprintf(sz, "%s", FItem(l) ? szObjName[l] : szObjUnknown);
          break;
        case 4:
#ifdef JPLWEB
          if (!GetJPLHorizons(l, &rT, &rT, &rT, &rT, &rT, &rT, sz))
#endif
            sprintf(sz, "%s", szObjUnknown);
          break;
        case 5:
          sprintf(sz, "%s", FValidPart(l) ? ai[l-1].name : szObjUnknown);
          break;
        }
        SetEdit(den01 - custLo + i, sz);
      }
#endif

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = custLo; i <= custHi; i++) {
#ifdef SWISS
          GetEdit(ded01 - custLo + i, sz);
          for (pch = sz; *pch; pch++)
            ;
          pchEnd = pch;
          for (pch--; pch > sz && *pch >= 'A'; pch--)
            ;
          if (pch >= sz && *pch < '0')
            *pch = chNull;
          pch = sz;
          k = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
            (*pch == 'A' ? 5 : (FNumCh(*pch) ? 1 : 2)))));
          if (k == 0 || k >= 3)
            pch++;
          l = (k == 2 ? NParseSz(pch, pmObject) : NFromSz(pch));
          EnsureN(l, FValidCustom(l, k), "definition");
#endif
          if (j) {
#ifdef SWISS
            rgTypSwiss[i - custLo] = k;
            rgObjSwiss[i - custLo] = l;
            k = l = 0;
            for (pch = pchEnd-1; pch > sz && *pch >= 'A'; pch--)
              ;
            if (pch > sz)
              for (pch = pchEnd-1; pch > sz && *pch >= 'A'; pch--) {
                if (*pch == 'n') k = 1;
                if (*pch == 's') k = 2;
                if (*pch == 'p') k = 3;
                if (*pch == 'a') k = 4;
                if (*pch == 'H') l |= 1;
                if (*pch == 'S') l |= 2;
                if (*pch == 'B') l |= 4;
                if (*pch == 'N') l |= 8;
                if (*pch == 'T') l |= 16;
                if (*pch == 'V') l |= 32;
              }
            rgPntSwiss[i - custLo] = k;
            rgFlgSwiss[i - custLo] = l;
#endif
            GetEdit(den01 - custLo + i, sz);
            if (!FEqSz(sz, szObjDisp[i]))
              FCloneSzCore(sz, (char **)&szObjDisp[i],
                szObjDisp[i] == szObjName[i]);
          }
        }
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the custom stars dialog, as brought up with the
// Setting / Planetary Moons / Star Customization menu command.

flag API DlgCustomS(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j;
#ifdef SWISS
  int k;
#endif

  switch (message) {
  case WM_INITDIALOG:
    for (i = starLo; i <= starHi; i++) {
      SetEdit(den01 - starLo + i, szObjDisp[i]);
#ifdef SWISS
      k = i - starLo + 1;
      if (FSzSet(szStarCustom[k]))
        SetEdit(ded01 - starLo + i, szStarCustom[k]);
      else if (*szStarNameSwiss[k])
        SetEdit(ded01 - starLo + i, szStarNameSwiss[k]);
      else
        SetEdit(ded01 - starLo + i, szObjName[i]);
#endif
    }
#ifndef SWISS
    ShowWindow(GetDlgItem(hdlg, dbCu_l), SW_HIDE);
    for (i = starLo; i <= starHi; i++)
      ShowWindow(GetDlgItem(hdlg, ded01 - starLo + i), SW_HIDE);
#endif
    return fTrue;

  case WM_COMMAND:
#ifdef SWISS
    if (wParam == dbCu_l)
      for (i = starLo; i <= starHi; i++) {
        GetEdit(den01 - starLo + i, sz);
        if (*sz && !FEqSz(sz, szObjUnknown))
          continue;
        GetEdit(ded01 - starLo + i, sz);
        if (!SwissTestStar(sz))
          sprintf(sz, "%s", szObjUnknown);
        SetEdit(den01 - starLo + i, sz);
      }
#endif

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = starLo; i <= starHi; i++) {
          if (j) {
            GetEdit(den01 - starLo + i, sz);
            if (!FEqSz(sz, szObjDisp[i]))
              FCloneSzCore(sz, (char **)&szObjDisp[i],
                szObjDisp[i] == szObjName[i]);
#ifdef SWISS
            GetEdit(ded01 - starLo + i, sz);
            k = i - starLo + 1;
            FCloneSz(FEqSz(sz, *szStarNameSwiss[k] ? szStarNameSwiss[k] :
              szObjName[i]) ? NULL : sz, &szStarCustom[k]);
#endif
          }
        }
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the object restrictions dialog, as invoked with
// both the Setting / Restrictions and Transit Restrictions menu commands.

flag API DlgRestrict(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  byte *pb;
  int i;

  switch (message) {
  case WM_INITDIALOG:
    if (wi.wCmd == cmdRes)
      pb = ignore;
    else {
      SetWindowText(hdlg, "Transit Object Restrictions");
      SetDlgItemText(hdlg, dbRT, "Copy &From Standard Restriction Set");
      pb = ignore2;
    }
    for (i = 0; i <= dwarfHi; i++)
      SetCheck(dx01 + i, pb[i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbRe_R0:
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbRe_R1:
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    case dbRe_R:
      for (i = oMain+1; i <= oCore; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_RC:
      for (i = cuspLo; i <= cuspHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_Ru:
      for (i = uranLo; i <= uranHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_Ry:
      for (i = dwarfLo; i <= dwarfHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRT:
      pb = (wi.wCmd == cmdRes ? ignore2 : ignore);
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, pb[i]);
      break;
    case dbRe_YRi:
      pb = (wi.wCmd == cmdRes ? ignoreMem : ignore2Mem);
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, pb[i]);
      break;
    }

    if (wParam == IDOK) {
      pb = (wi.wCmd == cmdRes ? ignore : ignore2);
      for (i = 0; i <= dwarfHi; i++)
        pb[i] = GetCheck(dx01 + i);
      if (!us.fCusp) {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fCusp = fTrue;
            WiCheckMenu(cmdResCusp, fTrue);
            break;
          }
      } else {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > cuspHi) {
          us.fCusp = fFalse;
          WiCheckMenu(cmdResCusp, fFalse);
        }
      }
      if (!us.fUranian) {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fUranian = fTrue;
            WiCheckMenu(cmdResUranian, fTrue);
            break;
          }
      } else {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > uranHi) {
          us.fUranian = fFalse;
          WiCheckMenu(cmdResUranian, fFalse);
        }
      }
      if (!us.fDwarf) {
        for (i = dwarfLo; i <= dwarfHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fDwarf = fTrue;
            WiCheckMenu(cmdResDwarf, fTrue);
            break;
          }
      } else {
        for (i = dwarfLo; i <= dwarfHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > dwarfHi) {
          us.fDwarf = fFalse;
          WiCheckMenu(cmdResDwarf, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the star restrictions dialog, as brought up with
// the Setting / Star Restrictions menu command.

flag API DlgStar(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  int i;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cStar; i++)
      SetCheck(dx01 + i, ignore[starLo + i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbSt_RU0:
      for (i = 0; i < cStar; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbSt_RU1:
      for (i = 0; i < cStar; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    }

    if (wParam == IDOK) {
      for (i = 0; i < cStar; i++)
        ignore[starLo + i] = GetCheck(dx01 + i);
      if (!us.fStar) {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i]) {
            us.fStar = fTrue;
            WiCheckMenu(cmdResStar, fTrue);
            break;
          }
      } else {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i])
            break;
        if (i > starHi) {
          us.fStar = fFalse;
          WiCheckMenu(cmdResStar, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the planetary moons restrictions dialog, as brought
// up with the Setting / Planetary Moons / Moon Restrictions menu command.

flag API DlgMoons(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  int i;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cMoons2; i++)
      SetCheck(dx01 + i, ignore[moonsLo + i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbMo_Rm0:
      for (i = 0; i < cMoons2; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbMo_Rm1:
      for (i = 0; i < cMoons2; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    case dbMo_Mar:
      for (i = 0; i <= 1; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Jup:
      for (i = 2; i <= 5; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Sat:
      for (i = 6; i <= 13; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Ura:
      for (i = 14; i <= 18; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Nep:
      for (i = 19; i <= 21; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Plu:
      for (i = 22; i <= 26; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_COB:
      for (i = 27; i <= 31; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    }

    if (wParam == IDOK) {
      for (i = 0; i < cMoons2; i++)
        ignore[moonsLo + i] = GetCheck(dx01 + i);
      if (!us.fMoons) {
        for (i = moonsLo; i <= moonsHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fMoons = fTrue;
            WiCheckMenu(cmdResMoons, fTrue);
            break;
          }
      } else {
        for (i = moonsLo; i <= moonsHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > moonsHi) {
          us.fMoons = fFalse;
          WiCheckMenu(cmdResMoons, fFalse);
        }
      }
      if (!us.fCOB) {
        for (i = cobLo; i <= cobHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fCOB = fTrue;
            WiCheckMenu(cmdResCOB, fTrue);
            break;
          }
      } else {
        for (i = cobLo; i <= cobHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > cobHi) {
          us.fCOB = fFalse;
          WiCheckMenu(cmdResCOB, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the standard calculation settings dialog, as
// brought up with the Setting / Calculation Settings menu command.

flag API DlgCalc(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], *pch;
  real rs, rx;
  int nc, nh, n4, n1, i;

  switch (message) {
  case WM_INITDIALOG:
#ifdef SWISS
    SetCombo(dcSe_b, szEphem[cmSwiss]);
    SetCombo(dcSe_b, szEphem[cmMoshier]);
    SetCombo(dcSe_b, szEphem[cmJPL]);
#endif
#ifdef JPLWEB
    if (!us.fNoNetwork)
      SetCombo(dcSe_b, szEphem[cmJPLWeb]);
#endif
#ifdef PLACALC
    if (!us.fNoPlacalc)
      SetCombo(dcSe_b, szEphem[cmPlacalc]);
#endif
#ifdef MATRIX
    if (!us.fNoPlacalc)
      SetCombo(dcSe_b, szEphem[cmMatrix]);
#endif
    SetCombo(dcSe_b, szEphem[cmNone]);
    i = FCmSwissEph() ? cmSwiss : (FCmSwissMosh() ? cmMoshier :
      (FCmSwissJPL() ? cmJPL : (FCmPlacalc() ? cmPlacalc :
      (FCmMatrix() ? cmMatrix : (FCmJPLWeb() ? cmJPLWeb : cmNone)))));
    SetEdit(dcSe_b, szEphem[i]);
    SetEditR(hdlg, dcSe_s, us.rZodiacOffset, 6);
    for (i = 0; *rgZodiacOffset[i].sz; i++) {
      FormatR(sz, rgZodiacOffset[i].r, 6);
      for (pch = sz; *pch; pch++)
        ;
      sprintf(pch, " %s", rgZodiacOffset[i].sz);
      SetCombo(dcSe_s, sz);
      if (us.rZodiacOffset == rgZodiacOffset[i].r)
        SetEdit(dcSe_s, sz);
    }
    SetEdit(dcSe_c, szSystem[us.nHouseSystem]);
    for (i = 0; i < cSystem; i++)
      SetCombo(dcSe_c, szSystem[i]);
    SetEditR(hdlg, deSe_x, us.rHarmonic, -6);
    SetEditN(deSe_4, us.nDwad);
    SetCheck(dxSe_Yh, us.fBarycenter);
    SetCheck(dxSe_Yn, us.fTrueNode);
    SetCheck(dxSe_Yc0, us.fHouseAngle);
    SetCheck(dxSe_Yf, us.fRefract);
    SetCheck(dxSe_ys, us.fSidereal2);
    SetCheck(dxSe_Yn0, us.fNoNutation);
    SetEdit(deSe_h, szObjName[us.objCenter]);
    SetRadio(us.objOnAsc == 0 ? dr01 : (us.objOnAsc > 0 ? dr02 : dr03),
      dr01, dr03);
    SetEdit(deSe_1, szObjName[us.objOnAsc == 0 ? oSun : NAbs(us.objOnAsc)-1]);
    SetCheck(dxSe_10, us.fSolarWhole);
    SetCheck(dxSe_sr0, us.fEquator2);
    SetCheck(dxSe_sr, us.fEquator);
    SetCheck(dxSe_yt, us.fTruePos);
    SetCheck(dxSe_yv, us.fTopoPos);
    SetCheck(dxSe_A3, us.fAspect3D);
    SetCheck(dxSe_Ap, us.fAspectLat);
    SetCheck(dxSe_c3, us.fHouse3D);
    SetRadio(dr03 + us.nHouse3D, dr04, dr06);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      rs = GetEditR(hdlg, dcSe_s);
      GetEdit(dcSe_c, sz); nc = NParseSz(sz, pmSystem);
      GetEdit(deSe_h, sz); nh = NParseSz(sz, pmObject);
      GetEdit(deSe_x, sz);
      i = ChCap(sz[0]) == 'D';
      rx = RFromSz(sz + i);
      if (i != 0 && rx != 0.0)
        rx = rDegMax / rx;
      n4 = GetEditN(hdlg, deSe_4);
      GetEdit(deSe_1, sz); n1 = NParseSz(sz, pmObject);
      EnsureR(rs, FValidOffset(rs), "zodiac offset");
      EnsureN(nc, FValidSystem(nc), "house system");
      EnsureN(nh, FValidCenter(nh), "central planet");
      EnsureR(rx, FValidHarmonic(rx), "harmonic factor");
      EnsureN(n4, FValidDwad(n4), "dwad nesting");
      EnsureN(n1, FItem(n1), "Solar chart planet");
      GetEdit(dcSe_b, sz);
      us.fEphemFiles = us.fPlacalcPla = us.fMatrixPla = fFalse;
      us.nSwissEph = 0;
#ifdef SWISS
      if (FMatchSz(sz, szEphem[cmSwiss])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 0;
      } else if (FMatchSz(sz, szEphem[cmMoshier])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 1;
      } else if (FMatchSz(sz, szEphem[cmJPL])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 2;
      } else if (FMatchSz(sz, szEphem[cmJPLWeb])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 3;
      }
#endif
#ifdef PLACALC
      if (FMatchSz(sz, szEphem[cmPlacalc]))
        us.fEphemFiles = us.fPlacalcPla = fTrue;
#endif
#ifdef MATRIX
      if (FMatchSz(sz, szEphem[cmMatrix]))
        us.fMatrixPla = fTrue;
#endif
      us.rZodiacOffset = rs;
      us.nHouseSystem = nc;
      SetCentric(nh);
      WiCheckMenu(cmdHeliocentric, us.objCenter != oEar);
      us.rHarmonic = rx;
      us.nDwad = n4;
      WiCheckMenu(cmdHouseSetDwad, us.nDwad > 0);
      us.fBarycenter = GetCheck(dxSe_Yh);
      us.fTrueNode = GetCheck(dxSe_Yn);
      us.fHouseAngle = GetCheck(dxSe_Yc0);
      us.fRefract = GetCheck(dxSe_Yf);
      us.fSidereal2 = GetCheck(dxSe_ys);
      us.fNoNutation = GetCheck(dxSe_Yn0);
      us.fAspect3D = GetCheck(dxSe_A3);
      us.fAspectLat = GetCheck(dxSe_Ap);
      us.objOnAsc = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? n1+1 : -n1-1);
      WiCheckMenu(cmdHouseSetSolar, us.objOnAsc);
      us.fSolarWhole = GetCheck(dxSe_10);
      us.fEquator2 = GetCheck(dxSe_sr0);
      us.fEquator = GetCheck(dxSe_sr);
      us.fTruePos = GetCheck(dxSe_yt);
      us.fTopoPos = GetCheck(dxSe_yv);
      us.fHouse3D = GetCheck(dxSe_c3);
      us.nHouse3D = GetRadio(hdlg, dr04, dr06) + 1;
      WiCheckMenu(cmdHouseSet3D, us.fHouse3D);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the display settings dialog, as brought up with the
// Setting / Display Settings menu command.

flag API DlgDisplay(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int na, ni, nro, i;
  real ryw;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxDi_Yd, us.fEuroDate);
    SetCheck(dxDi_Yt, us.fEuroTime);
    SetCheck(dxDi_Yz1, us.fOffsetOnly);
    SetCheck(dxDi_Yv, us.fEuroDist);
    SetCheck(dxDi_Yr, us.fRound);
    SetCheck(dxDi_b0, us.fSeconds);
    SetCheck(dxDi_b1, us.fSecond1K);
    SetCheck(dxDi_b2, us.fSecondHide);
    SetEditN(deDi_A, us.nAsp);
    SetCheck(dxDi_YC, us.fSmartCusp);
    if (us.objRequire >= 0)
      SetEdit(deDi_RO, szObjName[us.objRequire]);
    else
      SetEdit(deDi_RO, "None");
    SetCheck(dxDi_AP, us.fParallel2);
    SetCheck(dxDi_gd, us.fDistance);
    SetEditN(deDi_I, us.nScreenWidth);
    SetCheck(dxDi_Y8, us.fClip80);
    SetCheck(dxDi_I0, us.fSabian);
    SetEditR(hdlg, deDi_Yw, us.rStation, -6);
    SetCheck(dxDi_Yu, us.fEclipse);
    SetCheck(dxDi_Yu0, !us.fEclipseAny);
    for (i = 0; i < arMax; i++)
      SetCheck(dxDi_z0 + i, ignorez[i]);
    SetRadio(dr04 + us.nDegForm, dr04, dr07);
    SetRadio(dr08 + us.nCharset, dr08, dr11);
    for (i = 0; i < rrMax; i++)
      SetCheck(dxDi_r0 + i, ignore7[i]);
    SetRadio(dr12 + us.nAppSep, dr12, dr14);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      GetEdit(deDi_A, sz); na = NParseSz(sz, pmAspect);
      GetEdit(deDi_RO, sz); nro = NParseSz(sz, pmObject);
      ni = GetEditN(hdlg, deDi_I);
      ryw = GetEditR(hdlg, deDi_Yw);
      EnsureN(na, FValidAspect(na), "aspect count");
      EnsureN(nro, FItem(nro) || nro == -1, "required object");
      EnsureN(ni, FValidScreen(ni), "text columns");
      EnsureR(ryw, ryw >= 0.0, "stationary velocity");
      us.fEuroDate = GetCheck(dxDi_Yd);
      us.fEuroTime = GetCheck(dxDi_Yt);
      us.fOffsetOnly = GetCheck(dxDi_Yz1);
      us.fEuroDist = GetCheck(dxDi_Yv);
      us.fRound = GetCheck(dxDi_Yr);
      us.fSeconds = GetCheck(dxDi_b0);
      WiCheckMenu(cmdSecond, us.fSeconds);
      us.fSecond1K = GetCheck(dxDi_b1);
      us.fSecondHide = GetCheck(dxDi_b2);
      us.nAsp = na;
      for (i = us.nAsp + 1; i <= na; i++)
        ignorea[i] = fFalse;
      for (i = na + 1; i <= cAspect; i++)
        ignorea[i] = fTrue;
      us.fSmartCusp = GetCheck(dxDi_YC);
      us.objRequire = nro;
      us.fParallel2 = GetCheck(dxDi_AP);
      us.fDistance = GetCheck(dxDi_gd);
      us.nScreenWidth = ni;
      us.fClip80 = GetCheck(dxDi_Y8);
      us.fSabian = GetCheck(dxDi_I0);
      us.rStation = ryw;
      us.fEclipse = GetCheck(dxDi_Yu);
      us.fEclipseAny = !GetCheck(dxDi_Yu0);
      for (i = 0; i < arMax; i++)
        ignorez[i] = GetCheck(dxDi_z0 + i);
      us.nDegForm = GetCheck(dr04) ? 0 : (GetCheck(dr05) ? 1 :
        (GetCheck(dr06) ? 2 : 3));
      us.nCharset = GetRadio(hdlg, dr08, dr11);
      for (i = 0; i < rrMax; i++)
        ignore7[i] = GetCheck(dxDi_r0 + i);
      if (!ignore7[rrRay])
        EnsureRay();
      us.nAppSep = GetRadio(hdlg, dr12, dr14);
      WiCheckMenu(cmdApplying, us.nAppSep == 1);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the transit chart dialog, as brought up with the
// Chart / Transits menu command.

flag API DlgTransit(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int mon, day, yea, nty, nd, n1, n2;
  real tim, dst, zon;

  switch (message) {
  case WM_INITDIALOG:
    if (us.fInDay)           n1 = 1;
    else if (us.fInDayInf)   n1 = 2;
    else if (us.fInDayGra)   n1 = 3;
    else if (us.fTransit)    n1 = 4;
    else if (us.fTransitInf) n1 = 5;
    else if (us.fTransitGra) n1 = 6;
    else                     n1 = 0;
    SetRadio(dr01 + n1, dr01, dr07);
    SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
      MonT, DayT, YeaT, TimT);
    SetEditSZOA(hdlg, dcTrDst, dcTrZon, dcDeLon, dcDeLat,
      DstT, ZonT, LonT, LatT);
    SetCombo(dcTrDst, "Autodetect");
    n2 = us.fInDayMonth + us.fInDayYear +
      us.fInDayYear*(NAbs(us.nEphemYears) > 1);
    SetRadio(dr08 + n2, dr08, dr11);
    SetEditN(deTr_tY, us.nEphemYears);
    SetCheck(dxTr_YR0_s, us.fIgnoreSign);
    SetCheck(dxTr_YR0_d, us.fIgnoreDir);
    SetCheck(dxTr_YR1_l, us.fIgnoreDiralt);
    SetCheck(dxTr_YR1_d, us.fIgnoreDirlen);
    SetCheck(dxTr_YR2_0, us.fIgnoreAlt0);
    SetCheck(dxTr_YR2_d, us.fIgnoreDisequ);
    SetCheck(dxTr_p, is.fProgress);
    SetCheck(dxTr_r, is.fReturn);
    SetCheck(dxTr_5, us.fListAuto);
    SetCheck(dxTr_g, us.fGraphAll);
    SetEditN(deTr_d, us.nDivision);
    SetFocus(GetDlgItem(hdlg, dcTrMon));
    return fFalse;

  case WM_COMMAND:
#ifdef TIME
    if (wParam == dbTr_tn) {
      GetTimeNow(&mon, &day, &yea, &tim, ciDefa.dst, ciDefa.zon);
      SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
        mon, day, yea, tim);
      SetEditSZOA(hdlg, dcTrDst, dcTrZon, dcDeLon, dcDeLat,
        ciDefa.dst, ciDefa.zon, ciDefa.lon, ciDefa.lat);
    }
#endif
    if (wParam == IDOK) {
      GetEdit(dcTrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcTrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcTrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcTrTim, sz); tim = RParseSz(sz, pmTim);
      GetEdit(dcTrDst, sz); dst = RParseSz(sz, pmDst);
      GetEdit(dcTrZon, sz); zon = RParseSz(sz, pmZon);
      nty = GetEditN(hdlg, deTr_tY);
      nd = GetEditN(hdlg, deTr_d);
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      EnsureR(dst, FValidDst(dst), "daylight saving");
      EnsureR(zon, FValidZon(zon), "time zone");
      EnsureN(nd, FValidDivision(nd), "searching divisions");
      SetCI(ciTran, mon, day, yea, tim, dst, zon, ciDefa.lon, ciDefa.lat);
      us.nEphemYears = nty;
      us.fIgnoreSign   = GetCheck(dxTr_YR0_s);
      us.fIgnoreDir    = GetCheck(dxTr_YR0_d);
      us.fIgnoreDiralt = GetCheck(dxTr_YR1_l);
      us.fIgnoreDirlen = GetCheck(dxTr_YR1_d);
      us.fIgnoreAlt0   = GetCheck(dxTr_YR2_0);
      us.fIgnoreDisequ = GetCheck(dxTr_YR2_d);
      is.fProgress = GetCheck(dxTr_p);
      is.fReturn   = GetCheck(dxTr_r);
      us.fListAuto = GetCheck(dxTr_5);
      us.fGraphAll = GetCheck(dxTr_g);
      us.nDivision = nd;
      for (n1 = dr01; n1 < dr07; n1++)
        if (GetCheck(n1))
          break;
      n1 -= dr01;
      switch (n1) {
      case 1: wi.nMode = gTraTraTim; break;
      case 2: wi.nMode = gTraTraInf; break;
      case 3: wi.nMode = gTraTraGra; break;
      case 4: wi.nMode = gTraNatTim; break;
      case 5: wi.nMode = gTraNatInf; break;
      case 6: wi.nMode = gTraNatGra; break;
      default:
        if (us.fInDay || us.fInDayInf || us.fInDayGra ||
          us.fTransit || us.fTransitInf || us.fTransitGra)
          wi.nMode = gWheel;
      }
      n2 = GetCheck(dr08) ? 0 : (GetCheck(dr09) ? 1 :
        (GetCheck(dr10) ? 2 : 3));
      us.fInDayMonth = n2 >= 1;
      us.fInDayYear = us.fInDayMonth && n2 >= 2;
      if (n2 == 2 && NAbs(us.nEphemYears) > 1)
        us.nEphemYears = 0;
      if (n1 == 3 || n1 == 6)
        us.nEphemYears = (n2 <= 2 ? 1 : (nty <= 1 ? 5 : nty));
      else {
        if (n1 > 0)
          us.fGraphics = fFalse;
        if (n1 == 2 || n1 == 5) {
          wi.fCast = fTrue;
          if (n1 == 2)
            us.fProgress = is.fProgress;
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


CONST char *rgszProg[4] = {"Primary", "Secondary", "Tertiary2", "Tertiary1"};
CONST real rgrProg[4] =
  {rDayInYear * rDegMax, rDayInYear, 29.530588, 27.321661};
CONST char *rgszProgCusp[2] = {"Quotidian", "Solar"};
CONST real rgrProgCusp[2] = {1.0, rDayInYear};

// Processing function for the progression settings dialog, as brought up with
// the Chart / Progressions menu command.

flag API DlgProgress(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], szT[cchSzDef];
  int npO, mon, day, yea, i;
  real tim, dst, zon, rd, rC;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxPr_p, us.fProgress);
    SetRadio(dr01 + (us.nProgress == ptCast ? 0 :
      (us.nProgress == ptMixed ? 1 : 2)), dr01, dr03);
    SetEditR(hdlg, dcPr_pd, us.rProgDay, -6);
    for (i = 0; i < 4; i++) {
      FormatR(szT, rgrProg[i], -6);
      sprintf(sz, "%s %s", szT, rgszProg[i]);
      SetCombo(dcPr_pd, sz);
      if (us.rProgDay == rgrProg[i])
        SetEdit(dcPr_pd, sz);
    }
    SetEditR(hdlg, dcPr_pC, us.rProgCusp, -6);
    for (i = 0; i < 2; i++) {
      FormatR(szT, rgrProgCusp[i], -6);
      sprintf(sz, "%s %s", szT, rgszProgCusp[i]);
      SetCombo(dcPr_pC, sz);
      if (us.rProgCusp == rgrProgCusp[i])
        SetEdit(dcPr_pC, sz);
    }
    if (us.objProgArc >= 0)
      SetEdit(dePr_pO, szObjName[us.objProgArc]);
    else
      SetEdit(dePr_pO, "None");
    SetCheck(dxPr_pc, us.fProgRAMC);
    SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
      MonT, DayT, YeaT, TimT);
    SetEditSZOA(hdlg, dcPrDst, dcPrZon, dcDeLon, dcDeLat,
      DstT, ZonT, LonT, LatT);
    SetCombo(dcPrDst, "Autodetect");
    SetFocus(GetDlgItem(hdlg, dcPrMon));
    return fFalse;

  case WM_COMMAND:
#ifdef TIME
    if (wParam == dbPr_pn) {
      GetTimeNow(&mon, &day, &yea, &tim, ciDefa.dst, ciDefa.zon);
      SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
        mon, day, yea, tim);
      SetEditSZOA(hdlg, dcPrDst, dcPrZon, dcDeLon, dcDeLat,
        ciDefa.dst, ciDefa.zon, ciDefa.lon, ciDefa.lat);
    }
#endif

    if (wParam == IDOK) {
      GetEdit(dcPr_pd, sz);
      i = ChCap(sz[0]) == 'X';
      rd = RFromSz(sz + i);
      if (i != 0 && rd != 0.0)
        rd = rDayInYear / rd;
      rC = GetEditR(hdlg, dcPr_pC);
      GetEdit(dePr_pO, sz); npO = NParseSz(sz, pmObject);
      GetEdit(dcPrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcPrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcPrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcPrTim, sz); tim = RParseSz(sz, pmTim);
      GetEdit(dcPrDst, sz); dst = RParseSz(sz, pmDst);
      GetEdit(dcPrZon, sz); zon = RParseSz(sz, pmZon);
      EnsureR(rd, rd != 0.0, "degree per day");
      EnsureR(rC, rC != 0.0, "cusp move ratio");
      EnsureN(npO, FValidProgArc(npO), "solar arc planet");
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      EnsureR(dst, FValidDst(dst), "daylight saving");
      EnsureR(zon, FValidZon(zon), "time zone");
      us.fProgress = GetCheck(dxPr_p);
      WiCheckMenu(cmdProgress, us.fProgress);
      us.nProgress = GetCheck(dr01) ? ptCast :
        (GetCheck(dr02) ? ptMixed : ptSolarArc);
      us.rProgDay = rd;
      us.rProgCusp = rC;
      us.objProgArc = npO;
      us.fProgRAMC = GetCheck(dxPr_pc);
      SetCI(ciTran, mon, day, yea, tim, dst, zon, ciDefa.lon, ciDefa.lat);
      is.JDp = MdytszToJulian(MonT, DayT, YeaT, TimT, ciDefa.dst, ciDefa.zon);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


CONST char *rgszSort[asMax] = {"Power", "Orb Magnitude",
  "Orb Value", "1st Object Index", "2nd Object Index",
  "Aspect", "1st Object Position", "2nd Object Position", "Midpoint"};

// Processing function for the chart subsettings dialog, as brought up with
// the Chart / Chart Settings menu command.

flag API DlgChart(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int nw, nl, nl2, np, nn, yb, nT, i;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxCh_v0, us.fVelocity);
    SetEditN(deCh_w,  us.nWheelRows);
    SetCheck(dxCh_w0, us.fWheelReverse);
    SetCheck(dxCh_g0, us.fGridConfig);
    SetCheck(dxCh_gm, us.fGridMidpoint);
    SetCheck(dxCh_a0, us.fAspSummary);
    SetCheck(dxCh_m0, us.fMidSummary);
    SetCheck(dxCh_ma, us.fMidAspect);
    SetCheck(dxCh_Z0, us.fPrimeVert);
    SetCheck(dxCh_l,  us.fSectorApprox);
    SetCheck(dxCh_Ky, us.fCalendarYear);
    SetCheck(dxCh_j0, us.fInfluenceSign);
    SetEditN(deCh_L,  us.nAstroGraphStep);
    SetCheck(dxCh_L0, us.fLatitudeCross);
    sprintf(sz, "%d%s", us.nAstroGraphDist, us.fEuroDist ? "km" : "mi");
    SetEdit(deCh_L2, sz);
    SetCheck(dxCh_Ey, us.nEphemYears != 0);
    SetEditN(deCh_P,  us.nArabicParts);
    SetCheck(dxCh_P0, us.fArabicFlip);
    SetEditN(deCh_Nl, us.nAtlasList);
    SetEditN(deCh_Yb, us.nBioday);
    switch (us.nStarSort) {
    case 'z': nT = dr02; break;
    case 'l': nT = dr03; break;
    case 'n': nT = dr04; break;
    case 'b': nT = dr05; break;
    case 'd': nT = dr06; break;
    case 'v': nT = dr07; break;
    default:  nT = dr01;
    }
    SetRadio(nT, dr01, dr07);
    switch (us.nArabicSort) {
    case 'z': nT = dr09; break;
    case 'n': nT = dr10; break;
    case 'f': nT = dr11; break;
    default:  nT = dr08;
    }
    SetRadio(nT, dr08, dr11);
    for (i = 0; i < asMax; i++)
      SetCombo(dcCh_a, rgszSort[i]);
    SetEdit(dcCh_a, rgszSort[us.nAspectSort]);
    for (i = 0; i < ddMax; i++)
      SetCombo(dcCh_v3, rgszDecan[i]);
    SetEdit(dcCh_v3, rgszDecan[us.fListDecan ? us.nDecanType : 0]);
    SetEditR(hdlg, deCh_rc0, us.rRatio, 6);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      nw = GetEditN(hdlg, deCh_w);
      nl = GetEditN(hdlg, deCh_L);
      nl2 = GetEditN(hdlg, deCh_L2);
      np = GetEditN(hdlg, deCh_P);
      nn = GetEditN(hdlg, deCh_Nl);
      yb = GetEditN(hdlg, deCh_Yb);
      EnsureN(nw, FValidWheel(nw), "wheel row");
      EnsureN(nl, FValidAstrograph(nl), "astrocartography step");
      EnsureN(nl2, nl2 >= 0, "latitude crossing count");
      EnsureN(np, FValidPart(np), "Arabic part");
      EnsureN(nn, nn >= 0, "nearest city count");
      EnsureN(yb, FValidBioday(yb), "Biorhythm days");
      us.fVelocity = GetCheck(dxCh_v0);
      us.nWheelRows = nw;
      us.fWheelReverse = GetCheck(dxCh_w0);
      us.fGridConfig = GetCheck(dxCh_g0);
      us.fGridMidpoint = GetCheck(dxCh_gm);
      us.fAspSummary = GetCheck(dxCh_a0);
      us.fMidSummary = GetCheck(dxCh_m0);
      us.fMidAspect = GetCheck(dxCh_ma);
      us.fPrimeVert = GetCheck(dxCh_Z0);
      us.fSectorApprox = GetCheck(dxCh_l);
      us.fCalendarYear = GetCheck(dxCh_Ky);
      us.fInfluenceSign = GetCheck(dxCh_j0);
      us.nAstroGraphStep = nl;
      us.fLatitudeCross = GetCheck(dxCh_L0);
      us.nAstroGraphDist = nl2;
      us.nEphemYears = GetCheck(dxCh_Ey);
      us.nArabicParts = np;
      us.fArabicFlip = GetCheck(dxCh_P0);
      us.nAtlasList = nn;
      us.nBioday = yb;
      us.nStarSort = GetCheck(dr02) ? 'z' : (GetCheck(dr03) ? 'l' :
        (GetCheck(dr04) ? 'n' : (GetCheck(dr05) ? 'b' :
        (GetCheck(dr06) ? 'd' : (GetCheck(dr07) ? 'v' : 0)))));
      us.nArabicSort = GetCheck(dr09) ? 'z' : (GetCheck(dr10) ? 'n' :
        (GetCheck(dr11) ? 'f' : 0));
      GetEdit(dcCh_a, sz);
      for (i = 1; i < asMax; i++)
        if (FMatchSz(sz, rgszSort[i]))
          us.nAspectSort = i;
      us.rRatio = GetEditR(hdlg, deCh_rc0);
      GetEdit(dcCh_v3, sz);
      for (i = 0; i < ddMax; i++)
        if (FMatchSz(sz, rgszDecan[i]))
          break;
      if (i >= ddMax)
        i = 0;
      us.fListDecan = (i > ddNone);
      if (i > ddNone)
        us.nDecanType = i;
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


CONST char *rgszFontDisp[cFont] = {szAppNameCore, "Wingdings", "Astro",
  "Enigma", "Hamburg", "Astronomicon", "StarFont", "StarFont Serif",
  "Hank's Nakshatra", "Arial", "Courier New", "Consolas", "Lucida",
  "Cascadia"};
CONST char *rgszCityColor[6] = {"None", "Region", "Region+State",
  "Generic Zone", "Current Zone", "Rainbow"};
CONST char *rgszWheelCorner[7] = {"None", "Spider Web", "Moire Pattern",
  "Rays 1", "Rays 1,2", "Rays 12345", "Hearts"};
CONST char *rgszDecaFill[8] = {"None", "Standard", "Rainbow RGB",
  "Rainbow RYB", "Ruler Sign", "Ruler House", "7 Rays Sign", "7 Rays House"};

// Processing function for the graphic settings dialog, as brought up with the
// Graphics / Graphics Settings menu command.

flag API DlgGraphics(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int nx, ny, ns, nS, ng, nxz, nwn, nx1, nyxv, is, i, j;
  real ryxs, rxw, rxg;

  switch (message) {
  case WM_INITDIALOG:
    SetEditN(deGr_Xw_x, gs.xWin);
    SetEditN(deGr_Xw_y, gs.yWin);
    SetCheck(dxGr_XQ, gs.fKeepSquare);
    for (is = 100; is <= MAXSCALE; is += 50) {
      sprintf(sz, "%d", is);
      if (is % 100 == 0)
        SetCombo(dcGr_Xs, sz);
      SetCombo(dcGr_XSS, sz);
    }
    SetEditN(dcGr_Xs, gs.nScale);
    SetEditN(dcGr_XSS, gs.nScaleText);
    SetCheck(dxGr_XQ0, gs.fAutoScale);
    SetEditN(deGr_YXg, gs.nGridCell);
    SetEditN(deGr_YXj, gs.cspace);
    if (gs.objTrack >= 0)
      SetEdit(deGr_XZ, szObjName[gs.objTrack]);
    else
      SetEdit(deGr_XZ, "None");
    SetEditR(hdlg, deGr_YXS, gs.rspace, -6);
    SetEditR(hdlg, deGr_XW, gs.rRot, -3);
    SetEditR(hdlg, deGr_XG, gs.rTilt, -3);
    SetCheck(dxGr_XP0, gs.fSouth);
    SetCheck(dxGr_XW0, gs.fMollweide);
    SetEditN(deGr_WN, wi.nTimerDelay);
    SetCheck(dxGr_XN, gs.fAnimMap);
    SetCheck(dxGr_Wn, wi.fNoUpdate);
    SetCheck(dxGr_XU1, FOdd(gs.nAllStar));
    SetCheck(dxGr_XU2, (gs.nAllStar & 2) > 0);
    SetRadio(gs.objLeft > 0 ? dr02 :
      (gs.objLeft < 0 ? dr03 : dr01), dr01, dr03);
    SetEdit(deGr_X1, szObjName[gs.objLeft == 0 ? oSun : NAbs(gs.objLeft)-1]);
    for (i = 0; i < 7; i++)
      SetCombo(dcGr_YXv, rgszWheelCorner[i < 3 ? i : (i == 3 ? 6 : i-1)]);
    SetEdit(dcGr_YXv, rgszWheelCorner[gs.nDecaType]);
    SetEditN(deGr_YXv, gs.nDecaSize);
    for (i = 0; i < 8; i++)
      SetCombo(dcGr_Xv, rgszDecaFill[i]);
    SetEdit(dcGr_Xv, rgszDecaFill[gs.nDecaFill]);
    for (i = dcGr_Xf0; i <= dcGr_Xf5; i++)
      for (j = 0; j < cFont; j++) {
        if (rgszFontAllow[i-dcGr_Xf0][j] < '0')
          continue;
        SetCombo(i, rgszFontDisp[j]);
      }
    SetEdit(dcGr_Xf0, rgszFontDisp[gs.nFontTxt]);
    SetEdit(dcGr_Xf1, rgszFontDisp[gs.nFontSig]);
    SetEdit(dcGr_Xf2, rgszFontDisp[gs.nFontHou]);
    SetEdit(dcGr_Xf3, rgszFontDisp[gs.nFontObj]);
    SetEdit(dcGr_Xf4, rgszFontDisp[gs.nFontAsp]);
    SetEdit(dcGr_Xf5, rgszFontDisp[gs.nFontNak]);
    SetRadio(drg01 + gs.nGlyphCap - 1, drg01, drg02);
    SetRadio(drg03 + gs.nGlyphUra - 1, drg03, drg04);
    SetRadio(drg05 + gs.nGlyphPlu - 1, drg05, drg07);
    SetRadio(drg08 + gs.nGlyphLil - 1, drg08, drg09);
    SetRadio(drg10 + gs.nGlyphVer - 1, drg10, drg11);
    SetRadio(drg12 + gs.nGlyphEri - 1, drg12, drg13);
    for (i = 0; i < 6; i++)
      SetCombo(dcGr_XL, rgszCityColor[i]);
    SetEdit(dcGr_XL, rgszCityColor[gs.fLabelAsp ? gs.nLabelCity : 0]);
    SetFocus(GetDlgItem(hdlg, deGr_Xw_x));
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      nx = GetEditN(hdlg, deGr_Xw_x);
      ny = GetEditN(hdlg, deGr_Xw_y);
      ns = GetEditN(hdlg, dcGr_Xs);
      nS = GetEditN(hdlg, dcGr_XSS);
      ng = GetEditN(hdlg, deGr_YXg);
      GetEdit(deGr_XZ, sz); nxz = NParseSz(sz, pmObject);
      ryxs = GetEditR(hdlg, deGr_YXS);
      rxw = GetEditR(hdlg, deGr_XW);
      rxg = GetEditR(hdlg, deGr_XG);
      nwn = GetEditN(hdlg, deGr_WN);
      nyxv = GetEditN(hdlg, deGr_YXv);
      GetEdit(deGr_X1, sz); nx1 = NParseSz(sz, pmObject);
      EnsureN(nx, FValidGraphX(nx), "horizontal size");
      EnsureN(ny, FValidGraphY(ny), "vertical size");
      EnsureN(ns, FValidScale(ns), "character scale");
      EnsureN(nS, FValidScaleText(nS), "text scale");
      EnsureN(ng, FValidGrid(ng), "grid cell");
      EnsureN(nxz, FValidTelescope(nxz), "telescope planet");
      EnsureR(ryxs, FValidZoom(ryxs), "telescope zoom");
      EnsureR(rxw, FValidRotation(rxw), "horizontal rotation");
      EnsureR(rxg, FValidTilt(rxg), "vertical tilt");
      EnsureN(nwn, FValidTimer(nwn), "animation delay");
      EnsureN(nx1, FItem(nx1), "rotation planet");
      EnsureN(nyxv, FValidDecaSize(nyxv), "wheel corners coverage");
      if (gs.xWin != nx || gs.yWin != ny) {
        gs.xWin = nx; gs.yWin = ny;
        if (wi.fWindowChart)
          ResizeWindowToChart();
      }
      gs.fKeepSquare = GetCheck(dxGr_XQ);
      gs.nScale = ns; gs.nScaleText = nS;
      gs.fAutoScale = GetCheck(dxGr_XQ0);
      gs.nGridCell = ng;
      i = GetEditN(hdlg, deGr_YXj);
      if (gs.cspace != i) {
        gs.cspace = i;
        if (gi.rgspace != NULL) {
          DeallocateP(gi.rgspace);
          gi.rgspace = NULL;
        }
      }
      gs.objTrack = nxz;
      gs.rspace = ryxs;
      gs.rRot = rxw; gs.rTilt = rxg;
      if (wi.nTimerDelay != nwn) {
        wi.nTimerDelay = nwn;
        if (wi.lTimer != 0)
          KillTimer(wi.hwnd, 1);
        wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
      }
      gs.fSouth = GetCheck(dxGr_XP0);
      gs.fMollweide = GetCheck(dxGr_XW0);
      gs.fAnimMap = GetCheck(dxGr_XN);
      wi.fNoUpdate = GetCheck(dxGr_Wn);
      gs.nAllStar = (GetCheck(dxGr_XU2) << 1) | GetCheck(dxGr_XU1);
      gs.objLeft = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? nx1+1 : -nx1-1);
      GetEdit(dcGr_YXv, sz);
      for (i = 0; i < 7; i++)
        if (FEqSzI(sz, rgszWheelCorner[i]))
          break;
      gs.nDecaType = i < 7 ? i : 0;
      gs.nDecaSize = nyxv;
      GetEdit(dcGr_Xv, sz);
      for (i = 0; i < 8; i++)
        if (FEqSzI(sz, rgszDecaFill[i]))
          break;
      gs.nDecaFill = i < 8 ? i : 0;
      for (i = dcGr_Xf0; i <= dcGr_Xf5; i++) {
        GetEdit(i, sz);
        // Astro font in slot #2 gets checked first, since it's a substring.
        for (j = 2; j < cFont; j += (j == 2 ? -2 : (j == 1 ? 2 : 1))) {
          if (rgszFontAllow[i-dcGr_Xf0][j] >= '0' &&
            FMatchSz(sz, rgszFontDisp[j])) {
            break;
          }
        }
        if (j >= cFont)    // If no match, default to "Astrolog".
          j = 0;
        switch (i) {
        case dcGr_Xf0: gs.nFontTxt = j; break;
        case dcGr_Xf1: gs.nFontSig = j; break;
        case dcGr_Xf2: gs.nFontHou = j; break;
        case dcGr_Xf3: gs.nFontObj = j; break;
        case dcGr_Xf4: gs.nFontAsp = j; break;
        case dcGr_Xf5: gs.nFontNak = j; break;
        }
      }
      gs.nFontAll = gs.nFontTxt*0x100000 + gs.nFontSig*0x10000 +
        gs.nFontHou*0x1000 + gs.nFontObj*0x100 + gs.nFontAsp*0x10 +
        gs.nFontNak;
      if (gs.nFontAll != 0)
        gi.nFontPrev = gs.nFontAll;
      gs.nGlyphCap = GetRadio(hdlg, drg01, drg02) + 1;
      gs.nGlyphUra = GetRadio(hdlg, drg03, drg04) + 1;
      gs.nGlyphPlu = GetRadio(hdlg, drg05, drg07) + 1;
      gs.nGlyphLil = GetRadio(hdlg, drg08, drg09) + 1;
      gs.nGlyphVer = GetRadio(hdlg, drg10, drg11) + 1;
      gs.nGlyphEri = GetRadio(hdlg, drg12, drg13) + 1;
      GetEdit(dcGr_XL, sz);
      for (i = 0; i < 6; i++)
        if (FEqSzI(sz, rgszCityColor[i]))
          break;
      if (i <= 0)
        gs.fLabelAsp = fFalse;
      else if (i < 6) {
        gs.fLabelAsp = fTrue;
        gs.nLabelCity = i;
      }
      us.fGraphics = wi.fRedraw = wi.fMenuAll = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the about dialog, showing copyrights and credits,
// as brought up with the Help / About Astrolog menu command.

flag API DlgAbout(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
#ifdef DEBUG
    SetWindowText(hdlg, "About Astrolog (DEBUG)");
#endif
    sprintf(sz, "%s version %s for %s Windows",
      szAppName, szVersionCore, szArchCore);
    SetDlgItemText(hdlg, ds01, sz);
    sprintf(sz, "Released %s", szDateCore);
    SetDlgItemText(hdlg, ds02, sz);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}
#endif // WIN

/* wdialog.cpp */
