/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JS Array interface. */

#ifndef builtin_Array_h
#define builtin_Array_h

#include "mozilla/Attributes.h"

#include "vm/JSObject.h"

namespace js {

namespace jit {
class TrampolineNativeFrameLayout;
}

class ArrayObject;

MOZ_ALWAYS_INLINE bool IdIsIndex(jsid id, uint32_t* indexp) {
  if (id.isInt()) {
    int32_t i = id.toInt();
    MOZ_ASSERT(i >= 0);
    *indexp = uint32_t(i);
    return true;
  }

  if (MOZ_UNLIKELY(!id.isAtom())) {
    return false;
  }

  JSAtom* atom = id.toAtom();
  return atom->isIndex(indexp);
}

// The methods below only create dense boxed arrays.

// Create a dense array with no capacity allocated, length set to 0, in the
// normal (i.e. non-tenured) heap.
extern ArrayObject* NewDenseEmptyArray(JSContext* cx);

// Create a dense array with no capacity allocated, length set to 0, in the
// tenured heap.
extern ArrayObject* NewTenuredDenseEmptyArray(JSContext* cx);

// Create a dense array with a set length, but without allocating space for the
// contents. This is useful, e.g., when accepting length from the user.
extern ArrayObject* NewDenseUnallocatedArray(
    JSContext* cx, uint32_t length, NewObjectKind newKind = GenericObject);

// Create a dense array with length and capacity == 'length', initialized length
// set to 0.
extern ArrayObject* NewDenseFullyAllocatedArray(
    JSContext* cx, uint32_t length, NewObjectKind newKind = GenericObject,
    gc::AllocSite* site = nullptr);

// Create a dense array with length == 'length', initialized length set to 0,
// and capacity == 'length' clamped to EagerAllocationMaxLength.
extern ArrayObject* NewDensePartlyAllocatedArray(
    JSContext* cx, uint32_t length, NewObjectKind newKind = GenericObject);

// Like NewDensePartlyAllocatedArray, but the array will have |proto| as
// prototype (or Array.prototype if |proto| is nullptr).
extern ArrayObject* NewDensePartlyAllocatedArrayWithProto(JSContext* cx,
                                                          uint32_t length,
                                                          HandleObject proto);

// Create a dense array from the given array values, which must be rooted.
extern ArrayObject* NewDenseCopiedArray(JSContext* cx, uint32_t length,
                                        const Value* values,
                                        NewObjectKind newKind = GenericObject);

// Create a dense array from the given (linear)string values, which must be
// rooted
extern ArrayObject* NewDenseCopiedArray(JSContext* cx, uint32_t length,
                                        JSLinearString** values,
                                        NewObjectKind newKind = GenericObject);

// Like NewDenseCopiedArray, but the array will have |proto| as prototype (or
// Array.prototype if |proto| is nullptr).
extern ArrayObject* NewDenseCopiedArrayWithProto(JSContext* cx, uint32_t length,
                                                 const Value* values,
                                                 HandleObject proto);

// Create a dense array with the given shape and length.
extern ArrayObject* NewDenseFullyAllocatedArrayWithShape(
    JSContext* cx, uint32_t length, Handle<SharedShape*> shape);

extern ArrayObject* NewArrayWithShape(JSContext* cx, uint32_t length,
                                      Handle<Shape*> shape);

extern bool ToLength(JSContext* cx, HandleValue v, uint64_t* out);

extern bool GetLengthProperty(JSContext* cx, HandleObject obj,
                              uint64_t* lengthp);

extern bool SetLengthProperty(JSContext* cx, HandleObject obj, uint32_t length);

/*
 * Copy 'length' elements from aobj to vp.
 *
 * This function assumes 'length' is effectively the result of calling
 * GetLengthProperty on aobj. vp must point to rooted memory.
 */
extern bool GetElements(JSContext* cx, HandleObject aobj, uint32_t length,
                        js::Value* vp);

/* Natives exposed for optimization by the interpreter and JITs. */

extern bool array_includes(JSContext* cx, unsigned argc, js::Value* vp);
extern bool array_indexOf(JSContext* cx, unsigned argc, js::Value* vp);
extern bool array_lastIndexOf(JSContext* cx, unsigned argc, js::Value* vp);
extern bool array_pop(JSContext* cx, unsigned argc, js::Value* vp);
extern bool array_join(JSContext* cx, unsigned argc, js::Value* vp);
extern bool array_sort(JSContext* cx, unsigned argc, js::Value* vp);

extern void ArrayShiftMoveElements(ArrayObject* arr);

extern JSObject* ArraySliceDense(JSContext* cx, HandleObject obj, int32_t begin,
                                 int32_t end, HandleObject result);

extern JSObject* ArgumentsSliceDense(JSContext* cx, HandleObject obj,
                                     int32_t begin, int32_t end,
                                     HandleObject result);

extern ArrayObject* NewArrayWithNullProto(JSContext* cx);

/*
 * Append the given (non-hole) value to the end of an array.  The array must be
 * a newborn array -- that is, one which has not been exposed to script for
 * arbitrary manipulation.  (This method optimizes on the assumption that
 * extending the array to accommodate the element will never make the array
 * sparse, which requires that the array be completely filled.)
 */
extern bool NewbornArrayPush(JSContext* cx, HandleObject obj, const Value& v);

extern ArrayObject* ArrayConstructorOneArg(JSContext* cx,
                                           Handle<ArrayObject*> templateObject,
                                           int32_t lengthInt);

#ifdef DEBUG
extern bool ArrayInfo(JSContext* cx, unsigned argc, Value* vp);
#endif

/* Array constructor native. Exposed only so the JIT can know its address. */
extern bool ArrayConstructor(JSContext* cx, unsigned argc, Value* vp);

// Like Array constructor, but doesn't perform GetPrototypeFromConstructor.
extern bool array_construct(JSContext* cx, unsigned argc, Value* vp);

extern JSString* ArrayToSource(JSContext* cx, HandleObject obj);

extern bool IsCrossRealmArrayConstructor(JSContext* cx, JSObject* obj,
                                         bool* result);

extern bool ObjectMayHaveExtraIndexedOwnProperties(JSObject* obj);

extern bool ObjectMayHaveExtraIndexedProperties(JSObject* obj);

extern bool PrototypeMayHaveIndexedProperties(NativeObject* obj);

// JS::IsArray has multiple overloads, use js::IsArrayFromJit to disambiguate.
extern bool IsArrayFromJit(JSContext* cx, HandleObject obj, bool* isArray);

extern bool ArrayLengthGetter(JSContext* cx, HandleObject obj, HandleId id,
                              MutableHandleValue vp);

extern bool ArrayLengthSetter(JSContext* cx, HandleObject obj, HandleId id,
                              HandleValue v, ObjectOpResult& result);

// Note: we use uint32_t because the JIT code uses branch32.
enum class ArraySortResult : uint32_t {
  Failure,
  Done,
  CallJS,
  CallJSSameRealmNoRectifier
};

// We use a JIT trampoline to optimize sorting with a comparator function. The
// trampoline frame has an ArraySortData instance that contains all state used
// by the sorting algorithm. The sorting algorithm is implemented as a C++
// "generator" that can yield to the trampoline to perform a fast JIT => JIT
// call to the comparator function. When the comparator function returns, the
// trampoline calls back into C++ to resume the sorting algorithm.
//
// ArraySortData stores the JS Values in a js::Vector. To ensure we don't leak
// its memory, we have debug assertions to check that for each C++ constructor
// call we call |freeMallocData| exactly once. C++ code calls |freeMallocData|
// when it's done sorting and the JIT exception handler calls it when unwinding
// the trampoline frame.
class ArraySortData {
 public:
  enum class ComparatorKind : uint8_t {
    Unoptimized,
    JS,
    JSSameRealmNoRectifier,
  };

