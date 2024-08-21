# Making Booleans Taint-aware

The way we made booleans taint aware is similar to the way we made numbers taint aware. For a more in depth
explination see the NumberTainind.md documentation. The main idea is that BooleanObject is used to keep the
boolean value and the taint information. All operations that are supported on tainted numbers are also supported
on tainted booleans.

However, there are two issues with Boolean tainting. The first issue is with the new Boolean() function.
Tainted booleans are stored inside of Boolean objects. When an operation is made on the Booleanobject, it implicitly
calls the function `tosource`. Normally this would return a string in this format: `new Boolean(true)`.
In order to hide the fact that a boolean is tainted, there is a check to see if the the boolean object is tainted.
If it is tainted, the string returned is in the format: `true`. The following example shows when this apraoch
goes wrong: `new Boolean(taint(1)) == new Boolean(1)`. Here we are comparing a boolean object that was created
with a tainted number and one that was created with a non tainted number. The expected result is true but this
evaluates to false since the left side is reduced to `new Boolean(true)` while the right side is reduced to `true`.

The second issue is with taint propagation for the "and" and "or" operators. To better understand the issue, it is
important to look at how the JS engine treats these types of operations. The same logic applies to both operatiors
so we will only examine the `or` operator. When there is an expression of the form `a || b`, and `a` is true, the
right hand side of the expression will not be evaluated because of short circuiting. To support this feautre, the
JS parser will parenthissize the expression to be able to skip the right hand side. So the expression `a || b || c`
will be considered as `a || (b || c)`. This expression is then put on the execution stack for the interpreter to
evaluate. The interpreter will read the or expression and only take the first value on the stack (left hand side
operator) and will check if it is true. If it is, it will skip the right hand side, and if it is false, it won't do
anything. Effectively, the interpreter only looks at the left hand side of an or expression and never looks at the right.
Once the "or" expression is done being evaluated, the result will be the last value on the stack. So if all operators
were false, the result will be the last right hand side operator (which was never read during the or evalution). If
any operator was true, the rest right hand side of that operator will be skipped, and the result will be true.

This is different than how the other two value expressions are evaluated. For example, in the case of an addition,
the let and right expressions will be read, added together, and the result will be placed back on the stack. This
optimization done for the "or" operator means that if we have the following expression: `a || b` where `a` is a tainted
boolean which has the value of `false`, it will be skipped. So the result of the expression will not be tainted.
To solve this issue, there needs to be a rework of how the JS parser deals with "and" and "or" operators as well
as the interpretor. The benefit gained from proper taint propagation for these operators does not outweigh the
potential destructive behaviour of changing core functionalities of the parser and interpreter.