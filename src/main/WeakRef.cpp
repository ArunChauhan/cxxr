/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998-2007   The R Development Core Team.
 *  Copyright (C) 2007 Andrew Runnalls.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street Fifth Floor, Boston, MA 02110-1301  USA
 */

/** @file WeakRef.cpp
 *
 * Class WeakRef
 */

#include <iostream>

#include "CXXR/WeakRef.h"

using namespace std;
using namespace CXXR;

const unsigned int WeakRef::READY_TO_FINALIZE;
const unsigned int WeakRef::FINALIZE_ON_EXIT;

WeakRef::WRList WeakRef::s_live;
WeakRef::WRList WeakRef::s_f10n_pending;
WeakRef::WRList WeakRef::s_tombstone;
int WeakRef::s_count = 0;

WeakRef::WeakRef(RObject* key, RObject* value, RObject* R_finalizer,
		 bool finalize_on_exit)
    : m_key(key), m_value(key, value), m_Rfinalizer(key, R_finalizer),
      m_lit(s_live.insert(s_live.end(), this))
{
    if (!m_key)
	tombstone();
    m_flags[FINALIZE_ON_EXIT] = finalize_on_exit;
    ++s_count;
}

WeakRef::WeakRef(RObject* key, RObject* value, R_CFinalizer_t C_finalizer,
		 bool finalize_on_exit)
    : m_key(key), m_value(key, value), m_Rfinalizer(key, 0),
      m_Cfinalizer(C_finalizer), m_lit(s_live.insert(s_live.end(), this))
{
    if (!m_key)
	tombstone();
    m_flags[FINALIZE_ON_EXIT] = finalize_on_exit;
    ++s_count;
}

WeakRef::~WeakRef()
{
    wrList()->erase(m_lit);
    --s_count;
}

bool WeakRef::check()
{
    // Check sizes:
    if (int(s_live.size() + s_f10n_pending.size() + s_tombstone.size())
	!= s_count) {
	cerr << "WeakRef::check() : tally error\n"
	     << "s_live.size(): " << s_live.size()
	     << "\ns_f10n_pending.size(): " << s_f10n_pending.size()
	     << "\ns_tombstone.size(): " << s_tombstone.size()
	     << "\ns_count: " << s_count << '\n';
	abort();
    }
    // Check s_live:
    for (WRList::const_iterator lit = s_live.begin();
	 lit != s_live.end(); ++lit) {
	const WeakRef* wr = *lit;
	if (wr->m_flags[READY_TO_FINALIZE]) {
	    cerr << "Node on s_live set READY_TO_FINALIZE\n";
	    abort();
	}
	if (!wr->m_key) {
	    cerr << "Node on s_live with null key\n";
	    abort();
	}
    }
    // Check s_f10n_pending:
    for (WRList::const_iterator lit = s_f10n_pending.begin();
	 lit != s_f10n_pending.end(); ++lit) {
	const WeakRef* wr = *lit;
	if (!wr->m_flags[READY_TO_FINALIZE]) {
	    cerr << "Node on s_f10n_pending not READY_TO_FINALIZE\n";
	    abort();
	}
	if (!wr->m_key) {
	    cerr << "Node on s_f10n_pending with null key\n";
	    abort();
	}
	if (!wr->m_Rfinalizer && !wr->m_Cfinalizer) {
	    cerr << "Node on s_f10n_pending without finalizer\n";
	    abort();
	}
    }
    // Check s_tombstone:
    for (WRList::const_iterator lit = s_tombstone.begin();
	 lit != s_tombstone.end(); ++lit) {
	const WeakRef* wr = *lit;
	if (wr->m_flags[READY_TO_FINALIZE]) {
	    cerr << "Node on s_tombstone set READY_TO_FINALIZE\n";
	    abort();
	}
	if (wr->m_key) {
	    cerr << "Node on s_tombstone with non-null key\n";
	    abort();
	}
    }
}

void WeakRef::markThru(unsigned int max_gen)
{
    WeakRef::check();
    GCNode::Marker marker(max_gen);
    WRList newlive;
    // Step 2-3 of algorithm.  Mark the value and R finalizer if the
    // key is marked, or in a generation not being collected.
    {
	bool newmarks;
	do {
	    newmarks = false;
	    WRList::iterator lit = s_live.begin();
	    while (lit != s_live.end()) {
		WeakRef* wr = *lit++;
		RObject* key = wr->key();
		if (key->m_gcgen > max_gen || key->isMarked()) {
		    RObject* value = wr->value();
		    if (value && value->conductVisitor(&marker))
			newmarks = true;
		    RObject* Rfinalizer = wr->m_Rfinalizer;
		    if (Rfinalizer && Rfinalizer->conductVisitor(&marker))
			newmarks = true;
		    wr->transfer(&s_live, &newlive);
		}
	    }
	} while (newmarks);
    }
    // Step 4 of algorithm.  Process references with unmarked keys.
    {
	WRList::iterator lit = s_live.begin();
	while (lit != s_live.end()) {
	    WeakRef* wr = *lit++;
	    RObject* Rfinalizer = wr->m_Rfinalizer;
	    if (Rfinalizer) Rfinalizer->conductVisitor(&marker);
	    if (Rfinalizer || wr->m_Cfinalizer) {
		wr->m_flags[READY_TO_FINALIZE] = true;
		wr->transfer(&s_live, &s_f10n_pending);
	    }
	    else
		wr->tombstone();
	}
    }
    // Step 5 of algorithm.  Mark all live references with reachable keys.
    {
	s_live.splice(s_live.end(), newlive);
	for (WRList::iterator lit = s_live.begin();
	     lit != s_live.end(); ++lit) {
	    WeakRef* wr = *lit;
	    wr->conductVisitor(&marker);
	}
    }
}

void WeakRef::tombstone()
{
    WRList* oldl = wrList();
    m_key = 0;
    m_value.redirect(0, 0);
    m_Rfinalizer.redirect(0, 0);
    m_Cfinalizer = 0;
    m_flags[READY_TO_FINALIZE] = false;
    transfer(oldl, &s_tombstone);
}

WeakRef::WRList* WeakRef::wrList() const
{
    return m_flags[READY_TO_FINALIZE] ? &s_f10n_pending :
	(m_key ? &s_live : &s_tombstone);
}
