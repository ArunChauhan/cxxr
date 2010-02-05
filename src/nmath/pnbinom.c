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
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000-8 The R Development Core Team
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
 *	The distribution function of the negative binomial distribution.
 *
 *  NOTES
 *
 *	x = the number of failures before the n-th success
 */

#include "nmath.h"
#include "dpq.h"

double pnbinom(double x, double size, double prob, int lower_tail, int log_p)
{
#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(size) || ISNAN(prob))
	return x + size + prob;
    if(!R_FINITE(size) || !R_FINITE(prob))	ML_ERR_return_NAN;
#endif
    if (size <= 0 || prob <= 0 || prob > 1)	ML_ERR_return_NAN;

    if (x < 0) return R_DT_0;
    if (!R_FINITE(x)) return R_DT_1;
    x = floor(x + 1e-7);
    return pbeta(prob, size, x + 1, lower_tail, log_p);
}

double pnbinom_mu(double x, double size, double mu, int lower_tail, int log_p)
{
#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(size) || ISNAN(mu))
	return x + size + mu;
    if(!R_FINITE(size) || !R_FINITE(mu))	ML_ERR_return_NAN;
#endif
    if (size <= 0 || mu < 0)	ML_ERR_return_NAN;

    if (x < 0) return R_DT_0;
    if (!R_FINITE(x)) return R_DT_1;
    x = floor(x + 1e-7);
    /* return
     * pbeta(pr, size, x + 1, lower_tail, log_p);  pr = size/(size + mu), 1-pr = mu/(size+mu)
     *
     *= pbeta_raw(pr, size, x + 1, lower_tail, log_p)
     *            x.  pin   qin
     *=  bratio (pin,  qin, x., 1-x., &w, &wc, &ierr, log_p),  and return w or wc ..
     *=  bratio (size, x+1, pr, 1-pr, &w, &wc, &ierr, log_p) */
    {
	int ierr;
	double w, wc;
	bratio(size, x+1, size/(size+mu), mu/(size+mu), &w, &wc, &ierr, log_p);
	if(ierr)
	    MATHLIB_WARNING(_("pnbinom_mu() -> bratio() gave error code %d"), ierr);
	return lower_tail ? w : wc;
    }
}
