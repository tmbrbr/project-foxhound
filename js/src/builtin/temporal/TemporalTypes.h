/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef builtin_temporal_TemporalTypes_h
#define builtin_temporal_TemporalTypes_h

#include "mozilla/Assertions.h"
#include "mozilla/CheckedInt.h"

#include <stdint.h>

#include "builtin/temporal/TemporalUnit.h"

namespace js::temporal {

// Use __builtin_assume when available, otherwise fall back to
// __builtin_unreachable.
#if defined __has_builtin
#  if __has_builtin(__builtin_assume)
#    define JS_ASSUME(x) __builtin_assume(x)
#  elif __has_builtin(__builtin_unreachable)
#    define JS_ASSUME(x) \
      if (!(x)) __builtin_unreachable()
#  else
#  endif
#endif

// Fallback to no-op if neither built-in is available.
#ifndef JS_ASSUME
#  define JS_ASSUME(x) \
    do {               \
    } while (false)
#endif

/**
 * Struct to represent a seconds and nanoseconds value. The nanoseconds value
 * is normalized to the range [0, 999'999'999].
 */
template <typename Derived>
struct SecondsAndNanoseconds {
  // Seconds part in the range allowed by the derived class.
  int64_t seconds = 0;

  // Nanoseconds part in the range [0, 999'999'999].
  int32_t nanoseconds = 0;

  constexpr bool operator==(const SecondsAndNanoseconds& other) const {
    return seconds == other.seconds && nanoseconds == other.nanoseconds;
  }

  constexpr bool operator<(const SecondsAndNanoseconds& other) const {
    // The compiler can optimize expressions like |instant < Instant{}| to a
    // single right-shift operation when we propagate the range of nanoseconds.
    JS_ASSUME(nanoseconds >= 0);
    JS_ASSUME(other.nanoseconds >= 0);
    return (seconds < other.seconds) ||
           (seconds == other.seconds && nanoseconds < other.nanoseconds);
  }

  // Other operators are implemented in terms of operator== and operator<.
  constexpr bool operator!=(const SecondsAndNanoseconds& other) const {
    return !(*this == other);
  }
  constexpr bool operator>(const SecondsAndNanoseconds& other) const {
    return other < *this;
  }
  constexpr bool operator<=(const SecondsAndNanoseconds& other) const {
    return !(other < *this);
  }
  constexpr bool operator>=(const SecondsAndNanoseconds& other) const {
    return !(*this < other);
  }

