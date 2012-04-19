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
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1999-2006   The R Development Core Team.
 *
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

/** @file StdFrame.hpp
 *
 * @brief Class CXXR::StdFrame.
 */

#ifndef STDFRAME_HPP
#define STDFRAME_HPP

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <tr1/unordered_map>
#include "CXXR/Allocator.hpp"
#include "CXXR/BSerializer.hpp"
#include "CXXR/Frame.hpp"
#include "CXXR/Symbol_serialization.hpp"

namespace CXXR {
    /** @brief General-purpose implementation of CXXR::Frame.
     */
    class StdFrame : public Frame {
    private:
	typedef
	std::tr1::unordered_map<const Symbol*, Binding,
				std::tr1::hash<const Symbol*>,
				std::equal_to<const Symbol*>,
				CXXR::Allocator<std::pair<const Symbol* const,
							  Binding> >
	                        > map;
    public:
	/**
	 * @param initial_capacity A hint to the implementation that
	 *          the constructed StdFrame should be
	 *          configured to have capacity for at least \a
	 *          initial_capacity Bindings.  This does not impose an
	 *          upper limit on the capacity of the StdFrame,
	 *          but some reconfiguration (and consequent time
	 *          penalty) may occur if it is exceeded.
	 */
	explicit StdFrame(size_t initial_capacity = 15);
	// Why 15?  Because if the implementation uses a prime number
	// hash table sizing policy, this will result in the
	// allocation of a hash table array comprising 31 buckets.  On
	// a 32-bit architecture, this will fit well into two 64-byte
	// cache lines.

	// Virtual functions of Frame (qv):
	PairList* asPairList() const;
	Binding* binding(const Symbol* symbol);
	const Binding* binding(const Symbol* symbol) const;
	void clear();
	bool erase(const Symbol* symbol);
	void import(const Frame* frame);
	void lockBindings();
	size_t numBindings() const;
	Binding* obtainBinding(const Symbol* symbol);
	size_t size() const;
	std::vector<const Symbol*> symbols(bool include_dotsymbols) const;

	// Virtual function of GCNode:
	void visitReferents(const_visitor* v) const;
    protected:
	// Virtual function of GCNode:
	void detachReferents();
    private:
	friend class boost::serialization::access;

	template<class Archive>
	void load(Archive & ar, const unsigned int verison) {
	    ar >> boost::serialization::base_object<Frame>(*this);
	    size_t numberOfBindings;
	    ar >> numberOfBindings;
	    for (size_t i=0; i<numberOfBindings; i++ ) {
		const Symbol* symbol=static_cast<Symbol*>(loadSymbol(ar));
		const Binding* binding;
		ar >> binding;
		m_map.insert(map::value_type(symbol, *binding));
		delete binding;
	    }
	}
	
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
	    ar << boost::serialization::base_object<Frame>(*this);
	    size_t numberOfBindings = size();
	    ar << numberOfBindings;
	    for (map::const_iterator it=m_map.begin();
	         it!=m_map.end();
		 ++it) {
		const Symbol* symbol=(*it).first;
		const Binding* binding=&(*it).second;
		saveSymbol(ar, symbol);
		ar << binding;
	    }
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
	    BSerializer::Frame frame("StdFrame");
	    boost::serialization::split_member(ar, *this, version);
	}

	map m_map;

	// Declared private to ensure that StdFrame objects are
	// created only using 'new':
	~StdFrame() {}

	// Not (yet) implemented.  Declared to prevent
	// compiler-generated versions:
	StdFrame(const Frame&);
	StdFrame& operator=(const Frame&);
    };
}  // namespace CXXR

// Export class as serializable with boost::serialization
BOOST_CLASS_EXPORT(CXXR::StdFrame)

#endif // STDFRAME_HPP
