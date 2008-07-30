/*
 * Copyright (c) 2003-2008, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SCOPE_H
#define _SCOPE_H

#include "expr.h"
#include "op.h"

namespace ledger {

class scope_t
{
public:
  explicit scope_t() {
    TRACE_CTOR(scope_t, "");
  }
  virtual ~scope_t() {
    TRACE_DTOR(scope_t);
  }

  virtual expr_t::ptr_op_t lookup(const string& name) = 0;

  value_t resolve(const string& name) {
    expr_t::ptr_op_t definition = lookup(name);
    if (definition)
      return definition->calc(*this);
    else
      return NULL_VALUE;
  }
};

class child_scope_t : public noncopyable, public scope_t
{
public:
  scope_t * parent;

  explicit child_scope_t() : parent(NULL) {
    TRACE_CTOR(child_scope_t, "");
  }
  explicit child_scope_t(scope_t& _parent)
    : parent(&_parent) {
    TRACE_CTOR(child_scope_t, "scope_t&");
  }
  virtual ~child_scope_t() {
    TRACE_DTOR(child_scope_t);
  }

  virtual expr_t::ptr_op_t lookup(const string& name) {
    if (parent)
      return parent->lookup(name);
    return expr_t::ptr_op_t();
  }
};

class symbol_scope_t : public child_scope_t
{
  typedef std::map<const string, expr_t::ptr_op_t> symbol_map;

  symbol_map symbols;

public:
  explicit symbol_scope_t() {
    TRACE_CTOR(symbol_scope_t, "");
  }
  explicit symbol_scope_t(scope_t& _parent) : child_scope_t(_parent) {
    TRACE_CTOR(symbol_scope_t, "scope_t&");
  }
  virtual ~symbol_scope_t() {
    TRACE_DTOR(symbol_scope_t);
  }

  void define(const string& name, const value_t& val) {
    define(name, expr_t::op_t::wrap_value(val));
  }
  void define(const string& name, const function_t& func) {
    define(name, expr_t::op_t::wrap_functor(func));
  }
  virtual void define(const string& name, expr_t::ptr_op_t def);

  virtual expr_t::ptr_op_t lookup(const string& name);
};

class call_scope_t : public child_scope_t
{
  value_t args;

  call_scope_t();

public:
  explicit call_scope_t(scope_t& _parent) : child_scope_t(_parent) {
    TRACE_CTOR(call_scope_t, "scope_t&");
  }
  virtual ~call_scope_t() {
    TRACE_DTOR(call_scope_t);
  }

  void set_args(const value_t& _args) {
    args = _args;
  }
  value_t& value() {
    return args;
  }

  value_t& operator[](const unsigned int index) {
    // jww (2008-07-21): exception here if it's out of bounds
    return args[index];
  }
  const value_t& operator[](const unsigned int index) const {
    // jww (2008-07-21): exception here if it's out of bounds
    return args[index];
  }

  void push_back(const value_t& val) {
    args.push_back(val);
  }
  void pop_back() {
    args.pop_back();
  }

  const std::size_t size() const {
    return args.size();
  }
};

template <typename T>
class var_t : public noncopyable
{
  T * value;

  var_t();

public:
  // jww (2008-07-21): Give a good exception here if we can't find "name"
  var_t(scope_t& scope, const string& name)
    : value(scope.resolve(name).template as_pointer<T>()) {
    TRACE_CTOR(var_t, "scope_t&, const string&");
  }
  var_t(call_scope_t& scope, const unsigned int idx)
    : value(scope[idx].template as_pointer<T>()) {
    TRACE_CTOR(var_t, "call_scope_t&, const unsigned int");
  }
  ~var_t() throw() {
    TRACE_DTOR(var_t);
  }

  T& operator *() { return *value; }
  T * operator->() { return value; }
};

} // namespace ledger

#endif // _SCOPE_H