  static constexpr size_t ComparatorActualArgs = 2;

  // Insertion sort is used if the length is < InsertionSortLimit.
  static constexpr size_t InsertionSortLimit = 24;

  using ValueVector = GCVector<Value, 8, SystemAllocPolicy>;

 protected:  // Silence Clang warning about unused private fields.
  // Data for the comparator call. These fields must match the JitFrameLayout
  // to let us perform efficient calls to the comparator from JIT code.
  // This is asserted in the JIT trampoline code.
  // callArgs[0] is also used to store the return value of the sort function and
  // the comparator.
  uintptr_t descriptor_;
  JSObject* comparator_ = nullptr;
  Value thisv;
  Value callArgs[ComparatorActualArgs];

 private:
  ValueVector vec;
  Value item;
  JSContext* cx_;
  JSObject* obj_ = nullptr;

  Value* list;
  Value* out;

  // The value of the .length property.
  uint32_t length;

  // The number of items to sort. Can be less than |length| if the object has
  // holes.
  uint32_t denseLen;

  uint32_t windowSize;
  uint32_t start;
  uint32_t mid;
  uint32_t end;
  uint32_t i, j, k;

  // The state value determines where we resume in sortWithComparator.
  enum class State : uint8_t {
    Initial,
    InsertionSortCall1,
    InsertionSortCall2,
    MergeSortCall1,
    MergeSortCall2
  };
  State state = State::Initial;
  ComparatorKind comparatorKind_;

  // Optional padding to ensure proper alignment of the comparator JIT frame.
#if !defined(JS_64BIT) && !defined(DEBUG)
 protected:  // Silence Clang warning about unused private field.
  size_t padding;
#endif

 public:
  explicit ArraySortData(JSContext* cx);

