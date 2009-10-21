/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-9 Andrew R. Runnalls, subject to such other
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

/** @file Environment.cpp
 *
 *
 * @brief Implementation of class CXXR:Environment and associated C
 * interface.
 */

#include "CXXR/Environment.h"

#include "R_ext/Error.h"
#include "localization.h"
#include "CXXR/Symbol.h"

using namespace std;
using namespace CXXR;

// Force the creation of non-inline embodiments of functions callable
// from C:
namespace CXXR {
    namespace ForceNonInline {
	SEXP (*ENCLOSp)(SEXP x) = ENCLOS;
	Rboolean (*ENV_DEBUGp)(SEXP x) = ENV_DEBUG;
	Rboolean (*isEnvironmentptr)(SEXP s) = Rf_isEnvironment;
	SEXP (*FRAMEp)(SEXP x) = FRAME;
	void (*SET_ENCLOSp)(SEXP x, SEXP v) = SET_ENCLOS;
	void (*SET_ENV_DEBUGp)(SEXP x, Rboolean v) = SET_ENV_DEBUG;
	void (*SET_SYMVALUEp)(SEXP x, SEXP v) = SET_SYMVALUE;
	SEXP (*SYMVALUEp)(SEXP x) = SYMVALUE;
    }
}

namespace {
    // Used in {,un}packGPBits():
    const unsigned int FRAME_LOCK_MASK = 1<<14;
    const unsigned int GLOBAL_FRAME_MASK = 1<<15;
}

// Predefined Environments:
namespace CXXR {
    const GCRoot<Environment>
    EmptyEnvironment(GCNode::expose(new Environment(0)));

    const GCRoot<Environment>
    BaseEnvironment(GCNode::expose(new Environment(EmptyEnvironment)));

    const GCRoot<Environment>
    GlobalEnvironment(GCNode::expose(new Environment(BaseEnvironment)));

    const GCRoot<Environment>
    BaseNamespace(GCNode::expose(new Environment(GlobalEnvironment,
						 BaseEnvironment->frame())));
}

SEXP R_EmptyEnv = EmptyEnvironment;
SEXP R_BaseEnv = BaseEnvironment;
SEXP R_GlobalEnv = GlobalEnvironment;
SEXP R_BaseNamespace = BaseNamespace;

void Environment::detachReferents()
{
    m_enclosing.detach();
    m_frame.detach();
    RObject::detachReferents();
}

unsigned int Environment::packGPBits() const
{
    unsigned int ans = RObject::packGPBits();
    if (m_locked) ans |= FRAME_LOCK_MASK;
    // if (m_globally_cached) ans |= GLOBAL_FRAME_MASK;
    return ans;
}

const char* Environment::typeName() const
{
    return staticTypeName();
}

void Environment::unpackGPBits(unsigned int gpbits)
{
    RObject::unpackGPBits(gpbits);
    // Be careful with precedence!
    m_locked = ((gpbits & FRAME_LOCK_MASK) != 0);
    // m_globally_cached = ((gpbits & GLOBAL_FRAME_MASK) != 0);
}

void Environment::visitReferents(const_visitor* v) const
{
    const GCNode* enc = m_enclosing;
    const GCNode* frame = m_frame;
    RObject::visitReferents(v);
    if (enc) enc->conductVisitor(v);
    if (frame) frame->conductVisitor(v);
}

// ***** Free-standing functions *****

namespace {
    // Predicate used to test whether a Binding's value is a function.
    class FunctionTester : public unary_function<RObject*, bool> {
    public:
	FunctionTester(const Symbol* symbol)
	    : m_symbol(symbol)
	{}

	bool operator()(const RObject* obj);
    private:
	const Symbol* m_symbol;
    };

    bool FunctionTester::operator()(const RObject* obj)
    {
	if (obj == R_MissingArg)
	    Rf_error(_("argument \"%s\" is missing, with no default"),
		     m_symbol->name()->c_str());
	return FunctionBase::isA(obj);
    }
}

namespace CXXR {
    pair<Environment*, Frame::Binding*>
    findBinding(const Symbol* symbol, Environment* env)
    {
	while (env) {
	    Frame::Binding* bdg = env->frame()->binding(symbol);
	    if (bdg)
		return make_pair(env, bdg);
	    env = env->enclosingEnvironment();
	}
	return pair<Environment*, Frame::Binding*>(0, 0);
    }

    pair<const Environment*, const Frame::Binding*>
    findBinding(const Symbol* symbol, const Environment* env)
    {
	while (env) {
	    const Frame::Binding* bdg = env->frame()->binding(symbol);
	    if (bdg)
		return make_pair(env, bdg);
	    env = env->enclosingEnvironment();
	}
	return pair<const Environment*, const Frame::Binding*>(0, 0);
    }

    pair<Environment*, FunctionBase*>
    findFunction(const Symbol* symbol, Environment* env, bool inherits)
    {
	FunctionTester functest(symbol);
	pair<Environment*, RObject*> pr
	    = findTestedValue(symbol, env, functest, inherits);
	return make_pair(pr.first, static_cast<FunctionBase*>(pr.second));
    }
}
