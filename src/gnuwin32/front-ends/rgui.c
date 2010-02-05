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
 *  Copyright (C) 1998--2007  R Development Core Team
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
 *  along with this program; if not,  a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/* For AttachConsole: seems the MinGW headers are wrong and that
   requires XP or later, not 2000 or later. */
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdio.h>
#include <Rversion.h>
#include <Startup.h>
#include <stdlib.h>		/* for exit */

extern void cmdlineoptions(int, char **);
extern int setupui(void);
extern void Rf_mainloop(void);
__declspec(dllimport) extern UImode CharacterMode;
extern void GA_exitapp(void);

extern char *getDLLVersion(void);

static char Rversion[25];
char *getRVersion(void)
{
    snprintf(Rversion, 25, "%s.%s", R_MAJOR, R_MINOR);
    return(Rversion);
}

#include <wincon.h>
typedef BOOL (*AC)(DWORD);
int AppMain (int argc, char **argv)
{
    AC entry;

    CharacterMode = RGui;
    if(strcmp(getDLLVersion(), getRVersion()) != 0) {
	MessageBox(0, "R.DLL version does not match", "Terminating",
		   MB_TASKMODAL | MB_ICONSTOP | MB_OK);
	exit(1);
    }
    cmdlineoptions(argc, argv);
    if (!setupui()) {
        MessageBox(0, "Error setting up console.  Try --vanilla option.",
                      "Terminating", MB_TASKMODAL | MB_ICONSTOP | MB_OK);
        GA_exitapp();
    }

/* If we have this, C writes to stdout/stderr would get set to the 
   launching terminal (if there was one).  Unfortunately needs XP, and
   works for C but not Fortran. */

    entry = (AC) GetProcAddress((HMODULE)GetModuleHandle("KERNEL32"),
				"AttachConsole");
    if (entry && entry(ATTACH_PARENT_PROCESS))
    {
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
    }

    Rf_mainloop();
    /* NOTREACHED */
    return 0;
}
