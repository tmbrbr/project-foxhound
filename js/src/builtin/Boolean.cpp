/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * JS boolean implementation.
 */

#include "builtin/Boolean-inl.h"

#include "jsapi.h"
#include "jstaint.h"
#include "jstypes.h"

#include "jit/InlinableNatives.h"
#include "js/Conversions.h"
#include "js/PropertySpec.h"
#include "util/StringBuffer.h"
#include "vm/BigIntType.h"
#include "vm/GlobalObject.h"
#include "vm/JSContext.h"
#include "vm/JSObject.h"

#include "vm/BooleanObject-inl.h"

using namespace js;

const JSClass BooleanObject::class_ = {
    "Boolean",
    JSCLASS_HAS_RESERVED_SLOTS(2) | JSCLASS_HAS_CACHED_PROTO(JSProto_Boolean),
    JS_NULL_CLASS_OPS, &BooleanObject::classSpec_};

MOZ_ALWAYS_INLINE bool IsBoolean(HandleValue v) {
  return v.isBoolean() || (v.isObject() && v.toObject().is<BooleanObject>());
}

// ES2020 draft rev ecb4178012d6b4d9abc13fcbd45f5c6394b832ce
// 19.4.3 Properties of the Boolean Prototype Object,  thisBooleanValue.
static MOZ_ALWAYS_INLINE bool ThisBooleanValue(HandleValue val) {
  // Step 3, the error case, is handled by CallNonGenericMethod.
  MOZ_ASSERT(IsBoolean(val));

  // Step 1.
  if (val.isBoolean()) {
    return val.toBoolean();
  }

  // Step 2.
  return val.toObject().as<BooleanObject>().unbox();
}

MOZ_ALWAYS_INLINE bool bool_toSource_impl(JSContext* cx, const CallArgs& args) {
  bool b = ThisBooleanValue(args.thisv());

  JSStringBuilder sb(cx);
  if (!sb.append("(new Boolean(") || !BooleanToStringBuffer(b, sb) ||
      !sb.append("))")) {
    return false;
  }

  JSString* str = sb.finishString();
  if (!str) {
    return false;
  }

  // TaintFox: Propagate Boolean Taint to String
  if (isTaintedBoolean(args.thisv())) {
    // Atoms cannot be tainted. Atoms are created for ints<6bit and other
    // common strings. If we are dealing with an Atom, wrap it inside a
    // dependent String, which can be tainted.
    if(str->isAtom()){
      str = NewDependentString(cx,str,0,str->length());
    }
    SafeStringTaint newTaint(getBooleanTaint(args.thisv()),str->length());
    str->setTaint(cx, newTaint);
  }


  args.rval().setString(str);
  return true;
}

static bool bool_toSource(JSContext* cx, unsigned argc, Value* vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  return CallNonGenericMethod<IsBoolean, bool_toSource_impl>(cx, args);
}

// ES2020 draft rev ecb4178012d6b4d9abc13fcbd45f5c6394b832ce
// 19.3.3.2 Boolean.prototype.toString ( )
MOZ_ALWAYS_INLINE bool bool_toString_impl(JSContext* cx, const CallArgs& args) {
  // Step 1.
  bool b = ThisBooleanValue(args.thisv());

  JSString* str = BooleanToString(cx, b);

  // TaintFox: Propagate Boolean Taint to String
  if (isTaintedBoolean(args.thisv())) {
    // Atoms cannot be tainted. Atoms are created for ints<6bit and other
    // common strings. If we are dealing with an Atom, wrap it inside a
    // dependent String, which can be tainted.
    if(str->isAtom()){
      str = NewDependentString(cx,str,0,str->length());
    }
    SafeStringTaint newTaint(getBooleanTaint(args.thisv()),str->length());
    str->setTaint(cx, newTaint);
  }

  // Step 2.
  args.rval().setString(str);
  return true;
}

static bool bool_toString(JSContext* cx, unsigned argc, Value* vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  return CallNonGenericMethod<IsBoolean, bool_toString_impl>(cx, args);
}

// ES2020 draft rev ecb4178012d6b4d9abc13fcbd45f5c6394b832ce
// 19.3.3.3 Boolean.prototype.valueOf ( )
MOZ_ALWAYS_INLINE bool bool_valueOf_impl(JSContext* cx, const CallArgs& args) {
  // Step 1.
  // TaintFox: Propagate Boolean Taint to valueOf
  if (isTaintedBoolean(args.thisv())) {
    args.rval().setObject(args.thisv().toObject());
  }else{
    args.rval().setBoolean(ThisBooleanValue(args.thisv()));
  }
  return true;
}

