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

/** @file Symbol.h
 * @brief Class CXXR::Symbol and associated C interface.
 */

#ifndef RSYMBOL_H
#define RSYMBOL_H

#include "CXXR/RObject.h"

#ifdef __cplusplus

#include "CXXR/BuiltInFunction.h"
#include "CXXR/GCRoot.h"
#include "CXXR/SEXP_downcast.hpp"
#include "CXXR/CachedString.h"

namespace CXXR {
    /** @brief Class used to represent R symbols.
     *
     * A Symbol is an R identifier.  Each Symbol (except for special
     * symbols, see below) has a name, namely a CachedString giving
     * the textual representation of the identifier.  Generally
     * speaking, however, a Symbol object is identified by its address
     * rather than by its name.  Consequently, the class enforces the
     * invariant that there is a most one Symbol object with a given
     * name (but this does not apply to special symbols).
     *
     * Symbols come in two varieties, standard symbols and special
     * symbols, both implemented by this class.  Dot-dot symbols are a
     * subvariety of standard symbols.
     *
     * Standard symbols are generated using the static member function
     * obtain(), and (as explained above) have the property that there
     * is at most one standard symbol with a given name.  This is
     * enforced by an internal table mapping names to standard
     * symbols.
     *
     * Dot-dot symbols have names of the form '<tt>..</tt><i>n</i>',
     * where <i>n</i> is a positive integer.  These are preferably
     * generated using the static member function obtainDotDotSymbol()
     * (though they can also be generated using obtain() ), and are
     * used internally by the interpreter to refer to elements of a
     * '<tt>...</tt>' argument list.  (Note that CR does not
     * consistently enforce the 'at most one Symbol per name' rule for
     * dot-dot symbols; CXXR does.)
     *
     * Special symbols are used to implement certain pseudo-objects
     * (::R_MissingArg, ::R_RestartToken and ::R_UnboundValue) that CR
     * expects to have ::SEXPTYPE SYMSXP.  Each special symbol has a
     * blank string as its name, but despite this each of them is a
     * distinct symbol.
     *
     * @note Following the practice with CR's symbol table, Symbol
     * objects, once created, are permanently preserved against
     * garbage collection.  There is no inherent reason for this in
     * CXXR, but some packages may rely on it.
     */
    class Symbol : public RObject {
    private:
	// A table is used to ensure that, for standard symbols,
	// there is at most one Symbol object with a particular name.
	typedef
	std::tr1::unordered_map<const CachedString*, GCRoot<Symbol>,
				std::tr1::hash<const CachedString*>,
				std::equal_to<const CachedString*>,
				CXXR::Allocator<std::pair<const CachedString* const,
							  GCRoot<Symbol> > >
	                        > map;
    public:
	// It is assumed that this dereferences to
	// const std::pair<const CachedString*, Symbol*>.
	typedef map::const_iterator const_iterator;

	static const_iterator begin()
	{
	    return s_table->begin();
	}

	static const_iterator end()
	{
	    return s_table->end();
	}

	/** @brief Is this a double-dot symbol?
	 *
	 * @return true iff this symbol relates to an element of a
	 *         <tt>...</tt> argument list.
	 */
	bool isDotDotSymbol() const
	{
	    return m_dd_symbol;
	}

	/** @brief Missing argument.
	 *
	 * @return a pointer to the 'missing argument' pseudo-object.
	 */
	static Symbol* missingArgument()
	{
	    return *s_missing_arg;
	}

	/** @brief Access name.
	 *
	 * @return const reference to the name of this Symbol.
	 */
	const CachedString* name() const
	{
	    return (m_name ? m_name : CachedString::blank());
	}

	/** @brief Get a pointer to a regular Symbol object.
	 *
	 * If no Symbol with the specified name currently exists, one
	 * will be created, and a pointer to it returned.  Otherwise a
	 * pointer to the existing Symbol will be returned.
	 *
	 * @param name The name of the required Symbol.  At present no
	 *          check is made that the supplied string is a valid
	 *          symbol name.
	 *
	 * @return Pointer to a Symbol (preexisting or newly
	 * created) with the required name.
	 */
	static Symbol* obtain(const CachedString* name);

