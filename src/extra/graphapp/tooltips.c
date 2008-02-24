/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008 Andrew R. Runnalls, subject to such other
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
 *  Support for tooltip
 *  int addtooltip(obj,char *) (on error return 0)
 */

/*
   This file is an add-on  to GraphApp, a cross-platform C graphics library.
 */

#include "internal.h"
static HWND hwndToolTip = 0;
#ifndef TOOLTIPS_CLASS
#include "commctrl.h"
#endif

int addtooltip(control c, const char *tp)
{
    TOOLINFO ti;
    char *cc = (char*) &ti;
    int i, lim = sizeof (ti);
    for (i = 0; i++ < lim; *cc++ = 0);
    if (hwndToolTip == 0) {
        InitCommonControls();
    	hwndToolTip = CreateWindowEx(
           0,TOOLTIPS_CLASS,NULL,WS_POPUP|TTS_ALWAYSTIP,
           CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
           hwndFrame,NULL,this_instance,NULL);
        if(!hwndToolTip) return 0;
    }
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = (HWND) c->parent->handle;
    ti.uId = (UINT_PTR) c->handle;
    ti.lpszText = (LPSTR) tp;
    return
    (SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti)==TRUE)?
      1:0;
}



