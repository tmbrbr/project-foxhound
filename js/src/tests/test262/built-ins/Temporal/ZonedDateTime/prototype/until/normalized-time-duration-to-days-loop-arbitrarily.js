// |reftest| skip-if(!this.hasOwnProperty('Temporal')) -- Temporal is not enabled unconditionally
// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.until
description: >
  NormalizedTimeDurationToDays can loop arbitrarily up to max safe integer
info: |
  NormalizedTimeDurationToDays ( norm, zonedRelativeTo, timeZoneRec [ , precalculatedPlainDatetime ] )
  ...
  21. Repeat, while done is false,
    a. Let oneDayFarther be ? AddDaysToZonedDateTime(relativeResult.[[Instant]],
      relativeResult.[[DateTime]], timeZoneRec, zonedRelativeTo.[[Calendar]], sign).
    b. Set dayLengthNs to NormalizedTimeDurationFromEpochNanosecondsDifference(oneDayFarther.[[EpochNanoseconds]],
      relativeResult.[[EpochNanoseconds]]).
    c. Let oneDayLess be ? SubtractNormalizedTimeDuration(norm, dayLengthNs).
    c. If NormalizedTimeDurationSign(oneDayLess) × sign ≥ 0, then
        i. Set norm to oneDayLess.
        ii. Set relativeResult to oneDayFarther.
        iii. Set days to days + sign.
    d. Else,
        i. Set done to true.
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const calls = [];
const dayLengthNs = 86400000000000n;
const other = new Temporal.ZonedDateTime(dayLengthNs, "UTC", "iso8601");

function createRelativeTo(count) {
  const dayInstant = new Temporal.Instant(dayLengthNs);
  const substitutions = [];
  const timeZone = new Temporal.TimeZone("UTC");
  // Return constant value for first _count_ calls
  TemporalHelpers.substituteMethod(
    timeZone,
    "getPossibleInstantsFor",
    substitutions
  );
  substitutions.length = count;
  let i = 0;
  for (i = 0; i < substitutions.length; i++) {
    // (this value)
    substitutions[i] = [dayInstant];
  }
  // Record calls in calls[]
  TemporalHelpers.observeMethod(calls, timeZone, "getPossibleInstantsFor");
  return new Temporal.ZonedDateTime(0n, timeZone);
}

let zdt = createRelativeTo(50);
calls.splice(0); // Reset calls list after ZonedDateTime construction
zdt.until(other, {
  largestUnit: "day",
});
assert.sameValue(
  calls.length,
  50 + 1,
  "Expected ZonedDateTime.until to call getPossibleInstantsFor correct number of times"
);

zdt = createRelativeTo(100);
calls.splice(0); // Reset calls list after previous loop + ZonedDateTime construction
zdt.until(other, {
  largestUnit: "day",
});
assert.sameValue(
  calls.length,
  100 + 1,
  "Expected ZonedDateTime.until to call getPossibleInstantsFor correct number of times"
);

zdt = createRelativeTo(105);
assert.throws(RangeError, () => zdt.until(other, { largestUnit: "day" }), "105 days > 2⁵³ ns");

reportCompare(0, 0);
