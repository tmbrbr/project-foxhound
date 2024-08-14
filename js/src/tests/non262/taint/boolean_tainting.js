function booleanTaintingBasicTest() {
    var a = taint(true);
    var b = taint(false);
    var c = true;
    var d = false;

    assertBooleanTainted(a);
    assertBooleanTainted(b);
    assertEq(a, c);
    assertEq(b, d);
    assertEq(JSON.stringify(a), 'true');
    assertEq(JSON.stringify(b), 'false');
    assertBooleanNotTainted(c);
    assertBooleanNotTainted(d);
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
  assertEq(JSON.stringify(!a), 'false');
  assertBooleanTainted(!b);
  assertEq(JSON.stringify(!b), 'true');
  assertBooleanNotTainted(!c);
  assertBooleanNotTainted(!d);
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
  assertEq(JSON.stringify(a || a), 'true');
  assertEq(JSON.stringify(a || b), 'true');
  assertEq(JSON.stringify(a || c), 'true');
  assertEq(JSON.stringify(a || d), 'true');

  assertBooleanTainted(b || a);
  assertBooleanTainted(b || b);
  assertBooleanNotTainted(b || c);
  assertBooleanNotTainted(b || d);
  assertEq(JSON.stringify(b || a), 'true');
  assertEq(JSON.stringify(b || b), 'false');
  assertEq(JSON.stringify(b || c), 'true');
  assertEq(JSON.stringify(b || d), 'false');

  assertBooleanNotTainted(c || a);
  assertBooleanNotTainted(c || b);
  assertBooleanNotTainted(c || c);
  assertBooleanNotTainted(c || d);
  assertEq(JSON.stringify(c || a), 'true');
  assertEq(JSON.stringify(c || b), 'true');

  assertBooleanTainted(d || a);
  assertBooleanTainted(d || b);
  assertBooleanNotTainted(d || c);
  assertBooleanNotTainted(d || d);
  assertEq(JSON.stringify(d || a), 'true');
  assertEq(JSON.stringify(d || b), 'false');
}

function booleanTaintingAndTest() {
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertBooleanTainted(a && a);
  assertBooleanTainted(a && b);
  assertBooleanNotTainted(a && c);
  assertBooleanNotTainted(a && d);
  assertEq(JSON.stringify(a && a), 'true');
  assertEq(JSON.stringify(a && b), 'false');
  assertEq(JSON.stringify(a && c), 'true');
  assertEq(JSON.stringify(a && d), 'false');

  assertBooleanTainted(b && a);
  assertBooleanTainted(b && b);
  assertBooleanTainted(b && c);
  assertBooleanTainted(b && d);
  assertEq(JSON.stringify(b && a), 'false');
  assertEq(JSON.stringify(b && b), 'false');
  assertEq(JSON.stringify(b && c), 'false');
  assertEq(JSON.stringify(b && d), 'false');

  assertBooleanTainted(c && a);
  assertBooleanTainted(c && b);
  assertBooleanNotTainted(c && c);
  assertBooleanNotTainted(c && d);
  assertEq(JSON.stringify(c && a), 'true');
  assertEq(JSON.stringify(c && b), 'false');

  assertBooleanNotTainted(d && a);
  assertBooleanNotTainted(d && b);
  assertBooleanNotTainted(d && c);
  assertBooleanNotTainted(d && d);
  assertEq(JSON.stringify(d && a), 'false');
  assertEq(JSON.stringify(d && b), 'false');
}

function booleanTaintingStringConversionTest() {
  var str = 'hello';

  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertTainted(a + str);
  assertEq(a + str, c + str);
  assertTainted(str + b);
  assertEq(str + b, str + d);

  assertTainted(JSON.stringify(a));
  assertEq(JSON.stringify(a), JSON.stringify(c));
  assertTainted(JSON.stringify(b + str));
  assertEq(JSON.stringify(b + str), JSON.stringify(d + str));

  assertTainted(String(a));
  assertEq(String(a), String(c));
  assertTainted(String(b + str));
  assertEq(String(b + str), String(d + str));

  assertTainted(a.toString());
  assertEq(a.toString(), c.toString());
  assertTainted(b.toString());
  assertEq(b.toString(), d.toString());
}

function booleanTaintingNumberOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;


  assertNumberTainted(Number(a));
  assertEq(Number(a), 1);
  assertNumberTainted(Number(b));
  assertEq(Number(b), 0);
  assertNumberTainted(new Number(a));
  assertEq(Number(a), 1);
  assertNumberTainted(new Number(b));
  assertEq(Number(b), 0);

  assertNumberTainted(num + a);
  assertEq(num + a, num + c);
  assertNumberTainted(b + num);
  assertEq(b + num, d + num);

  assertNumberTainted(num - a);
  assertEq(num - a, num - c);
  assertNumberTainted(b - num);
  assertEq(b - num, d - num);

  assertNumberTainted(num * a);
  assertEq(num * a, num * c);
  assertNumberTainted(b * num);
  assertEq(b * num, d * num);

  assertNumberTainted(num / a);
  assertEq(num / a, num / c);
  assertNumberTainted(b / num);
  assertEq(b / num, d / num);
  //Testing division by zero
  assertNumberTainted(num / b);
  assertEq(num / b, num / d);

  assertNumberTainted(num % a);
  assertEq(num % a, num % c);
  assertNumberTainted(b % num);
  assertEq(b % num, d % num);

  assertNumberTainted(num ** a);
  assertEq(num ** a, num ** c);
  assertNumberTainted(b ** num);
  assertEq(b ** num, d ** num);
}

function booleanTaintingBitwiseOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertNumberTainted(num & a);
  assertEq(num & a, num & c);
  assertNumberTainted(b & num);
  assertEq(b & num, d & num);

  assertNumberTainted(num | a);
  assertEq(num | a, num | c);
  assertNumberTainted(b | num);
  assertEq(b | num, d | num);

  assertNumberTainted(num ^ a);
  assertEq(num ^ a, num ^ c);
  assertNumberTainted(b ^ num);
  assertEq(b ^ num, d ^ num);

  assertNumberTainted(~a);
  assertEq(~a, ~c);
  assertNumberTainted(~b);
  assertEq(~b, ~d);

  assertNumberTainted(num << a);
  assertEq(num << a, num << c);
  assertNumberTainted(b << num);
  assertEq(b << num, d << num);

  assertNumberTainted(num >> a);
  assertEq(num >> a, num >> c);
  assertNumberTainted(b >> num);
  assertEq(b >> num, d >> num);

  assertNumberTainted(num >>> a);
  assertEq(num >>> a, num >>> c);
  assertNumberTainted(b >>> num);
  assertEq(b >>> num, d >>> num);
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

if (typeof reportCompare === 'function')
  reportCompare(true, true);
