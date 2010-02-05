/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-10 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1999  Guido Masarotto
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/*
 *  int addstatusbar()  - add a simple status bar to the mdi frame
 *  void setstatus(char *text) - set text
 */

/*
   This file is an add-on  to GraphApp, a cross-platform C graphics library.
 */

#include "internal.h"

static char MDIStatusText[256] = "" ;
#ifndef SBARS_SIZEGRIP
#include "commctrl.h"
#endif

static HWND intMDIStatus=0;

int addstatusbar()
{
    int a[1] = {-1};
    if (!MDIFrame) return 0;
    if (MDIStatus) return 1;
    if(!intMDIStatus) {
	InitCommonControls();
	intMDIStatus = CreateStatusWindow(WS_CHILD|SBARS_SIZEGRIP|WS_VISIBLE,
					  "", hwndFrame, 121);
	if (!intMDIStatus) return 0;
	SendMessage(intMDIStatus,SB_SETPARTS,(WPARAM)1,(LPARAM)a);
	SendMessage(intMDIStatus,SB_SETTEXT,
		    (WPARAM) 0|0, (LPARAM)MDIStatusText);
    }
    MDIStatus = intMDIStatus;
    SendMessage(hwndFrame,WM_PAINT,(WPARAM) 0,(LPARAM) 0);
    return 1;
}

int delstatusbar()
{
    if (!MDIFrame) return 0;
    MDIStatus = 0; /* handle_mdiframeresize notices this */
    SendMessage(hwndFrame,WM_PAINT,(WPARAM) 0,(LPARAM) 0);
    return 1;
}

PROTECTED void updatestatus(const char *text)
{
    /* strncpy(MDIStatusText, text, 255); */
    if (!MDIStatus) return;
    SendMessage(MDIStatus,SB_SETTEXT,
		(WPARAM) 0|0, (LPARAM)MDIStatusText);
    SendMessage(MDIStatus, WM_PAINT, (WPARAM)0, (LPARAM)0);
}

void setstatus(const char *text)
{
    strncpy(MDIStatusText, text, 255);
    if (!MDIStatus || !current_window) return;
    strncpy(current_window->status, text, 255);
    updatestatus(text);
}
