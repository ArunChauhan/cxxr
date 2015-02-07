/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR
 *CXXR CXXR is Copyright (C) 2008-14 Andrew R. Runnalls, subject to such other
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

#include "gtest/gtest.h"
#include "CXXR/NodeStack.hpp"
#include "CXXR/RObject.h"

namespace CXXR {
namespace {
class DummyIntSxp : public RObject {
 public:
  DummyIntSxp(int id = 0) : RObject(INTSXP), id_(id) {}
  virtual ~DummyIntSxp() {}
  inline int GetId() const { return id_; }

 private:
  int id_;  // Some id to be able to verify the object.
};
}  // namespace

// Make sure we can create a NodeStack object, i.e., check the constructor.
TEST(NodeStackTest, checkCreation) {
  std::unique_ptr<NodeStack> with_zero_size(new NodeStack(0));
  std::unique_ptr<NodeStack> with_nonzero_size(new NodeStack(512));
  EXPECT_NE(with_zero_size, nullptr);
  EXPECT_NE(with_nonzero_size, nullptr);
}

// Try to push some objects onto the stack.
TEST(NodeStackTest, checkPush) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp;
  EXPECT_EQ(0, stack->size());
  stack->push(&dummy_int_sxp);
  EXPECT_EQ(1, stack->size());
  stack->push(&dummy_int_sxp);
  EXPECT_EQ(2, stack->size());
}

// Try a combination of pushes and pops.
TEST(NodeStackTest, checkPushAndPop) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp1(1), dummy_int_sxp2(2);
  {
    // Push a couple of values and pop with topnpop().
    EXPECT_EQ(0, stack->size());
    stack->push(&dummy_int_sxp1);
    stack->push(&dummy_int_sxp2);
    EXPECT_EQ(2, stack->size());
    auto top1 = stack->topnpop();
    EXPECT_EQ(1, stack->size());
    EXPECT_EQ(static_cast<DummyIntSxp *>(top1)->GetId(), 2);
    auto top2 = stack->topnpop();
    EXPECT_EQ(0, stack->size());
    EXPECT_EQ(static_cast<DummyIntSxp *>(top2)->GetId(), 1);
  }
  {
    // Push a couple of values and pop with pop().
    EXPECT_EQ(0, stack->size());
    stack->push(&dummy_int_sxp1);
    stack->push(&dummy_int_sxp2);
    EXPECT_EQ(2, stack->size());
    stack->pop(2);
    EXPECT_EQ(stack->size(), 0);
  }
}

// Try several pushes, exceeding the initially specified stack size
TEST(NodeStackTest, checkSeveralPushesToGrowStack) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp;
  // Push a several pointers.
  for (int i = 0; i < 64; i++) {
    stack->push(&dummy_int_sxp);
    EXPECT_EQ(stack->size(), i+1);
  }
  // Now, pop all of them.
  stack->pop(64);
  EXPECT_EQ(stack->size(), 0);
}

// Read values on the stack as RObject pointers.
TEST(NodeStackTest, checkElementReadAccessAsRObject) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp1(1), dummy_int_sxp2(2), dummy_int_sxp3(3);
  stack->push(&dummy_int_sxp1);
  stack->push(&dummy_int_sxp2);
  stack->push(&dummy_int_sxp3);
  const RObject* elt1 = (*stack)[0];
  const RObject* elt2 = (*stack)[1];
  const RObject* elt3 = (*stack)[2];
  EXPECT_EQ(static_cast<const DummyIntSxp *>(elt1)->GetId(), 1);
  EXPECT_EQ(static_cast<const DummyIntSxp *>(elt2)->GetId(), 2);
  EXPECT_EQ(static_cast<const DummyIntSxp *>(elt3)->GetId(), 3);
}

// Read values on the stack as ElementProxy objects.
TEST(NodeStackTest, checkElementReadAccessWithElementProxy) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp1(1), dummy_int_sxp2(2), dummy_int_sxp3(3);
  stack->push(&dummy_int_sxp1);
  stack->push(&dummy_int_sxp2);
  stack->push(&dummy_int_sxp3);
  NodeStack::ElementProxy elt1 = (*stack)[0];
  NodeStack::ElementProxy elt2 = (*stack)[1];
  NodeStack::ElementProxy elt3 = (*stack)[2];
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt1))->GetId(), 1);
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt2))->GetId(), 2);
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt3))->GetId(), 3);
}

// Change values on the stack using ElementProxy objects.
TEST(NodeStackTest, checkElementWriteAccessWithElementProxy) {
  std::unique_ptr<NodeStack> stack(new NodeStack(16));
  DummyIntSxp dummy_int_sxp1(1), dummy_int_sxp2(2), dummy_int_sxp3(3);
  stack->push(&dummy_int_sxp1);
  stack->push(&dummy_int_sxp2);
  stack->push(&dummy_int_sxp3);
  NodeStack::ElementProxy elt1 = (*stack)[0];
  NodeStack::ElementProxy elt2 = (*stack)[1];
  NodeStack::ElementProxy elt3 = (*stack)[2];
  elt1 = &dummy_int_sxp3;
  elt2 = &dummy_int_sxp3;
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt1))->GetId(), 3);
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt2))->GetId(), 3);
  EXPECT_EQ(static_cast<const DummyIntSxp *>((RObject*)(elt3))->GetId(), 3);
}
}  // namespace CXXR
