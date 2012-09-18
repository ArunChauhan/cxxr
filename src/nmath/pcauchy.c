/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-12 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000 The R Core Team
 *  Copyright (C) 2004 The R Foundation
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
 *
 *  DESCRIPTION
 *
 *	The distribution function of the Cauchy distribution.
 */

#include "nmath.h"
#include "dpq.h"

double pcauchy(double x, double location, double scale,
	       int lower_tail, int log_p)
{
#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(location) || ISNAN(scale))
	return x + location + scale;
#endif
    if (scale <= 0) ML_ERR_return_NAN;

    x = (x - location) / scale;
    if (ISNAN(x)) ML_ERR_return_NAN;
#ifdef IEEE_754
    if(!R_FINITE(x)) {
	if(x < 0) return R_DT_0;
	else return R_DT_1;
    }
#endif
    if (!lower_tail)
	x = -x;
    /* for large x, the standard formula suffers from cancellation.
     * This is from Morten Welinder thanks to  Ian Smith's  atan(1/x) : */
    if (fabs(x) > 1) {
	double y = atan(1/x) / M_PI;
	return (x > 0) ? R_D_Clog(y) : R_D_val(-y);
    } else
	return R_D_val(0.5 + atan(x) / M_PI);
}
