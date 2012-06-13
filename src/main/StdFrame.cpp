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

/** @file StdFrame.cpp
 *
 *
 * @brief Implementation of class CXXR:StdFrame.
 */

#include "CXXR/StdFrame.hpp"

#include <cmath>
#include "localization.h"
#include "R_ext/Error.h"
#include "CXXR/GCStackRoot.hpp"
#include "CXXR/SEXP_downcast.hpp"
#include "CXXR/Symbol.h"

using namespace std;
using namespace CXXR;

// We want to be able to determine quickly if a symbol is *not*
// defined in an frame, so that we can carry on working up the
// chain of enclosing frames.  On average the number of tests
// needed to determine that a symbol is not present is 1 + 2L, where L
// is the load factor.  So we keep the load factor small:
namespace {
    const float maximum_load_factor = 0.5;
}

StdFrame::StdFrame(size_t initial_capacity)
    : m_map(ceil(initial_capacity/maximum_load_factor))
{
    m_map.max_load_factor(maximum_load_factor);
}

PairList* StdFrame::asPairList() const
{
    GCStackRoot<PairList> ans(0);
    for (map::const_iterator it = m_map.begin(); it != m_map.end(); ++it)
	ans = (*it).second.asPairList(ans);
    return ans;
}

Frame::Binding* StdFrame::binding(const Symbol* symbol)
{
    map::iterator it = m_map.find(symbol);
    if (it == m_map.end())
	return 0;
    return &(*it).second;
}

const Frame::Binding* StdFrame::binding(const Symbol* symbol) const
{
    map::const_iterator it = m_map.find(symbol);
    if (it == m_map.end())
	return 0;
    return &(*it).second;
}

void StdFrame::clear()
{
    statusChanged(0);
    m_map.clear();
}

StdFrame* StdFrame::clone() const
{
    return expose(new StdFrame(*this));
}

void StdFrame::detachReferents()
{
    m_map.clear();
    Frame::detachReferents();
}

bool StdFrame::erase(const Symbol* symbol)
{
    if (isLocked())
	Rf_error(_("cannot remove bindings from a locked frame"));
    bool ans = m_map.erase(symbol);
    if (ans)
	statusChanged(symbol);
    return ans;
}

void StdFrame::import(const Frame* frame) {
    const StdFrame* stdFrame = static_cast<const StdFrame*>(frame);
    for (map::const_iterator it = stdFrame->m_map.begin(); it != stdFrame->m_map.end(); ++it) {
	const Symbol* symbol=(*it).first;
	const Binding* bdgSrc=&(*it).second;

	Binding* bdgDest = obtainBinding(symbol);
	bdgDest->setProvenance(const_cast<Provenance*>(bdgSrc->getProvenance()));
	bdgDest->setValue(bdgSrc->rawValue(), bdgSrc->origin(), TRUE);
    }
}

void StdFrame::lockBindings()
{
    for (map::iterator it = m_map.begin(); it != m_map.end(); ++it)
	(*it).second.setLocking(true);
}

Frame::Binding* StdFrame::obtainBinding(const Symbol* symbol)
{
    Binding& bdg = m_map[symbol];
    // Was this binding newly created?
    if (!bdg.frame()) {
	if (isLocked()) {
	    m_map.erase(symbol);
	    Rf_error(_("cannot add bindings to a locked frame"));
	}
	bdg.initialize(this, symbol);
	statusChanged(symbol);
    }
    return &bdg;
}

size_t StdFrame::size() const
{
    return m_map.size();
}

void StdFrame::softMergeInto(Frame* target) const
{
    for (map::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
	const Symbol* symbol = (*it).first;
	if (!target->binding(symbol)) {
	    const Binding& mybdg = (*it).second;
	    Binding* yourbdg = target->obtainBinding(symbol);
	    yourbdg->setValue(mybdg.value(), mybdg.origin());
	}
    }
}

vector<const Symbol*> StdFrame::symbols(bool include_dotsymbols) const
{
    vector<const Symbol*> ans;
    for (map::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
	const Symbol* symbol = (*it).first;
	if (include_dotsymbols || !isDotSymbol(symbol))
	    ans.push_back(symbol);
    }
    return ans;
}

void StdFrame::visitReferents(const_visitor* v) const
{
    Frame::visitReferents(v);
    for (map::const_iterator it = m_map.begin(); it != m_map.end(); ++it)
	(*it).second.visitReferents(v);
}
