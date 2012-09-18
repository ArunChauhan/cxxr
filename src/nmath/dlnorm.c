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
 *  Copyright (C) 2000-8 The R Core Team
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
 *    The density of the lognormal distribution.
 */

#include "nmath.h"
#include "dpq.h"

double dlnorm(double x, double meanlog, double sdlog, int give_log)
{
    double y;

#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(meanlog) || ISNAN(sdlog))
	return x + meanlog + sdlog;
#endif
    if(sdlog <= 0) ML_ERR_return_NAN;

    if(x <= 0) return R_D__0;

    y = (log(x) - meanlog) / sdlog;
    return (give_log ?
	    -(M_LN_SQRT_2PI   + 0.5 * y * y + log(x * sdlog)) :
	    M_1_SQRT_2PI * exp(-0.5 * y * y)  /	 (x * sdlog));
    /* M_1_SQRT_2PI = 1 / sqrt(2 * pi) */

}
