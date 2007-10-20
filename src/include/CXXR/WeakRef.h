/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1999-2006   The R Development Core Team.
 *  Andrew Runnalls (C) 2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

/** @file WeakRef.h
 * Class WeakRef and associated C interface.
 */

#ifndef WEAKREF_HPP
#define WEAKREF_HPP

#include "CXXR/RObject.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Finalization interface */
typedef void (*R_CFinalizer_t)(SEXP);
void R_RegisterFinalizer(SEXP s, SEXP fun);
void R_RegisterCFinalizer(SEXP s, R_CFinalizer_t fun);
void R_RegisterFinalizerEx(SEXP s, SEXP fun, Rboolean onexit);
void R_RegisterCFinalizerEx(SEXP s, R_CFinalizer_t fun, Rboolean onexit);

/* Weak reference interface */
SEXP R_MakeWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit);
SEXP R_MakeWeakRefC(SEXP key, SEXP val, R_CFinalizer_t fin, Rboolean onexit);
SEXP R_WeakRefKey(SEXP w);
SEXP R_WeakRefValue(SEXP w);

#ifdef __cplusplus
}  // extern "C"

#include <list>
#include "CXXR/Allocator.hpp"
#include "CXXR/GCEdge.hpp"

namespace CXXR {
    /** Weak reference.
     *
     * Refer to <em>Stretching the storage manager: weak pointers and
     * stable names in Haskell</em> by Peyton Jones, Marlow, and
     * Elliott (at <a
     * href="www.research.microsoft.com/Users/simonpj/papers/weak.ps.gz">www.research.microsoft.com/Users/simonpj/papers/weak.ps.gz</a>)
     * for the motivation and implementation of this class.
     *
     * Each weak reference has a key and, optionally, a value and/or a
     * finalizer.  The finalizer may either be a C function or an R
     * object.  The garbage collector will consider the value and
     * finalizer to be reachable provided the key is reachable.
     *
     * If, during a garbage collection, the key is found not to be
     * reachable then the finalizer (if any) will be run, and the weak
     * reference object will be 'tombstoned', so that subsequent calls
     * to key() and value() will return null pointers.
     *
     * A weak reference object with a reachable key will not be
     * garbage collected even if the weak reference object is not
     * itself reachable.
     */
    class WeakRef : public RObject {
    public:
	/**
	 * @param key Pointer to the key of the weak reference.
	 *
	 *          It is not forbidden but probably pointless for the
	 *          key to be null: in this event the reference will
	 *          immediately be tombstoned, and its finalizer (if
	 *          any) will never be run.
	 *
	 * @param value Pointer to the value of the weak reference
	 *          (may be null)
	 *
	 * @param R_finalizer Pointer to an R object to be evaluated
	 *          as a finalizer (may be null).  The finalizer will
	 *          be called with the key of the weak reference
	 *          object as its argument, and at the time of call
	 *          the key and finalizer will be protected from the
	 *          garbage collector.  However, the weak reference
	 *          object itself will already have been tombstoned.
	 *
	 * @param finalize_on_exit True iff the finalizer should be
	 *          run when CXXR exits.
	 */
	WeakRef(RObject* key, RObject* value, RObject* R_finalizer = 0,
		bool finalize_on_exit = false);

	/**
	 * @param key Pointer to the key of the weak reference.
	 *
	 *          It is not forbidden but probably pointless for the
	 *          key to be null: in this event the reference will
	 *          immediately be tombstoned, and its finalizer (if
	 *          any) will never be run.
	 *
	 * @param value Pointer to the value of the weak reference
	 *          (may be null).  The finalizer will be called with
	 *          a pointer to the key of the weak reference object
	 *          as its argument, and at the time of call the key
	 *          object will be protected from the garbage
	 *          collector.  However, the weak reference object
	 *          itself will already have been tombstoned.
	 *
	 * @param C_finalizer Pointer to an C function to be invoked
	 *          as a finalizer (may be null).
	 *
	 * @param finalize_on_exit True iff the finalizer should be
	 *          run when CXXR exits.
	 */
	WeakRef(RObject* key, RObject* value, R_CFinalizer_t C_finalizer,
		bool finalize_on_exit = false);

	~WeakRef();

	/** Integrity check.
	 *
	 * Aborts the program with an error message if the class is
	 * found to be internally inconsistent.
	 *
	 * @return true, if it returns at all.  The return value is to
	 * facilitate use with \c assert.
	 */
	static bool check();

	/**
	 * @return Pointer to the key of the WeakRef.
	 */
	RObject* key() const {return m_key;}

	/**
	 * Run the finalizers of all (non-tombstoned) WeakRef object
	 * for which 'finalize_on_exit' was specified.
	 */
	static void runExitFinalizers();

	/** Run finalizers.
	 *
	 * This is called by GCManager::gc() immediately after garbage
	 * collection, and runs the finalizers of any weak references that
	 * were identified during the garbage collection as being
	 * ready to finalize; when the call exits, all such weak
	 * references will have been tombstoned.  (Consequently,
	 * calling this method at any other time will effectively be a
	 * no-op.)
	 *
	 * @return true iff any finalizers are actually run (whether
	 * successfully or not).
	 */
	static bool runFinalizers();

	/**
	 * @return Pointer to the value of the WeakRef.
	 */
	RObject* value() const {return m_value;}
    private:
	// Flag positions:
	static const unsigned int READY_TO_FINALIZE = 0;
	static const unsigned int FINALIZE_ON_EXIT = 1;

	typedef std::list<WeakRef*, Allocator<WeakRef*> > WRList;
	static WRList s_live;
	static WRList s_f10n_pending;  // Finalization pending
	static WRList s_tombstone;

	static int s_count;  // Count of references in existence (for
			     // debugging)

	RObject* m_key;
	Edge m_value;
	Edge m_Rfinalizer;
	R_CFinalizer_t m_Cfinalizer;
	WRList::iterator m_lit;

	void finalize();

	/** Mark nodes reachable via weak references.
	 *
	 * This function implements the algorithm in Sec. 6.2 of the
	 * Peyton-Jones et al. paper.  If a WeakRef has a marked key,
	 * its value and R finalizer and their descendants are marked.
	 * If the key is not marked, and there is a finalizer, then
	 * the WeakRef is placed on a finalization pending list.  If
	 * the key is not marked and there is no finalizer, the
	 * WeakRef is tombstoned.
	 */
	static void markThru(unsigned int max_gen);

	// Tombstone the node:
	void tombstone();

	// Transfer the WeakRef from list 'from' to list 'to':
	void transfer(WRList* from, WRList* to)
	{
	    to->splice(to->end(), *from, m_lit);
	}

	// Return pointer to the list (s_live, s_f10n_pending or
	// s_tombstone) on which - according to its internal data -
	// the object currently should be listed (and quite possibly
	// is listed).
	WRList* wrList() const;

	friend class GCNode;
    };
}  // namespace CXXR

#endif /* __cplusplus */

#endif /* WEAKREF_HPP */
