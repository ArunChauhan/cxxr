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

/** @file Test of class ArgMatcher.
 */

#include "CXXR/ArgMatcher.hpp"

#define R_INTERFACE_PTRS

#include <algorithm>
#include <fstream>
#include <iostream>
#include "boost/regex.hpp"

// For Rf_InitOptions():
#include "Defn.h"

// Inclusion of Defn.h must precede this:
#include "Rinterface.h"

// Otherwise expanded to Rf_match:
#undef match

#include "CXXR/CachedString.h"
#include "CXXR/GCStackRoot.h"
#include "CXXR/PairList.h"
#include "CXXR/Symbol.h"

using namespace std;
using namespace CXXR;

extern "C" {
    void WriteConsoleEx(const char* buf, int, int)
    {
	cout << buf << endl;
    }
}

namespace {
    Environment *fenv, *senv;

    string kv_regex_string("([\\w\\.]*)\\s*:\\s*(\\w*)");
    boost::basic_regex<char> kv_regex(kv_regex_string);

    PairList* getArgs(const char* filename, bool names_as_symbols)
    {
	ifstream astrm(filename);
	if (!astrm) {
	    cerr << "Cannot open file " << filename << '\n';
	    exit(1);
	}
	typedef vector<pair<string, string> > Vps;
	Vps keyvals;
	// Read key-value pairs:
	{
	    string line;
	    while (getline(astrm, line)) {
		boost::smatch kv_match;
		if (!boost::regex_match(line, kv_match, kv_regex)) {
		    cerr << "Lines must match " << kv_regex_string << '\n';
		    exit(1);
		}
		cout << kv_match[1] << " : " << kv_match[2] << endl;
		keyvals.push_back(make_pair(kv_match[1], kv_match[2]));
	    }
	    if (!astrm.eof()) {
		cerr << "Read error in " << filename << '\n';
		exit(1);
	    }
	}
	// Construct result:
	{
	    PairList* ans = 0;
	    for (Vps::const_reverse_iterator rit = keyvals.rbegin();
		 rit != keyvals.rend(); ++rit) {
		const string& namestr = (*rit).first;
		const string& valstr = (*rit).second;
		RObject* value;
		if (valstr.empty())
		    value = Symbol::missingArgument();
		else value
		    = const_cast<CachedString*>(CachedString::obtain(valstr.c_str()));
		const RObject* tag = 0;
		if (!namestr.empty()) {
		    if (names_as_symbols)
			tag = Symbol::obtain(namestr.c_str());
		    else tag = CachedString::obtain(namestr.c_str());
		}
		ans = PairList::construct(value, ans,
					  const_cast<RObject*>(tag));
	    }
	    return ans;
	}
    }

    string getString(const RObject* value)
    {
	const CachedString* str = dynamic_cast<const CachedString*>(value);
	if (!str) {
	    cerr << "CachedString expected.\n";
	    abort();
	}
	return str->stdstring();
    }

    string originString(Frame::Binding::Origin origin)
    {
	static string origstr[] = {"EXPLICIT  ", "MISSING   ", "DEFAULTED "};
	return origstr[origin];
    }

    void showValue(RObject* value);

    void showConsCell(ConsCell* cell)
    {
	const CachedString* tag
	    = SEXP_downcast<const CachedString*>(cell->tag());
	if (tag)
	    cout << tag->c_str() << ' ';
	cout << ": ";
	showValue(cell->car());
    }

    void showValue(RObject* value)
    {
	if (!value)
	    cout << "NULL";
	else {
	    switch (value->sexptype()) {
	    case CHARSXP:
		{
		    cout << getString(value);
		}
		break;
	    case DOTSXP:
		{
		    ConsCell* cell = static_cast<ConsCell*>(value);
		    cout << "DottedArgs(";
		    showConsCell(cell);
		    cell = cell->tail();
		    while (cell) {
			cout << ", ";
			showConsCell(cell);
			cell = cell->tail();
		    }
		    cout << ')';
		}
		break;
	    case PROMSXP:
		{
		    Promise* prom = static_cast<Promise*>(value);
		    cout << "Promise("
			 << getString(prom->valueGenerator())
			 << ", ";
		    Environment* env = prom->environment();
		    if (env == fenv)
			cout << "fenv)";
		    else if (env == senv)
			cout << "senv)";
		    else {
			cerr << "Unexpected environment.\n";
			abort();
		    }
		}
		break;
	    case SYMSXP:
		{
		    if (value == Symbol::missingArgument())
			cout << "Symbol::missingArgument()";
		    else {
			Symbol* sym = static_cast<Symbol*>(value);
			cout << "Symbol(" << getString(sym->name()) << ')';
		    }
		}
		break;
	    default:
		cerr << "Unexpected SEXPTYPE.\n";
		abort();
	    }
	}
    }

    void showFrame(const Frame* frame)
    {
	typedef map<const CachedString*, const Frame::Binding*,
	    String::Comparator> FrameMap;
	FrameMap fmap;
	// Get bindings sorted by name:
	{
	    GCStackRoot<PairList> pl(frame->asPairList());
	    while (pl) {
		const Symbol* sym = dynamic_cast<const Symbol*>(pl->tag());
		if (!sym) {
		    cerr << "Binding tag isn't a Symbol.\n";
		    abort();
		}
		fmap[sym->name()] = frame->binding(sym);
		pl = pl->tail();
	    }
	}
	// Write out bindings in name order:
	for (FrameMap::const_iterator it = fmap.begin();
	     it != fmap.end(); ++it) {
	    const Frame::Binding* bdg = (*it).second;
	    string tag = bdg->symbol()->name()->stdstring();
	    RObject* value = bdg->value();
	    cout << originString(bdg->origin()) << tag << " : ";
	    showValue(value);
	    cout << endl;
	}
    }

    void usage(const char* cmd)
    {
	cerr << "Usage: " << cmd
	     << " formal_args_file supplied_args_file [prior_bindings]\n";
	exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4)
	usage(argv[0]);
    // Set up error reporting:
    ptr_R_WriteConsoleEx = WriteConsoleEx;
    Rf_InitOptions();
    // Set up Environments:
    GCStackRoot<Environment> fenvrt(GCNode::expose(new Environment(0)));
    fenv = fenvrt;
    GCStackRoot<Environment> senvrt(GCNode::expose(new Environment(0)));
    senv = senvrt;
    // Process formals:
    cout << "Formal arguments:\n\n";
    GCStackRoot<PairList> formals(getArgs(argv[1], true));
    GCStackRoot<ArgMatcher>
	matcher(GCNode::expose(new ArgMatcher(formals)));
    // Process supplied arguments:
    cout << "\nSupplied arguments:\n\n";
    GCStackRoot<PairList> supplied(getArgs(argv[2], false));
    // Set up frame and prior bindings (if any):
    Frame* frame = fenv->frame();
    if (argc == 4) {
	cout << "\nPrior bindings:\n\n";
	GCStackRoot<PairList> prior_bindings(getArgs(argv[3], true));
	for (PairList* pb = prior_bindings; pb; pb = pb->tail()) {
	    Symbol* tag = static_cast<Symbol*>(pb->tag());
	    Frame::Binding* bdg = frame->obtainBinding(tag);
	    bdg->setValue(pb->car(), Frame::Binding::EXPLICIT);
	}
    }
    // Perform match and show result:
    matcher->match(fenv, supplied, senv);
    cout << "\nMatch result:\n\n";
    showFrame(frame);
    return 0;
}


    
