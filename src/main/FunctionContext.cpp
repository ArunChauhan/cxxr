/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-13 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/** @file FunctionContext.cpp
 *
 * Implementation of class FunctionContext.
 */

#include "CXXR/FunctionContext.hpp"

#include "CXXR/Environment.h"

using namespace std;
using namespace CXXR;

RObject* R_Srcref;

FunctionContext::FunctionContext(const Expression* the_call,
				 Environment* call_env,
			         const FunctionBase* function)
    : m_srcref(R_Srcref), m_call(the_call), m_call_env(call_env),
      m_function(function)
{
    setType(FUNCTION);
}

FunctionContext::~FunctionContext()
{
    R_Srcref = m_srcref;
}
    
FunctionContext* FunctionContext::innermost(Evaluator::Context* start)
{
    while (start && start->type() < FUNCTION)
	start = start->nextOut();
    return static_cast<FunctionContext*>(start);
}
