function booleanTaintingBasicTest() {
    var a = taint(true);
    var b = taint(false);
    var c = true;
    var d = false;

    assertBooleanTainted(a);
    assertEq(JSON.stringify(a), 'true');
    assertBooleanTainted(b);
    assertEq(JSON.stringify(b), 'false');
    assertBooleanNotTainted(c);
    assertBooleanNotTainted(d);
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
}

function booleanTaintingNumberOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertTainted(num + a);
  assertEq(num + a, num + c);
  assertTainted(b + num);
  assertEq(b + num, d + num);

  assertTainted(num - a);
  assertEq(num - a, num - c);
  assertTainted(b - num);
  assertEq(b - num, d - num);

  assertTainted(num * a);
  assertEq(num * a, num * c);
  assertTainted(b * num);
  assertEq(b * num, d * num);

  assertTainted(num / a);
  assertEq(num / a, num / c);
  assertTainted(b / num);
  assertEq(b / num, d / num);
  //Testing division by zero
  assertTainted(num / b);
  assertEq(num / b, num / d);

  assertTainted(num % a);
  assertEq(num % a, num % c);
  assertTainted(b % num);
  assertEq(b % num, d % num);

  assertTainted(num ** a);
  assertEq(num ** a, num ** c);
  assertTainted(b ** num);
  assertEq(b ** num, d ** num);
}

function booleanTaintingBitwiseOperationsTest() {
  var num = 20;
  var a = taint(true);
  var b = taint(false);
  var c = true;
  var d = false;

  assertTainted(num & a);
  assertEq(num & a, num & c);
  assertTainted(b & num);
  assertEq(b & num, d & num);

  assertTainted(num | a);
  assertEq(num | a, num | c);
  assertTainted(b | num);
  assertEq(b | num, d | num);

  assertTainted(num ^ a);
  assertEq(num ^ a, num ^ c);
  assertTainted(b ^ num);
  assertEq(b ^ num, d ^ num);

  assertTainted(~a);
  assertEq(~a, ~c);
  assertTainted(~b);
  assertEq(~b, ~d);

  assertTainted(num << a);
  assertEq(num << a, num << c);
  assertTainted(b << num);
  assertEq(b << num, d << num);

  assertTainted(num >> a);
  assertEq(num >> a, num >> c);
  assertTainted(b >> num);
  assertEq(b >> num, d >> num);

  assertTainted(num >>> a);
  assertEq(num >>> a, num >>> c);
  assertTainted(b >>> num);
  assertEq(b >>> num, d >>> num);
}


runTaintTest(booleanTaintingBasicTest);
runTaintTest(booleanTaintingNegationTest);
runTaintTest(booleanTaintingOrTest);
runTaintTest(booleanTaintingAndTest);
runTaintTest(booleanTaintingStringConversionTest);
runTaintTest(booleanTaintingNumberOperationsTest);
runTaintTest(booleanTaintingBitwiseOperationsTest);

if (typeof reportCompare === 'function')
  reportCompare(true, true);
