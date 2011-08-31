/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-11 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1998-2005 R Development Core Team
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

#ifndef R_PARSE_H
#define R_PARSE_H

#include <R_ext/Parse.h>
#include <IOStuff.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public interface */
/* SEXP R_ParseVector(SEXP, int, ParseStatus *, SEXP); in R_ext/Parse.h */

/* Private interface */

typedef struct {

    Rboolean keepSrcRefs;	/* Whether to attach srcrefs to objects as they are parsed */
    SEXP SrcFile;		/* The srcfile object currently being parsed */
    PROTECT_INDEX SrcFileProt;	/* The SrcFile may change */
    				/* Position information about the current parse */
    int xxlineno;		/* Line number according to #line directives */
    int xxcolno;		/* Character number on line */
    int xxbyteno;		/* Byte number on line */
    int xxparseno;              /* Line number ignoring #line directives */
} SrcRefState;

void R_InitSrcRefState(SrcRefState *state);
void R_FinalizeSrcRefState(SrcRefState *state);

SEXP R_Parse1Buffer(IoBuffer*, int, ParseStatus *, SrcRefState *); /* in ReplIteration,
						       R_ReplDLLdo1 */
SEXP R_ParseBuffer(IoBuffer*, int, ParseStatus *, SEXP, SEXP); /* in source.c */
SEXP R_Parse1File(FILE*, int, ParseStatus *, SrcRefState *); /* in R_ReplFile */
SEXP R_ParseFile(FILE*, int, ParseStatus *, SEXP);  /* in edit.c */

#ifndef HAVE_RCONNECTION_TYPEDEF
typedef struct Rconn  *Rconnection;
#define HAVE_RCONNECTION_TYPEDEF
#endif
SEXP R_ParseConn(Rconnection con, int n, ParseStatus *status, SEXP srcfile);

	/* Report a parse error */
	
void parseError(SEXP call, int linenum);

#ifdef __cplusplus
}
#endif

#endif /* not R_PARSE_H */