static bool bool_valueOf(JSContext* cx, unsigned argc, Value* vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  return CallNonGenericMethod<IsBoolean, bool_valueOf_impl>(cx, args);
}

static const JSFunctionSpec boolean_methods[] = {
    JS_FN("toSource", bool_toSource, 0, 0),
    JS_FN("toString", bool_toString, 0, 0),
    JS_FN("valueOf", bool_valueOf, 0, 0), JS_FS_END};

// ES2020 draft rev ecb4178012d6b4d9abc13fcbd45f5c6394b832ce
// 19.3.1.1 Boolean ( value )
static bool Boolean(JSContext* cx, unsigned argc, Value* vp) {
  CallArgs args = CallArgsFromVp(argc, vp);

  // Step 1.
  bool b = args.length() != 0 ? JS::ToBoolean(args[0]) : false;

  if (args.isConstructing()) {
    // Steps 3-4.
    RootedObject proto(cx);
    if (!GetPrototypeFromBuiltinConstructor(cx, args, JSProto_Boolean,
                                            &proto)) {
      return false;
    }

    JSObject* obj = BooleanObject::create(cx, b, proto);
    if (!obj) {
      return false;
    }

    // Step 5.
    args.rval().setObject(*obj);
  } else {
    // Step 2.
    args.rval().setBoolean(b);
  }
  return true;
}

static bool
bool_taint_getter(JSContext* cx, unsigned argc, Value* vp)
{
  CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setNull();

  // This will be the case for unboxed booleans. In that case just return null.
  if (!args.thisv().isObject()) {
    return true;
  }

  RootedObject boolean(cx, &args.thisv().toObject());
  if (!boolean->is<BooleanObject>()) {
    return true;
  }

  // Return, if boolean is not tainted
  if (!boolean->as<BooleanObject>().isTainted()) {
    return true;
  }

  const TaintFlow& taint = boolean->as<BooleanObject>().taint();

  RootedObject taint_obj(cx, JS_NewObject(cx, nullptr));
  if (!getTaintFlowObject(cx, taint, taint_obj)) {
    return false;
  }

  args.rval().setObject(*taint_obj);
  return true;
}

static const
JSPropertySpec boolean_taint_properties[] = {
    JS_PSG("taint", bool_taint_getter, JSPROP_PERMANENT),
    JS_PS_END
};

JSObject* BooleanObject::createPrototype(JSContext* cx, JSProtoKey key) {
  BooleanObject* booleanProto =
      GlobalObject::createBlankPrototype<BooleanObject>(cx, cx->global());
  if (!booleanProto) {
    return nullptr;
  }
  booleanProto->setFixedSlot(BooleanObject::PRIMITIVE_VALUE_SLOT,
                             BooleanValue(false));
  return booleanProto;
}

const ClassSpec BooleanObject::classSpec_ = {
    GenericCreateConstructor<Boolean, 1, gc::AllocKind::FUNCTION,
                             &jit::JitInfo_Boolean>,
    BooleanObject::createPrototype,
    nullptr,
    nullptr,
    boolean_methods,
    boolean_taint_properties};

PropertyName* js::BooleanToString(JSContext* cx, bool b) {
  return b ? cx->names().true_ : cx->names().false_;
}

JS_PUBLIC_API bool js::ToBooleanSlow(HandleValue v) {
  if (v.isString()) {
    return v.toString()->length() != 0;
  }
  if (v.isBigInt()) {
    return !v.toBigInt()->isZero();
  }
#ifdef ENABLE_RECORD_TUPLE
  // proposal-record-tuple Section 3.1.1
  if (v.isExtendedPrimitive()) {
    return true;
  }
#endif

  // TaintFox: ensures we return the value of the Boolean object
  if (v.isObject() && v.toObject().is<BooleanObject>()) {
    BooleanObject& boolean = v.toObject().as<BooleanObject>();
    return boolean.unbox();
  }

  MOZ_ASSERT(v.isObject());
  return !EmulatesUndefined(&v.toObject());
}

/*
 * TaintFox: boolean.tainted() implementation.
 */
bool
js::bool_tainted(JSContext* cx, unsigned argc, Value* vp)
{
  CallArgs args = CallArgsFromVp(argc, vp);

  if (!IsBoolean(args.get(0))) {
      return false;
  }

  bool b = ToBoolean(args.get(0));
  TaintOperation op("manual taint source", { taintarg(cx, b)});
  op.setSource();
  JSObject* boolean = BooleanObject::createTainted(cx, b, TaintFlow(op));

  args.rval().setObject(*boolean);

  return true;
}
