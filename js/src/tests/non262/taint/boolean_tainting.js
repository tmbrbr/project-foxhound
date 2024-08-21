function booleanTaintingBasicTest() {
    var a = taint(true);
    var b = taint(false);
    var c = true;
    var d = false;

    assertBooleanTainted(a);
    assertBooleanTainted(b);
    assertEq(a, c);
    assertEq(b, d);
    assertEq(JSON.stringify(a), "true");
    assertEq(JSON.stringify(b), "false");
    assertBooleanNotTainted(c);
    assertBooleanNotTainted(d);

    assertBooleanTainted(Boolean(taint(1)));
    assertBooleanTainted(Boolean(taint(0)));
    assertBooleanTainted(Boolean(taint(1.0)));
    assertBooleanTainted(Boolean(taint(0.0)));
    assertBooleanTainted(new Boolean(taint(1)));
    assertBooleanTainted(new Boolean(taint(0)));
    assertEq(Boolean(taint(1)), Boolean(1));
    assertEq(Boolean(taint(0)), Boolean(0));
    assertEq(Boolean(taint(1.0)), Boolean(1.0));
    assertEq(Boolean(taint(0.0)), Boolean(0.0));

    // This is a limitation of the way boolean to source is implemented.
    // The idea is that if a tainted boolean object is seen, the boolean
    // value will be returned not the boolean Object itself. Hence why
    // the output of these two functions are not what we expect.
    assertNotEq(new Boolean(taint(1)), new Boolean(1));
    assertNotEq(new Boolean(taint(0)), new Boolean(0));

    assertBooleanTainted(Boolean(taint("true")));
    assertBooleanTainted(Boolean(taint("false")));
    assertBooleanTainted(new Boolean(taint("true")));
    assertBooleanTainted(new Boolean(taint("false")));
    // No matter what string is passed, if the string has a length bigger
    // than 0, the boolean will be considered true.
    assertEq(Boolean(taint("true")), Boolean("true"));
    assertEq(Boolean(taint("false")), Boolean("false"));
    //Same as before
    assertNotEq(new Boolean(taint("true")), new Boolean("true"));
    assertNotEq(new Boolean(taint("false")), new Boolean("false"));

    assertEq(typeof a, typeof a);
    assertEq(typeof a, typeof b);
    assertEq(typeof a, typeof c);
    assertEq(typeof a, typeof d);
}

function booleanTaintingEqualityTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(a == a);
  assertBooleanTainted(a == b);
  assertBooleanTainted(a == c);
  assertBooleanTainted(a == d);
  assertEq(a == a, c);
  assertEq(a == b, d);
  assertEq(a == c, c);
  assertEq(a == d, d);

  assertBooleanTainted(b == a);
  assertBooleanTainted(b == b);
  assertBooleanTainted(b == c);
  assertBooleanTainted(b == d);
  assertEq(b == a, d);
  assertEq(b == b, c);
  assertEq(b == c, d);
  assertEq(b == d, c);

  assertBooleanTainted(c == a);
  assertBooleanTainted(c == b);
  assertBooleanNotTainted(c == c);
  assertBooleanNotTainted(c == d);
  assertEq(c == a, c);
  assertEq(c == b, d);
  assertEq(c == c, c);
  assertEq(c == d, d);

  assertBooleanTainted(d == a);
  assertBooleanTainted(d == b);
  assertBooleanNotTainted(d == c);
  assertBooleanNotTainted(d == d);
  assertEq(d == a, d);
  assertEq(d == b, c);
  assertEq(d == c, d);
  assertEq(d == d, c);

  assertBooleanTainted(a === a);
  assertBooleanTainted(a === b);
  assertBooleanTainted(a === c);
  assertBooleanTainted(a === d);
  assertEq(a === a, c);
  assertEq(a === b, d);
  assertEq(a === c, c);
  assertEq(a === d, d);

  assertBooleanTainted(b === a);
  assertBooleanTainted(b === b);
  assertBooleanTainted(b === c);
  assertBooleanTainted(b === d);
  assertEq(b === a, d);
  assertEq(b === b, c);
  assertEq(b === c, d);
  assertEq(b === d, c);

  assertBooleanTainted(c === a);
  assertBooleanTainted(c === b);
  assertBooleanNotTainted(c === c);
  assertBooleanNotTainted(c === d);
  assertEq(c === a, c);
  assertEq(c === b, d);
  assertEq(c === c, c);
  assertEq(c === d, d);

  assertBooleanTainted(d === a);
  assertBooleanTainted(d === b);
  assertBooleanNotTainted(d === c);
  assertBooleanNotTainted(d === d);
  assertEq(d === a, d);
  assertEq(d === b, c);
  assertEq(d === c, d);
  assertEq(d === d, c);
}

function booleanTaintingNotEqualityTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(a != a);
  assertBooleanTainted(a != b);
  assertBooleanTainted(a != c);
  assertBooleanTainted(a != d);
  assertEq(a != a, d);
  assertEq(a != b, c);
  assertEq(a != c, d);
  assertEq(a != d, c);

  assertBooleanTainted(b != a);
  assertBooleanTainted(b != b);
  assertBooleanTainted(b != c);
  assertBooleanTainted(b != d);
  assertEq(b != a, c);
  assertEq(b != b, d);
  assertEq(b != c, c);
  assertEq(b != d, d);

  assertBooleanTainted(c != a);
  assertBooleanTainted(c != b);
  assertBooleanNotTainted(c != c);
  assertBooleanNotTainted(c != d);
  assertEq(c != a, d);
  assertEq(c != b, c);
  assertEq(c != c, d);
  assertEq(c != d, c);

  assertBooleanTainted(d != a);
  assertBooleanTainted(d != b);
  assertBooleanNotTainted(d != c);
  assertBooleanNotTainted(d != d);
  assertEq(d != a, c);
  assertEq(d != b, d);
  assertEq(d != c, c);
  assertEq(d != d, d);

  assertBooleanTainted(a !== a);
  assertBooleanTainted(a !== b);
  assertBooleanTainted(a !== c);
  assertBooleanTainted(a !== d);
  assertEq(a !== a, d);
  assertEq(a !== b, c);
  assertEq(a !== c, d);
  assertEq(a !== d, c);

  assertBooleanTainted(b !== a);
  assertBooleanTainted(b !== b);
  assertBooleanTainted(b !== c);
  assertBooleanTainted(b !== d);
  assertEq(b !== a, c);
  assertEq(b !== b, d);
  assertEq(b !== c, c);
  assertEq(b !== d, d);

  assertBooleanTainted(c !== a);
  assertBooleanTainted(c !== b);
  assertBooleanNotTainted(c !== c);
  assertBooleanNotTainted(c !== d);
  assertEq(c !== a, d);
  assertEq(c !== b, c);
  assertEq(c !== c, d);
  assertEq(c !== d, c);

  assertBooleanTainted(d !== a);
  assertBooleanTainted(d !== b);
  assertBooleanNotTainted(d !== c);
  assertBooleanNotTainted(d !== d);
  assertEq(d !== a, c);
  assertEq(d !== b, d);
  assertEq(d !== c, c);
  assertEq(d !== d, d);
}

function booleanTaintingNegationTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(!a);
  assertBooleanTainted(!b);
  assertBooleanNotTainted(!c);
  assertBooleanNotTainted(!d);
  assertEq(!a, !c);
  assertEq(!b, !d);
}

function booleanTaintingOrTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(a || a);
  assertBooleanTainted(a || b);
  assertBooleanTainted(a || c);
  assertBooleanTainted(a || d);
  assertEq(a || a, c);
  assertEq(a || b, c);
  assertEq(a || c, c);
  assertEq(a || d, c);

  assertBooleanTainted(b || a);
  assertBooleanTainted(b || b);
  // This is not tainted because of the way or operations are parsed.
  // A detailed explination can be found in the documentation.
  assertBooleanNotTainted(b || c);
  assertBooleanNotTainted(b || d);
  assertEq(b || a, c);
  assertEq(b || b, d);
  assertEq(b || c, c);
  assertEq(b || d, d);

  assertBooleanNotTainted(c || a);
  assertBooleanNotTainted(c || b);
  assertBooleanNotTainted(c || c);
  assertBooleanNotTainted(c || d);
  assertEq(c || a, c);
  assertEq(c || b, c);

  assertBooleanTainted(d || a);
  assertBooleanTainted(d || b);
  assertBooleanNotTainted(d || c);
  assertBooleanNotTainted(d || d);
  assertEq(d || a, c);
  assertEq(d || b, d);
}

function booleanTaintingAndTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(a && a);
  assertBooleanTainted(a && b);
  // This is not tainted because of the way or operations are parsed.
  // A detailed explination can be found in the documentation.
  assertBooleanNotTainted(a && c);
  assertBooleanNotTainted(a && d);
  assertEq(a && a, c);
  assertEq(a && b, d);
  assertEq(a && c, c);
  assertEq(a && d, d);

  assertBooleanTainted(b && a);
  assertBooleanTainted(b && b);
  assertBooleanTainted(b && c);
  assertBooleanTainted(b && d);
  assertEq(b && a, d);
  assertEq(b && b, d);
  assertEq(b && c, d);
  assertEq(b && d, d);

  assertBooleanTainted(c && a);
  assertBooleanTainted(c && b);
  assertBooleanNotTainted(c && c);
  assertBooleanNotTainted(c && d);
  assertEq(c && a, c);
  assertEq(c && b, d);

  assertBooleanNotTainted(d && a);
  assertBooleanNotTainted(d && b);
  assertBooleanNotTainted(d && c);
  assertBooleanNotTainted(d && d);
  assertEq(d && a, d);
  assertEq(d && b, d);
}

function booleanTaintingStringConversionTest() {
  var str = 'hello';

  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertTainted(a + str);
  assertTainted(str + b);
  assertEq(a + str, c + str);
  assertEq(str + b, str + d);

  assertTainted(JSON.stringify(a));
  assertTainted(JSON.stringify(b + str));
  assertEq(JSON.stringify(a), JSON.stringify(c));
  assertEq(JSON.stringify(b + str), JSON.stringify(d + str));

  assertTainted(String(a));
  assertTainted(String(b + str));
  assertEq(String(a), String(c));
  assertEq(String(b + str), String(d + str));

  assertTainted(a.toString());
  assertTainted(b.toString());
  assertEq(a.toString(), c.toString());
  assertEq(b.toString(), d.toString());
}

function booleanTaintingNumberOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;


  assertNumberTainted(Number(a));
  assertNumberTainted(Number(b));
  assertNumberTainted(new Number(a));
  assertNumberTainted(new Number(b));
  assertEq(Number(a), 1);
  assertEq(Number(b), 0);
  assertEq(new Number(a), 1);
  assertEq(new Number(b), 0);

  assertNumberTainted(num + a);
  assertNumberTainted(b + num);
  assertEq(num + a, num + c);
  assertEq(b + num, d + num);

  assertNumberTainted(num - a);
  assertNumberTainted(b - num);
  assertEq(num - a, num - c);
  assertEq(b - num, d - num);

  assertNumberTainted(num * a);
  assertNumberTainted(b * num);
  assertEq(num * a, num * c);
  assertEq(b * num, d * num);

  assertNumberTainted(num / a);
  assertNumberTainted(b / num);
  //Testing division by zero
  assertNumberTainted(num / b);
  assertEq(num / a, num / c);
  assertEq(b / num, d / num);
  assertEq(num / b, num / d);

  assertNumberTainted(num % a);
  assertNumberTainted(b % num);
  assertEq(num % a, num % c);
  assertEq(b % num, d % num);

  assertNumberTainted(num ** a);
  assertNumberTainted(b ** num);
  assertEq(num ** a, num ** c);
  assertEq(b ** num, d ** num);
}

function booleanTaintingBitwiseOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertNumberTainted(num & a);
  assertNumberTainted(b & num);
  assertEq(num & a, num & c);
  assertEq(b & num, d & num);

  assertNumberTainted(num | a);
  assertNumberTainted(b | num);
  assertEq(num | a, num | c);
  assertEq(b | num, d | num);

  assertNumberTainted(num ^ a);
  assertNumberTainted(b ^ num);
  assertEq(num ^ a, num ^ c);
  assertEq(b ^ num, d ^ num);

  assertNumberTainted(~a);
  assertNumberTainted(~b);
  assertEq(~a, ~c);
  assertEq(~b, ~d);

  assertNumberTainted(num << a);
  assertNumberTainted(b << num);
  assertEq(num << a, num << c);
  assertEq(b << num, d << num);

  assertNumberTainted(num >> a);
  assertNumberTainted(b >> num);
  assertEq(num >> a, num >> c);
  assertEq(b >> num, d >> num);

  assertNumberTainted(num >>> a);
  assertNumberTainted(b >>> num);
  assertEq(num >>> a, num >>> c);
  assertEq(b >>> num, d >>> num);
}

function booleanTaintingComparisonOperationsTest() {
  var num1 = taint(20);
  var num2 = taint(30);
  var num3 = 20;
  var num4 = 30;

  assertBooleanTainted(num1 < num1);
  assertBooleanTainted(num1 < num2);
  assertBooleanTainted(num1 < num3);
  assertBooleanTainted(num1 < num4);
  assertEq(num1 < num1, num3 < num3);
  assertEq(num1 < num2, num3 < num4);
  assertEq(num1 < num3, num3 < num3);
  assertEq(num1 < num4, num3 < num4);

  assertBooleanTainted(num2 < num1);
  assertBooleanTainted(num2 < num2);
  assertBooleanTainted(num2 < num3);
  assertBooleanTainted(num2 < num4);
  assertEq(num2 < num1, num4 < num3);
  assertEq(num2 < num2, num4 < num4);
  assertEq(num2 < num3, num4 < num3);
  assertEq(num2 < num4, num4 < num4);

  assertBooleanTainted(num3 < num1);
  assertBooleanTainted(num3 < num2);
  assertBooleanNotTainted(num3 < num3);
  assertBooleanNotTainted(num3 < num4);
  assertEq(num3 < num1, num3 < num3);
  assertEq(num3 < num2, num3 < num4);

  assertBooleanTainted(num4 < num1);
  assertBooleanTainted(num4 < num2);
  assertBooleanNotTainted(num4 < num3);
  assertBooleanNotTainted(num4 < num4);
  assertEq(num4 < num1, num4 < num3);
  assertEq(num4 < num2, num4 < num4);

  assertBooleanTainted(num1 <= num1);
  assertBooleanTainted(num1 <= num2);
  assertBooleanTainted(num1 <= num3);
  assertBooleanTainted(num1 <= num4);
  assertEq(num1 <= num1, num3 <= num3);
  assertEq(num1 <= num2, num3 <= num4);
  assertEq(num1 <= num3, num3 <= num3);
  assertEq(num1 <= num4, num3 <= num4);

  assertBooleanTainted(num2 <= num1);
  assertBooleanTainted(num2 <= num2);
  assertBooleanTainted(num2 <= num3);
  assertBooleanTainted(num2 <= num4);
  assertEq(num2 <= num1, num4 <= num3);
  assertEq(num2 <= num2, num4 <= num4);
  assertEq(num2 <= num3, num4 <= num3);
  assertEq(num2 <= num4, num4 <= num4);

  assertBooleanTainted(num3 <= num1);
  assertBooleanTainted(num3 <= num2);
  assertBooleanNotTainted(num3 <= num3);
  assertBooleanNotTainted(num3 <= num4);
  assertEq(num3 <= num1, num3 <= num3);
  assertEq(num3 <= num2, num3 <= num4);

  assertBooleanTainted(num4 <= num1);
  assertBooleanTainted(num4 <= num2);
  assertBooleanNotTainted(num4 <= num3);
  assertBooleanNotTainted(num4 <= num4);
  assertEq(num4 <= num1, num4 <= num3);
  assertEq(num4 <= num2, num4 <= num4);

  assertBooleanTainted(num1 > num1);
  assertBooleanTainted(num1 > num2);
  assertBooleanTainted(num1 > num3);
  assertBooleanTainted(num1 > num4);
  assertEq(num1 > num1, num3 > num3);
  assertEq(num1 > num2, num3 > num4);
  assertEq(num1 > num3, num3 > num3);
  assertEq(num1 > num4, num3 > num4);

  assertBooleanTainted(num2 > num1);
  assertBooleanTainted(num2 > num2);
  assertBooleanTainted(num2 > num3);
  assertBooleanTainted(num2 > num4);
  assertEq(num2 > num1, num4 > num3);
  assertEq(num2 > num2, num4 > num4);
  assertEq(num2 > num3, num4 > num3);
  assertEq(num2 > num4, num4 > num4);

  assertBooleanTainted(num3 > num1);
  assertBooleanTainted(num3 > num2);
  assertBooleanNotTainted(num3 > num3);
  assertBooleanNotTainted(num3 > num4);
  assertEq(num3 > num1, num3 > num3);
  assertEq(num3 > num2, num3 > num4);

  assertBooleanTainted(num4 > num1);
  assertBooleanTainted(num4 > num2);
  assertBooleanNotTainted(num4 > num3);
  assertBooleanNotTainted(num4 > num4);
  assertEq(num4 > num1, num4 > num3);
  assertEq(num4 > num2, num4 > num4);

  assertBooleanTainted(num1 >= num1);
  assertBooleanTainted(num1 >= num2);
  assertBooleanTainted(num1 >= num3);
  assertBooleanTainted(num1 >= num4);
  assertEq(num1 >= num1, num3 >= num3);
  assertEq(num1 >= num2, num3 >= num4);
  assertEq(num1 >= num3, num3 >= num3);
  assertEq(num1 >= num4, num3 >= num4);

  assertBooleanTainted(num2 >= num1);
  assertBooleanTainted(num2 >= num2);
  assertBooleanTainted(num2 >= num3);
  assertBooleanTainted(num2 >= num4);
  assertEq(num2 >= num1, num4 >= num3);
  assertEq(num2 >= num2, num4 >= num4);
  assertEq(num2 >= num3, num4 >= num3);
  assertEq(num2 >= num4, num4 >= num4);

  assertBooleanTainted(num3 >= num1);
  assertBooleanTainted(num3 >= num2);
  assertBooleanNotTainted(num3 >= num3);
  assertBooleanNotTainted(num3 >= num4);
  assertEq(num3 >= num1, num3 >= num3);
  assertEq(num3 >= num2, num3 >= num4);

  assertBooleanTainted(num4 >= num1);
  assertBooleanTainted(num4 >= num2);
  assertBooleanNotTainted(num4 >= num3);
  assertBooleanNotTainted(num4 >= num4);
  assertEq(num4 >= num1, num4 >= num3);
  assertEq(num4 >= num2, num4 >= num4);
}


runTaintTest(booleanTaintingBasicTest);
runTaintTest(booleanTaintingEqualityTest);
runTaintTest(booleanTaintingNotEqualityTest);
runTaintTest(booleanTaintingNegationTest);
runTaintTest(booleanTaintingOrTest);
runTaintTest(booleanTaintingAndTest);
runTaintTest(booleanTaintingStringConversionTest);
runTaintTest(booleanTaintingNumberOperationsTest);
runTaintTest(booleanTaintingBitwiseOperationsTest);
runTaintTest(booleanTaintingComparisonOperationsTest);

if (typeof reportCompare === 'function')
  reportCompare(true, true);
