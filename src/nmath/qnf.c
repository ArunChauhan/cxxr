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
 *  Copyright (C) 2006-8 The R Development Core Team
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

#include "nmath.h"
#include "dpq.h"

double qnf(double p, double df1, double df2, double ncp, int lower_tail, 
	   int log_p)
{
    double y;
    
#ifdef IEEE_754
    if (ISNAN(p) || ISNAN(df1) || ISNAN(df2) || ISNAN(ncp))
	return p + df1 + df2 + ncp;
#endif
    if (df1 <= 0. || df2 <= 0. || ncp < 0) ML_ERR_return_NAN;
    if (!R_FINITE(ncp)) ML_ERR_return_NAN;
    if (!R_FINITE(df1) && !R_FINITE(df2)) ML_ERR_return_NAN;
    R_Q_P01_boundaries(p, 0, ML_POSINF);

    if (df2 > 1e8) /* avoid problems with +Inf and loss of accuracy */
	return qnchisq(p, df1, ncp, lower_tail, log_p)/df1;

    y = qnbeta(p, df1 / 2., df2 / 2., ncp, lower_tail, log_p);
    return y/(1-y) * (df2/df1);
}
