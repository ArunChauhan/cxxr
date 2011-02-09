/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2009,2010 The R Development Core Team.
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

/* This is an experimental facility for printing low-level information
   about R objects. It is not intended to be exposed at the top level
   but rather used as a debugging/inspection facility. It is not
   necessarily complete - feel free to add missing pieces. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>

// This is currently a no-op in CXXR (and quite likely always will be):

/* internal API - takes one mandatory argument (object to inspect) and
   two optional arguments (deep and pvec - see above), positional argument
   matching only */
SEXP attribute_hidden do_inspect(SEXP call, SEXP op, SEXP args, SEXP env) {
    SEXP obj = CAR(args);
    int deep = -1;
    int pvec = 5;
    if (CDR(args) != R_NilValue) {
	deep = asInteger(CADR(args));
	if (CDDR(args) != R_NilValue)
	    pvec = asInteger(CADDR(args));
    }
	
    // inspect(0, CAR(args), deep, pvec);
    return obj;
}

