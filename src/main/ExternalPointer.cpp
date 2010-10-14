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

/** @file ExternalPointer.cpp
 *
 * @brief Class CXXR::ExternalPointer and associated C interface.
 */

#include "CXXR/ExternalPointer.h"

#include "localization.h"
#include "CXXR/GCStackRoot.hpp"

using namespace std;
using namespace CXXR;

// Force the creation of non-inline embodiments of functions in the C
// interface:
namespace CXXR {
    namespace ForceNonInline {
	void* (*R_ExternalPtrAddrp)(SEXP) = R_ExternalPtrAddr;
	RObject* (*R_ExternalPtrTagp)(SEXP) = R_ExternalPtrTag;
	RObject* (*R_ExternalPtrProtectedp)(SEXP) = R_ExternalPtrProtected;
	void (*R_ClearExternalPtrp)(SEXP) = R_ClearExternalPtr;
    }
}

void ExternalPointer::detachReferents()
{
    m_protege.detach();
    m_tag.detach();
    RObject::detachReferents();
}

const char* ExternalPointer::typeName() const
{
    return ExternalPointer::staticTypeName();
}

void ExternalPointer::visitReferents(const_visitor* v) const
{
    const GCNode* protege = m_protege;
    const GCNode* tag = m_tag;
    RObject::visitReferents(v);
    if (protege)
	(*v)(protege);
    if (tag)
	(*v)(tag);
}

SEXP R_MakeExternalPtr(void *p, SEXP tag, SEXP prot)
{
    return CXXR_NEW(ExternalPointer(p, tag, prot));
}

void R_SetExternalPtrAddr(SEXP s, void *p)
{
    CXXR::ExternalPointer& ep
	= *CXXR::SEXP_downcast<CXXR::ExternalPointer*>(s);
    ep.setPtr(p);
}

void R_SetExternalPtrTag(SEXP s, SEXP tag)
{
    CXXR::ExternalPointer& ep
	= *CXXR::SEXP_downcast<CXXR::ExternalPointer*>(s);
    ep.setTag(tag);
}

void R_SetExternalPtrProtected(SEXP s, SEXP p)
{
    CXXR::ExternalPointer& ep
	= *CXXR::SEXP_downcast<CXXR::ExternalPointer*>(s);
    ep.setProtege(p);
}
