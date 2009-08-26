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

/** @file CellPool.hpp
 *
 * @brief Class CXXR::CellPool.
 */

#ifndef CELLPOOL_HPP
#define CELLPOOL_HPP

#include <cstddef>
#include <new>
#include <vector>

#include "Rvalgrind.h"

namespace CXXR {
    /** Class to manage a pool of memory cells of a fixed size.
     * 
     * This class, based closely on Item 10 of Scott Meyers' 'Effective
     * C++ (2nd edition)' manages a collection of memory cells of a
     * specified size, and is intended as a back-end to implementations of
     * operator new and operator delete to enable the allocation and
     * deallocation of small objects quickly.
     *
     * Normally class CellPool operates a last-in-first-out allocation
     * policy; that is to say, if there are cells that have been
     * deallocated and not yet reallocated, the next one to be
     * reallocated will be the one that was most recently
     * deallocated.  This makes for efficient utilisation of the
     * processor caches.  However, if the class is compiled with the
     * preprocessor variable CELLFIFO defined, it instead operates a
     * first-in-first-out policy.  This results in a longer turnround
     * time in reallocating cells, which improves the lethality of the
     * FILL55 preprocessor variable in showing up premature
     * deallocation of cells.
     */
    class CellPool {
    public:
	/**
	 * @param dbls_per_cell (must be >= 1). Size of cells,
	 *         expressed as a multiple of sizeof(double).  For
	 *         example, if you require cells large enough to
	 *         contain one double, put dbls_per_cell as 1.  (NB:
	 *         cells can contain anything, not just doubles; we
	 *         work in doubles because these are likely to have
	 *         the most stringent address alignment requirements.)
	 *
	 * @param cells_per_superblock (must be >= 1).  Memory for cells is
	 *         obtained from the main heap in 'superblocks'
	 *         sufficient to contain this many cells.
	 */
	CellPool(size_t dbls_per_cell, size_t cells_per_superblock)
	    : m_cellsize(dbls_per_cell*sizeof(double)),
	      m_cells_per_superblock(cells_per_superblock),
	      m_superblocksize(m_cellsize*cells_per_superblock),
	      m_free_cells(0),
#ifdef CELLFIFO
	      m_last_free_cell(0),
#endif
	      m_cells_allocated(0)
	{
#if VALGRIND_LEVEL >= 2
	    VALGRIND_CREATE_MEMPOOL(this, 0, 0);
#endif
	}

	/** Destructor
	 *
	 * It is up to the user to check that any cells allocated from
	 * the pool have been freed before this destructor is
	 * invoked.  (Although the destructor could check this for
	 * itself and issue an error message, this message would
	 * probably be a nuisance if it occurred during program shutdown.)
	 */
        ~CellPool();

	/**
	 * @brief Allocate a cell from the pool.
	 *
	 * @return a pointer to the allocated cell.
	 *
	 * @throws bad_alloc if a cell cannot be allocated.
	 */
	void* allocate() throw (std::bad_alloc)
	{
	    if (!m_free_cells) seekMemory();
	    Cell* c = m_free_cells;
	    m_free_cells = c->m_next;
#ifdef CELLFIFO
	    if (!m_free_cells)
		m_last_free_cell = 0;
#endif
	    ++m_cells_allocated;
#if VALGRIND_LEVEL >= 2
	    VALGRIND_MEMPOOL_ALLOC(this, c, cellSize());
#endif
	    return c;
	}

	/**
	 * @return the size of each cell in bytes (well, strictly as a
	 * multiple of sizeof(char)).
	 */         
	size_t cellSize() const  {return m_cellsize;}

	/**
	 * @return the number of cells currently allocated from this
	 * pool.
	 */
	unsigned int cellsAllocated() const {return m_cells_allocated;}

	/** @brief Integrity check.
	 *
	 * Aborts the program with an error message if the object is
	 * found to be internally inconsistent.
	 *
	 * @return true, if it returns at all.  The return value is to
	 * facilitate use with \c assert .
	 */
	bool check() const;

	/** @brief Deallocate a cell
	 *
	 * @param p Pointer to a block of memory previously allocated
	 * from this pool, or a null pointer (in which case method
	 * does nothing).
	 */
	void deallocate(void* p)
	{
	    if (!p) return;
#ifdef DEBUG_RELEASE_MEM
	    checkAllocatedCell(p);
#endif
#if VALGRIND_LEVEL >= 2
	    VALGRIND_MEMPOOL_FREE(this, p);
	    VALGRIND_MAKE_MEM_UNDEFINED(p, sizeof(Cell));
#endif
	    Cell* c = static_cast<Cell*>(p);
#ifdef CELLFIFO
	    c->m_next = 0;
	    if (!m_free_cells)
		m_free_cells = m_last_free_cell = c;
	    else {
		m_last_free_cell->m_next = c;
		m_last_free_cell = c;
	    }
#else
	    c->m_next = m_free_cells;
	    m_free_cells = c;
#endif
	    --m_cells_allocated;
	}

	/** @brief Reorganise list of free cells within the CellPool.
	 *
	 * This is done with a view to increasing the probability that
	 * successive allocations will lie within the same cache line
	 * or (less importantly nowadays) memory page.
	 */
	void defragment();

	/** @brief Allocate a cell 'from stock'.
	 *
	 * Allocate a cell from the pool, provided it can be allocated
	 * 'from stock'.  Can be useful when called from other inlined
	 * functions in that it doesn't throw any exceptions.
	 *
	 * @return a pointer to the allocated cell, or 0 if the cell
	 * cannot be allocated from the current memory superblocks.
	 */
	void* easyAllocate() throw ()
	{
	    if (!m_free_cells) return 0;
	    Cell* c = m_free_cells;
	    m_free_cells = c->m_next;
	    ++m_cells_allocated;
#if VALGRIND_LEVEL >= 2
	    VALGRIND_MEMPOOL_ALLOC(this, c, cellSize());
#endif
	    return c;
	}

	/**
	 * @return The size in bytes of the superblocks from which
	 *         cells are allocated.
	 */
	size_t superblockSize() const {return m_superblocksize;}
    private:
	struct Cell {
	    Cell* m_next;

	    Cell(Cell* next = 0) : m_next(next) {}
	};

	const size_t m_cellsize;
	const size_t m_cells_per_superblock;
	const size_t m_superblocksize;
	std::vector<void*> m_superblocks;
	Cell* m_free_cells;
#ifdef CELLFIFO
	Cell* m_last_free_cell;
#endif
	unsigned int m_cells_allocated;

	// Checks that p is either null or points to a cell belonging
	// to this pool; aborts if not.
	void checkCell(const void* p) const;

	// Calls checkCell, and further checks that the cell is not on
	// the free list:
	void checkAllocatedCell(const void* p) const;

	void seekMemory() throw (std::bad_alloc);
    };
}

#endif /* CELLPOOL_HPP */
