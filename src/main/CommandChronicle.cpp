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

/* This file incorporates material Copyright (C) Chris A. Silles 2009-12.
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/** @file CommandChronicle.cpp
 *
 * Implementation of class CommandChronicle.
 */

#include "CXXR/CommandChronicle.hpp"

#include "CXXR/Provenance.hpp"

using namespace CXXR;

void CommandChronicle::detachReferents()
{
    m_read_set.clear();
    m_reads.clear();
    m_command.detach();
}

void CommandChronicle::readBinding(const Provenance* bdgprov)
{
    std::pair<std::set<const Provenance*>::iterator, bool> pr
	= m_read_set.insert(bdgprov);
    if (pr.second) {
	GCEdge<const Provenance> parent(bdgprov);
	m_reads.push_back(parent);
    }
}

void CommandChronicle::visitReferents(const_visitor* v) const
{
    for (ParentVector::const_iterator it = m_reads.begin();
	 it != m_reads.end(); ++it) {
	const GCNode* parent = *it;
	(*v)(parent);
    }
    if (m_command)
	(*v)(m_command);
}

BOOST_CLASS_EXPORT_IMPLEMENT(CXXR::CommandChronicle)
