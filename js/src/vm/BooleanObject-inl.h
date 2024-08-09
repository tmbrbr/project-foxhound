/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef vm_BooleanObject_inl_h
#define vm_BooleanObject_inl_h

#include "vm/BooleanObject.h"

#include "vm/JSObject-inl.h"

namespace js {

inline BooleanObject* BooleanObject::create(
    JSContext* cx, bool b, HandleObject proto /* = nullptr */) {
  BooleanObject* obj = NewObjectWithClassProto<BooleanObject>(cx, proto);
  if (!obj) {
    return nullptr;
  }
  obj->setPrimitiveValue(b);
    // Taintfox: initialize the taint slot to null
  obj->initReservedSlot(TAINT_SLOT, PrivateValue(nullptr));

  return obj;
}



inline BooleanObject*
BooleanObject::createTainted(JSContext* cx, bool b, const TaintFlow& taint, HandleObject proto /* = nullptr */)
{
  BooleanObject* obj = create(cx, b, proto);
  if (!obj) {
    return nullptr;
  }
  obj->setTaint(taint);
  return obj;
}

inline void BooleanObject::finalize(JS::GCContext* gcx, JSObject* obj) {
  MOZ_ASSERT(gcx->onMainThread());
  BooleanObject* numobj = static_cast<BooleanObject*>(obj);
  if (obj) {
    TaintFlow* flow = numobj->getTaintFlow();
    if (flow) {
      delete flow;
      numobj->setReservedSlot(TAINT_SLOT, PrivateValue(nullptr));
    }
  }
}


inline void BooleanObject::sweepAfterMinorGC(JS::GCContext* gcx, BooleanObject *obj) {
  bool wasInsideNursery = IsInsideNursery(obj);
  if (wasInsideNursery && !IsForwarded(obj)) {
    finalize(gcx, obj);
    return;
  }
}

}  // namespace js

#endif /* vm_BooleanObject_inl_h */
