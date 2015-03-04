/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2014 and onwards the CXXR Project Authors.
 *
 *  CXXR is not part of the R project, and bugs and other issues should
 *  not be reported via r-bugs or other R project channels; instead refer
 *  to the CXXR website.
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

#ifndef CXXR_JIT_COMPILED_FRAME_HPP
#define CXXR_JIT_COMPILED_FRAME_HPP

#include "CXXR/Frame.hpp"
#include "CXXR/GCEdge.hpp"
#include "CXXR/jit/FrameDescriptor.hpp"
#include <map>

namespace CXXR {
class Symbol;

namespace JIT {
class FrameDescriptor;

/*
 * A CompiledFrame is a frame which stores the bindings for the symbols in the
 * FrameDescriptor in a known location, for efficient lookup.
 * Symbols that are not in the FrameDescriptor get overflowed to a table on the
 * side.
 */
class CompiledFrame : public Frame {
public:
    explicit CompiledFrame(const FrameDescriptor* descriptor);
    CompiledFrame(const CompiledFrame& pattern);
    ~CompiledFrame() override;

    // Binding must exist, or returns null.
    Binding* binding(int location)
    {
    	assert(location >= 0);
    	assert(location < m_descriptor->getNumberOfSymbols());
    	Binding* binding = m_bindings + location;
    	if (isSet(*binding)) {
    	    return binding;
    	} else {
    	    return nullptr;
    	}
    }

    const Binding* binding(int location) const
    {
    	return const_cast<CompiledFrame*>(this)->binding(location);
    }

    BindingRange bindingRange() const override;

    Binding* obtainBinding(const Symbol* symbol, int location)
    {
    	assert(location >= 0);
    	assert(location < m_descriptor->getNumberOfSymbols());
    	Binding* binding = m_bindings + location;
    	if (!isSet(*binding)) {
	    initializeBinding(binding, symbol);
	}
	return binding;
    }

    CompiledFrame* clone() const override;

    void lockBindings() override;

    std::size_t size() const override;

    const FrameDescriptor* getDescriptor() const
    {
	return m_descriptor;
    }

    void detachReferents() override;
    void visitReferents(const_visitor* v) const override;

protected:
    void v_clear() override;
    bool v_erase(const Symbol* symbol) override;
    Binding* v_obtainBinding(const Symbol* symbol) override;
    Binding* v_binding(const Symbol* symbol) override;
    const Binding* v_binding(const Symbol* symbol) const override
    {
	return const_cast<CompiledFrame*>(this)->v_binding(symbol);
    }

private:
    Binding* m_bindings;

    GCEdge<const FrameDescriptor> m_descriptor;

    // Used to store any bindings not described in the descriptor.
    // Usually this is nullptr.
    std::map<const Symbol*, Binding>* m_extension;

    static bool isSet(const Binding& binding)
    {
	return binding.frame() != nullptr;
    }

    static void unsetBinding(Binding* binding);

    CompiledFrame& operator=(const CompiledFrame&) = delete;
};

} // namespace JIT
} // namespace CXXR

#endif // CXXR_JIT_COMPILED_FRAME_HPP
