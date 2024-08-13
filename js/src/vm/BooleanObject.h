/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef vm_BooleanObject_h
#define vm_BooleanObject_h

#include <iostream>

#include "Taint.h"

#include "builtin/Boolean.h"
#include "vm/NativeObject.h"

namespace js {

class BooleanObject : public NativeObject {
  /* Stores this Boolean object's [[PrimitiveValue]]. */
  static const unsigned PRIMITIVE_VALUE_SLOT = 0;

  /* Taintfox: Stores the Boolean object's taint information */
  static const unsigned TAINT_SLOT = 1;

  static const ClassSpec classSpec_;

 public:
  static const unsigned RESERVED_SLOTS = 2;

  static const JSClass class_;

  /*
   * Creates a new Boolean object boxing the given primitive bool.
   * If proto is nullptr, the [[Prototype]] will default to Boolean.prototype.
   */
  static inline BooleanObject* create(JSContext* cx, bool b,
                                      HandleObject proto = nullptr);

  bool unbox() const { return getFixedSlot(PRIMITIVE_VALUE_SLOT).toBoolean(); }

  static inline BooleanObject* createTainted(JSContext* cx, bool b,
                                            const TaintFlow& taint,
                                            HandleObject proto = nullptr);

  // TaintFox: A finalizer is required for correct memory handling.
  static void finalize(JS::GCContext* gcx, JSObject* obj);

  static void sweepAfterMinorGC(JS::GCContext* gcx, BooleanObject* numobj);

  const TaintFlow& taint() const {
    TaintFlow* flow = getTaintFlow();
    if (flow) {
      // head->addref();
      return *flow;
    }
    return TaintFlow::getEmptyTaintFlow();
  }

  void setTaint(const TaintFlow& taint) {
    TaintFlow* flow = getTaintFlow();
    if (flow) {
      delete flow;
    }
    setTaintFlow(taint);
  }

  bool isTainted() const {
    return !!getTaintNode();
  }

 private:
  static JSObject* createPrototype(JSContext* cx, JSProtoKey key);

  inline void setPrimitiveValue(bool b) {
    setFixedSlot(PRIMITIVE_VALUE_SLOT, BooleanValue(b));
  }

  inline TaintNode** getTaintNodeImpl() const {
    return maybePtrFromReservedSlot<TaintNode*>(TAINT_SLOT);
  }

  inline TaintNode* getTaintNode() const {
    TaintNode** n = getTaintNodeImpl();
    if (!n) {
      return nullptr;
    }
    return *n;
  }

  inline void setTaintNode(TaintNode* node) {
    TaintNode** n = getTaintNodeImpl();
    if (n != nullptr) {
      *n = node;
    }
  }

  inline TaintFlow* getTaintFlow() const {
    TaintFlow* n = maybePtrFromReservedSlot<TaintFlow>(TAINT_SLOT);
    return n;
  }

  inline void setTaintFlow(const TaintFlow& flow) {
    setReservedSlot(TAINT_SLOT, PrivateValue(new TaintFlow(flow)));
  }
};

}  // namespace js

#endif /* vm_BooleanObject_h */