  void MOZ_ALWAYS_INLINE init(JSObject* obj, JSObject* comparator,
                              ValueVector&& vec, uint32_t length,
                              uint32_t denseLen);

  JSContext* cx() const { return cx_; }

  JSObject* comparator() const {
    MOZ_ASSERT(comparator_);
    return comparator_;
  }

  Value returnValue() const { return callArgs[0]; }
  void setReturnValue(JSObject* obj) { callArgs[0].setObject(*obj); }

  Value comparatorArg(size_t index) {
    MOZ_ASSERT(index < ComparatorActualArgs);
    return callArgs[index];
  }
  Value comparatorThisValue() const { return thisv; }
  Value comparatorReturnValue() const { return callArgs[0]; }
  void setComparatorArgs(const Value& x, const Value& y) {
    callArgs[0] = x;
    callArgs[1] = y;
  }
  void setComparatorReturnValue(const Value& v) { callArgs[0] = v; }

  ComparatorKind comparatorKind() const { return comparatorKind_; }

  static ArraySortResult sortWithComparator(ArraySortData* d);

  void freeMallocData();
  void trace(JSTracer* trc);

  static constexpr int32_t offsetOfDescriptor() {
    return offsetof(ArraySortData, descriptor_);
  }
  static constexpr int32_t offsetOfComparator() {
    return offsetof(ArraySortData, comparator_);
  }
  static constexpr int32_t offsetOfComparatorReturnValue() {
    return offsetof(ArraySortData, callArgs[0]);
  }
  static constexpr int32_t offsetOfComparatorThis() {
    return offsetof(ArraySortData, thisv);
  }
  static constexpr int32_t offsetOfComparatorArgs() {
    return offsetof(ArraySortData, callArgs);
  }
};

extern ArraySortResult ArraySortFromJit(
    JSContext* cx, jit::TrampolineNativeFrameLayout* frame);

class MOZ_NON_TEMPORARY_CLASS ArraySpeciesLookup final {
  /*
   * An ArraySpeciesLookup holds the following:
   *
   *  Array.prototype (arrayProto_)
   *      To ensure that the incoming array has the standard proto.
   *
   *  Array.prototype's shape (arrayProtoShape_)
   *      To ensure that Array.prototype has not been modified.
   *
   *  Array (arrayConstructor_)
   *  Array's shape (arrayConstructorShape_)
   *       To ensure that Array has not been modified.
   *
   *  Array.prototype's slot number for constructor (arrayProtoConstructorSlot_)
   *      To quickly retrieve and ensure that the Array constructor
   *      stored in the slot has not changed.
   *
   *  Array's slot number for the @@species getter. (arraySpeciesGetterSlot_)
   *  Array's canonical value for @@species (canonicalSpeciesFunc_)
   *      To quickly retrieve and ensure that the @@species getter for Array
   *      has not changed.
   *
   * MOZ_INIT_OUTSIDE_CTOR fields below are set in |initialize()|.  The
   * constructor only initializes a |state_| field, that defines whether the
   * other fields are accessible.
   */

  // Pointer to canonical Array.prototype and Array.
  MOZ_INIT_OUTSIDE_CTOR NativeObject* arrayProto_;
  MOZ_INIT_OUTSIDE_CTOR NativeObject* arrayConstructor_;

  // Shape of matching Array, and slot containing the @@species property, and
  // the canonical value.
  MOZ_INIT_OUTSIDE_CTOR Shape* arrayConstructorShape_;
  MOZ_INIT_OUTSIDE_CTOR uint32_t arraySpeciesGetterSlot_;
  MOZ_INIT_OUTSIDE_CTOR JSFunction* canonicalSpeciesFunc_;

  // Shape of matching Array.prototype object, and slot containing the
  // constructor for it.
  MOZ_INIT_OUTSIDE_CTOR Shape* arrayProtoShape_;
  MOZ_INIT_OUTSIDE_CTOR uint32_t arrayProtoConstructorSlot_;

  enum class State : uint8_t {
    // Flags marking the lazy initialization of the above fields.
    Uninitialized,
    Initialized,

    // The disabled flag is set when we don't want to try optimizing
    // anymore because core objects were changed.
    Disabled
  };

  State state_ = State::Uninitialized;

  // Initialize the internal fields.
  void initialize(JSContext* cx);

  // Reset the cache.
  void reset();

  // Check if the global array-related objects have not been messed with
  // in a way that would disable this cache.
  bool isArrayStateStillSane();

 public:
  /** Construct an |ArraySpeciesLookup| in the uninitialized state. */
  ArraySpeciesLookup() { reset(); }

  // Try to optimize the @@species lookup for an array.
  bool tryOptimizeArray(JSContext* cx, ArrayObject* array);

  // Purge the cache and all info associated with it.
  void purge() {
    if (state_ == State::Initialized) {
      reset();
    }
  }
};

} /* namespace js */

#endif /* builtin_Array_h */