	/** @brief Get a pointer to a regular Symbol object.
	 *
	 * If no Symbol with the specified name currently exists, one
	 * will be created, and a pointer to it returned.  Otherwise a
	 * pointer to the existing Symbol will be returned.
	 *
	 * @param name The name of the required Symbol (CE_NATIVE
	 *          encoding is assumed).  At present no check is made
	 *          that the supplied string is a valid symbol name.
	 *
	 * @return Pointer to a Symbol (preexisting or newly
	 * created) with the required name.
	 */
	static Symbol* obtain(const std::string& name);

	/** @brief Create a double-dot symbol.
	 *
	 * @param n Index number of the required symbol; must be
	 *          strictly positive.
	 *
	 * @return a pointer to the created symbol, whose name will be
	 * <tt>..</tt><i>n</i>.
	 */
	static Symbol* obtainDotDotSymbol(unsigned int n);

	/** @brief Restart token.
	 *
	 * @return a pointer to the 'restart token' pseudo-object.
	 */
	static Symbol* restartToken()
	{
	    return *s_restart_token;
	}

	/** @brief The name by which this type is known in R.
	 *
	 * @return The name by which this type is known in R.
	 */
	static const char* staticTypeName()
	{
	    return "symbol";
	}

	/** @brief Unbound value.
	 *
	 * This is used as the 'value' of a Symbol that has not been
	 * assigned any actual value.
	 *
	 * @return a pointer to the 'unbound value' pseudo-object.
	 */
	static Symbol* unboundValue()
	{
	    return *s_unbound_value;
	}

	// Virtual function of RObject:
	const char* typeName() const;

	// Virtual function of GCNode:
	void visitReferents(const_visitor* v) const;
    protected:
	// Virtual function of GCNode:
	void detachReferents();
    private:
	static map* s_table;
	static GCRoot<Symbol>* s_missing_arg;
	static GCRoot<Symbol>* s_restart_token;
	static GCRoot<Symbol>* s_unbound_value;

	GCEdge<const CachedString> m_name;
	bool m_dd_symbol;

	/**
	 * @param name Pointer to String object representing the name
	 *          of the symbol.  Names of the form
	 *          <tt>..<em>n</em></tt>, where n is a (non-negative)
	 *          decimal integer signify that the Symbol to be
	 *          constructed relates to an element of a
	 *          <tt>...</tt> argument list.  A null pointer
	 *          signifies a special Symbol, which is not entered
	 *          into s_table.
	 *
	 * @param frozen true iff the Symbol should not be altered
	 *          after it is created.
	 */
	explicit Symbol(const CachedString* name = 0, bool frozen = true);

	// Declared private to ensure that Symbol objects are
	// allocated only using 'new':
	~Symbol();

	// Not (yet) implemented.  Declared to prevent
	// compiler-generated versions:
	Symbol(const Symbol&);
	Symbol& operator=(const Symbol&);

	// Free memory used by the static data members:
	static void cleanup();

	// Initialize the static data members:
	static void initialize();

	friend class SchwarzCounter<Symbol>;
    };

    /** @brief Does Symbol's name start with '.'?
     *
     * @param symbol pointer to Symbol to be tested, or a null pointer
     *          in which case the function returns false.
     *
     * @return true if the Symbol's name starts with '.'.
     */
    inline bool isDotSymbol(const Symbol* symbol)
    {
	return symbol && symbol->name()->c_str()[0] == '.';
    }

    /** @brief Does Symbol's name start with '..'?
     *
     * @param symbol pointer to Symbol to be tested, or a null pointer
     *          in which case the function returns false.
     *
     * @return true if the Symbol's name starts with '..'.
     */
    inline bool isDotDotSymbol(const Symbol* symbol)
    {
	return symbol && symbol->isDotDotSymbol();
    }

