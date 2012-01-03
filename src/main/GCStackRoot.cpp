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

/** @file GCStackRoot.cpp
 *
 * Implementation of class GCStackRootBase.
 */

#include "CXXR/GCStackRoot.hpp"

#include <cstdlib>
#include <iostream>

using namespace std;
using namespace CXXR;

GCStackRootBase* GCStackRootBase::s_roots = 0;

void GCStackRootBase::destruct_aux()
{
    GCNode::decRefCount(m_target);
}

void GCStackRootBase::protectAll()
{
    GCStackRootBase* sr = s_roots;
    while (sr && !sr->m_protecting) {
	GCNode::incRefCount(sr->m_target);
	sr->m_protecting = true;
	sr = sr->m_next;
    }
}

void GCStackRootBase::retarget_aux(const GCNode* node)
{
    GCNode::incRefCount(node);
    GCNode::decRefCount(m_target);
}

void GCStackRootBase::seq_error()
{
    cerr << "Fatal error:"
	    " GCStackRoots must be destroyed in reverse order of creation\n";
    abort();
}

void GCStackRootBase::visitRoots(GCNode::const_visitor* v)
{
    GCStackRootBase* root = s_roots;
    while (root) {
	if (root->m_target)
	    (*v)(root->m_target);
	root = root->m_next;
    }
}