 protected:
  template <typename T, typename U, class R = Derived>
  static constexpr R add(const SecondsAndNanoseconds<T>& self,
                         const SecondsAndNanoseconds<U>& other) {
    // The caller needs to make sure integer overflow won't happen. CheckedInt
    // will assert on overflow and we intentionally don't try to recover from
    // overflow in this method.

    mozilla::CheckedInt64 secs = self.seconds;
    secs += other.seconds;

    mozilla::CheckedInt32 nanos = self.nanoseconds;
    nanos += other.nanoseconds;

    if (nanos.value() >= 1'000'000'000) {
      secs += 1;
      nanos -= 1'000'000'000;
    }
    MOZ_ASSERT(0 <= nanos.value() && nanos.value() < 1'000'000'000);

    return {secs.value(), nanos.value()};
  }

  template <class T, class U, class R = Derived>
  static constexpr R subtract(const SecondsAndNanoseconds<T>& self,
                              const SecondsAndNanoseconds<U>& other) {
    // The caller needs to make sure integer underflow won't happen. CheckedInt
    // will assert on underflow and we intentionally don't try to recover from
    // underflow in this method.

    mozilla::CheckedInt64 secs = self.seconds;
    secs -= other.seconds;

    mozilla::CheckedInt32 nanos = self.nanoseconds;
    nanos -= other.nanoseconds;

    if (nanos.value() < 0) {
      secs -= 1;
      nanos += 1'000'000'000;
    }
    MOZ_ASSERT(0 <= nanos.value() && nanos.value() < 1'000'000'000);

    return {secs.value(), nanos.value()};
  }

  static constexpr Derived negate(const Derived& self) {
    return subtract(Derived{}, self);
  }

 public:
  /**
   * Return the absolute value.
   */
  constexpr Derived abs() const {
    int64_t sec = seconds;
    int32_t nanos = nanoseconds;
    if (sec < 0) {
      if (nanos > 0) {
        sec += 1;
        nanos -= 1'000'000'000;
      }
      sec = -sec;
      nanos = -nanos;
    }
    return {sec, nanos};
  }

  /**
   * Return the seconds value, rounded towards zero.
   */
  constexpr int64_t toSeconds() const {
    int64_t sec = seconds;
    int32_t nanos = nanoseconds;
    if (sec < 0 && nanos > 0) {
      sec += 1;
    }
    return sec;
  }

  /**
   * Return the milliseconds value, rounded towards zero.
   */
  constexpr int64_t toMilliseconds() const {
    int64_t sec = seconds;
    int32_t nanos = nanoseconds;
    if (sec < 0 && nanos > 0) {
      sec += 1;
      nanos -= 1'000'000'000;
    }
    return (sec * 1'000) + (nanos / 1'000'000);
  }

  /**
   * Return the microseconds value, rounded towards zero.
   */
  constexpr int64_t toMicroseconds() const {
    int64_t sec = seconds;
    int32_t nanos = nanoseconds;
    if (sec < 0 && nanos > 0) {
      sec += 1;
      nanos -= 1'000'000'000;
    }
    return (sec * 1'000'000) + (nanos / 1'000);
  }

  /**
   * Return the nanoseconds value.
   *
   * The returned nanoseconds amount can be invalid on overflow. The caller is
   * responsible for handling the overflow case.
   */
  constexpr mozilla::CheckedInt64 toNanoseconds() const {
    mozilla::CheckedInt64 nanos = seconds;
    nanos *= ToNanoseconds(TemporalUnit::Second);
    nanos += nanoseconds;
    return nanos;
  }

  /**
   * Create from a minutes value.
   */
  static constexpr Derived fromMinutes(int64_t minutes) {
    return {minutes * 60, 0};
  }

  /**
   * Create from a seconds value.
   */
  static constexpr Derived fromSeconds(int64_t seconds) { return {seconds, 0}; }

  /**
   * Create from a milliseconds value.
   */
  static constexpr Derived fromMilliseconds(int64_t milliseconds) {
    int64_t seconds = milliseconds / 1'000;
    int32_t millis = milliseconds % 1'000;
    if (millis < 0) {
      seconds -= 1;
      millis += 1'000;
    }
    return {seconds, millis * 1'000'000};
  }

  /**
   * Create from a microseconds value.
   */
  static constexpr Derived fromMicroseconds(int64_t microseconds) {
    int64_t seconds = microseconds / 1'000'000;
    int32_t micros = microseconds % 1'000'000;
    if (micros < 0) {
      seconds -= 1;
      micros += 1'000'000;
    }
    return {seconds, micros * 1'000};
  }

  /**
   * Create from a nanoseconds value.
   */
  static constexpr Derived fromNanoseconds(int64_t nanoseconds) {
    int64_t seconds = nanoseconds / 1'000'000'000;
    int32_t nanos = nanoseconds % 1'000'000'000;
    if (nanos < 0) {
      seconds -= 1;
      nanos += 1'000'000'000;
    }
    return {seconds, nanos};
  }
};

#undef JS_ASSUME

/**
 * InstantSpan represents a span of time between two Instants, measured in
 * nanoseconds.
 */
struct InstantSpan final : SecondsAndNanoseconds<InstantSpan> {
  constexpr InstantSpan& operator+=(const InstantSpan& other) {
    *this = add(*this, other);
    return *this;
  }

  constexpr InstantSpan& operator-=(const InstantSpan& other) {
    *this = subtract(*this, other);
    return *this;
  }

  constexpr InstantSpan operator+(const InstantSpan& other) const {
    return add(*this, other);
  }

  constexpr InstantSpan operator-(const InstantSpan& other) const {
    return subtract(*this, other);
  }

  constexpr InstantSpan operator-() const { return negate(*this); }

  /**
   * Returns the maximum instant span value.
   */
  static constexpr InstantSpan max() {
    // The limit is 2×8.64 × 10^21 nanoseconds, which is 2×8.64 × 10^12 seconds.
    constexpr int64_t seconds = 2 * 8'640'000'000'000;
    constexpr int64_t nanos = 0;
    return {seconds, nanos};
  }

  /**
   * Returns the minimum instant span value.
   */
  static constexpr InstantSpan min() { return -max(); }
};

/**
 * Instant represents a time since the epoch value, measured in nanoseconds.
 *
 * Instant supports a range of ±8.64 × 10^21 nanoseconds, covering ±10^8 days
 * in either direction relative to midnight at the beginning of 1 January 1970
 * UTC. The range also exactly matches the supported range of JavaScript Date
 * objects.
 *
 * C++ doesn't provide a built-in type capable of storing an integer in the
 * range ±8.64 × 10^21, therefore we need to create our own abstraction. This
 * struct follows the design of `std::timespec` and splits the instant into a
 * signed seconds part and an unsigned nanoseconds part.
 */
struct Instant final : SecondsAndNanoseconds<Instant> {
  constexpr Instant& operator+=(const InstantSpan& other) {
    *this = add(*this, other);
    return *this;
  }

  constexpr Instant& operator-=(const InstantSpan& other) {
    *this = subtract(*this, other);
    return *this;
  }

  constexpr Instant operator+(const InstantSpan& other) const {
    return add(*this, other);
  }

  constexpr Instant operator-(const InstantSpan& other) const {
    return subtract(*this, other);
  }

  constexpr InstantSpan operator-(const Instant& other) const {
    return subtract<Instant, Instant, InstantSpan>(*this, other);
  }

  constexpr Instant operator-() const { return negate(*this); }

  /**
   * Return this instant as microseconds from the start of the epoch. (Rounds
   * towards negative infinity.)
   */
  constexpr int64_t floorToMicroseconds() const {
    return (seconds * 1'000'000) + (nanoseconds / 1'000);
  }

  /**
   * Return this instant as milliseconds from the start of the epoch. (Rounds
   * towards negative infinity.)
   */
  constexpr int64_t floorToMilliseconds() const {
    return (seconds * 1'000) + (nanoseconds / 1'000'000);
  }

  /**
   * Return this instant as milliseconds from the start of the epoch. (Rounds
   * towards positive infinity.)
   */
  constexpr int64_t ceilToMilliseconds() const {
    return floorToMilliseconds() + int64_t(nanoseconds % 1'000'000 != 0);
  }

  /**
   * Returns the maximum instant value.
   */
  static constexpr Instant max() {
    // The limit is 8.64 × 10^21 nanoseconds, which is 8.64 × 10^12 seconds.
    constexpr int64_t seconds = 8'640'000'000'000;
    constexpr int64_t nanos = 0;
    return {seconds, nanos};
  }

  /**
   * Returns the minimum instant value.
   */
  static constexpr Instant min() { return -max(); }
};

/**
 * Plain date represents a date in the ISO 8601 calendar.
 */
struct PlainDate final {
  // [-271821, 275760]
  //
  // Dates are limited to the range of ±100'000'000 days relative to midnight at
  // the beginning of 1 January 1970 UTC. This limits valid years to [-271821,
  // 275760].
  int32_t year = 0;

  // [1, 12]
  int32_t month = 0;

  // [1, 31]
  int32_t day = 0;

  bool operator==(const PlainDate& other) const {
    return year == other.year && month == other.month && day == other.day;
  }

  bool operator!=(const PlainDate& other) const { return !(*this == other); }
};

/**
 * Plain time represents a time value on a 24-hour clock. Leap seconds aren't
 * supported.
 */
struct PlainTime final {
  // [0, 23]
  int32_t hour = 0;

  // [0, 59]
  int32_t minute = 0;

  // [0, 59]
  int32_t second = 0;

  // [0, 999]
  int32_t millisecond = 0;

  // [0, 999]
  int32_t microsecond = 0;

  // [0, 999]
  int32_t nanosecond = 0;

  bool operator==(const PlainTime& other) const {
    return hour == other.hour && minute == other.minute &&
           second == other.second && millisecond == other.millisecond &&
           microsecond == other.microsecond && nanosecond == other.nanosecond;
  }

  bool operator!=(const PlainTime& other) const { return !(*this == other); }
};

/**
 * Plain date-time represents a date-time value in the ISO 8601 calendar.
 */
struct PlainDateTime final {
  PlainDate date;
  PlainTime time;

  bool operator==(const PlainDateTime& other) const {
    return date == other.date && time == other.time;
  }

  bool operator!=(const PlainDateTime& other) const {
    return !(*this == other);
  }
};

/**
 * Duration represents the difference between dates or times. Each duration
 * component is an integer and all components must have the same sign.
 */
struct Duration final {
  double years = 0;
  double months = 0;
  double weeks = 0;
  double days = 0;
  double hours = 0;
  double minutes = 0;
  double seconds = 0;
  double milliseconds = 0;
  double microseconds = 0;
  double nanoseconds = 0;

  bool operator==(const Duration& other) const {
    return years == other.years && months == other.months &&
           weeks == other.weeks && days == other.days && hours == other.hours &&
           minutes == other.minutes && seconds == other.seconds &&
           milliseconds == other.milliseconds &&
           microseconds == other.microseconds &&
           nanoseconds == other.nanoseconds;
  }

  bool operator!=(const Duration& other) const { return !(*this == other); }

  /**
   * Return the date components of this duration.
   */
  Duration date() const { return {years, months, weeks, days}; }

  /**
   * Return the time components of this duration.
   */
  Duration time() const {
    return {
        0,
        0,
        0,
        0,
        hours,
        minutes,
        seconds,
        milliseconds,
        microseconds,
        nanoseconds,
    };
  }

  /**
   * Return a new duration with every component negated.
   */
  Duration negate() const {
    // Add zero to convert -0 to +0.
    return {
        -years + (+0.0),       -months + (+0.0),       -weeks + (+0.0),
        -days + (+0.0),        -hours + (+0.0),        -minutes + (+0.0),
        -seconds + (+0.0),     -milliseconds + (+0.0), -microseconds + (+0.0),
        -nanoseconds + (+0.0),
    };
  }
};

/**
 * Date duration represents the difference between dates. Each duration
 * component is an integer and all components must have the same sign.
 */
struct DateDuration final {
  double years = 0;
  double months = 0;
  double weeks = 0;
  double days = 0;

  Duration toDuration() { return {years, months, weeks, days}; }
};

/**
 * Time duration represents the difference between times. Each duration
 * component is an integer and all components must have the same sign.
 */
struct TimeDuration final {
  double days = 0;
  double hours = 0;
  double minutes = 0;
  double seconds = 0;
  double milliseconds = 0;
  double microseconds = 0;
  double nanoseconds = 0;

  Duration toDuration() {
    return {0,
            0,
            0,
            days,
            hours,
            minutes,
            seconds,
            milliseconds,
            microseconds,
            nanoseconds};
  }
};

} /* namespace js::temporal */

#endif /* builtin_temporal_TemporalTypes_h */
