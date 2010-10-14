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

/** @file UncachedString.cpp
 *
 * Implementation of class CXXR::UncachedString and related functions.
 */

#include <cstring>
#include "CXXR/UncachedString.h"
#include "CXXR/errors.h"

using namespace std;
using namespace CXXR;

namespace CXXR {
    namespace ForceNonInline {
	SEXP (*Rf_mkCharLenp)(const char*, int) = Rf_mkCharLen;
    }
}

UncachedString::UncachedString(const std::string& str, cetype_t encoding,
			       bool frozen)
    : String(str.size(), encoding), m_databytes(str.size() + 1),
      m_data(m_short_string)
{
    size_t sz = str.size();
    allocData(sz);
    memcpy(m_data, str.data(), sz);
    if (frozen) freeze();
}

void UncachedString::allocData(size_t sz)
{
    if (sz > s_short_strlen)
	m_data = static_cast<char*>(MemoryBank::allocate(m_databytes));
    // Insert trailing null byte:
    m_data[sz] = 0;
}

const char* UncachedString::c_str() const
{
    return m_data;
}

const char* UncachedString::typeName() const
{
    return UncachedString::staticTypeName();
}

// ***** C interface *****

SEXP Rf_mkCharLenCE(const char* text, int length, cetype_t encoding)
{
    switch(encoding) {
    case CE_NATIVE:
    case CE_UTF8:
    case CE_LATIN1:
    case CE_SYMBOL:
    case CE_ANY:
	break;
    default:
	Rf_error(_("unknown encoding: %d"), encoding);
    }
    string str(text, length);
    return CXXR_NEW(UncachedString(str, encoding));
}