    // Predefined Symbols visible in 'namespace CXXR':
    extern Symbol* const Bracket2Symbol;   // "[["
    extern Symbol* const BracketSymbol;    // "["
    extern Symbol* const BraceSymbol;      // "{"
    extern Symbol* const ClassSymbol;	   // "class"
    extern Symbol* const DimNamesSymbol;   // "dimnames"
    extern Symbol* const DimSymbol;	   // "dim"
    extern Symbol* const DollarSymbol;	   // "$"
    extern Symbol* const DotsSymbol;	   // "..."
    extern Symbol* const DropSymbol;	   // "drop"
    extern Symbol* const ExactSymbol;      // "exact"
    extern Symbol* const LevelsSymbol;	   // "levels"
    extern Symbol* const ModeSymbol;	   // "mode"
    extern Symbol* const NamesSymbol;	   // "names"
    extern Symbol* const NaRmSymbol;       // "ra.rm"
    extern Symbol* const RowNamesSymbol;   // "row.names"
    extern Symbol* const SeedsSymbol;	   // ".Random.seed"
    extern Symbol* const LastvalueSymbol;  // ".Last.value"
    extern Symbol* const TspSymbol;	   // "tsp"
    extern Symbol* const CommentSymbol;    // "comment"
    extern Symbol* const SourceSymbol;     // "source"
    extern Symbol* const DotEnvSymbol;     // ".Environment"
    extern Symbol* const RecursiveSymbol;  // "recursive"
    extern Symbol* const SrcfileSymbol;    // "srcfile"
    extern Symbol* const SrcrefSymbol;     // "srcref"
    extern Symbol* const TmpvalSymbol;     // "*tmp*"
    extern Symbol* const UseNamesSymbol;   // "use.names"
}  // namespace CXXR

namespace {
    CXXR::SchwarzCounter<CXXR::Symbol> symbol_schwarz_ctr;
}

extern "C" {
#endif

    /* Pseudo-objects */
    extern SEXP R_MissingArg;
    extern SEXP R_RestartToken;
    extern SEXP R_UnboundValue;

    /* Symbol Table Shortcuts */
    extern SEXP R_Bracket2Symbol;  /* "[[" */
    extern SEXP R_BracketSymbol;   /* "[" */
    extern SEXP R_BraceSymbol;     /* "{" */
    extern SEXP R_ClassSymbol;	   /* "class" */
    extern SEXP R_DimNamesSymbol;  /* "dimnames" */
    extern SEXP R_DimSymbol;	   /* "dim" */
    extern SEXP R_DollarSymbol;	   /* "$" */
    extern SEXP R_DotsSymbol;	   /* "..." */
    extern SEXP R_DropSymbol;	   /* "drop" */
    extern SEXP R_LevelsSymbol;	   /* "levels" */
    extern SEXP R_ModeSymbol;	   /* "mode" */
    extern SEXP R_NamesSymbol;	   /* "names" */
    extern SEXP R_RowNamesSymbol;  /* "row.names" */
    extern SEXP R_SeedsSymbol;	   /* ".Random.seed" */
    extern SEXP R_TspSymbol;	   /* "tsp" */

    /** @brief Does symbol relate to a <tt>...</tt> expression?
     *
     * @param x Pointer to a CXXR::Symbol (checked).
     *
     * @return \c TRUE iff this symbol denotes an element of a
     *         <tt>...</tt> expression.
     */
#ifndef __cplusplus
    Rboolean DDVAL(SEXP x);
#else
    inline Rboolean DDVAL(SEXP x)
    {
	using namespace CXXR;
	const Symbol& sym = *SEXP_downcast<Symbol*>(x);
	return Rboolean(sym.isDotDotSymbol());
    }
#endif

    /** @brief Get a pointer to a regular Symbol object.
     *
     * If no Symbol with the specified name currently exists, one will
     * be created, and a pointer to it returned.  Otherwise a pointer
     * to the existing Symbol will be returned.
     *
     * @param name The name of the required Symbol (CE_NATIVE encoding
     *          is assumed).
     *
     * @return Pointer to a Symbol (preexisting or newly created) with
     * the required name.
     */
    SEXP Rf_install(const char *name);

    /** @brief Test if SYMSXP.
     *
     * @param s Pointer to a CXXR::RObject.
     *
     * @return TRUE iff s points to a CXXR::RObject with ::SEXPTYPE
     *         SYMSXP. 
     */
#ifndef __cplusplus
    Rboolean Rf_isSymbol(SEXP s);
#else
    inline Rboolean Rf_isSymbol(SEXP s)
    {
	return Rboolean(s && TYPEOF(s) == SYMSXP);
    }
#endif

    /** @brief Symbol name.
     *
     * @param x Pointer to a CXXR::Symbol (checked).
     *
     * @return Pointer to a CXXR::CachedString representing \a x's name.
     */
#ifndef __cplusplus
    SEXP PRINTNAME(SEXP x);
#else
    inline SEXP PRINTNAME(SEXP x)
    {
	using namespace CXXR;
	const Symbol& sym = *SEXP_downcast<Symbol*>(x);
	return const_cast<CachedString*>(sym.name());
    }
#endif

#ifdef __cplusplus
}
#endif

#endif /* RSYMBOL_H */
