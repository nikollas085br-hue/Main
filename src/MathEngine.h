#pragma once
#include <Arduino.h>
namespace MathEngine {
struct Result { bool ok; double value; String text; };
Result evaluate(const String& expression, double x=0.0);
Result solveLinear(double a,double b);
Result solveQuadratic(double a,double b,double c);
Result solveExponential(double a,double b,double c,double d); // a^(b*x+c)=d
String describeExpression(const String& expression);
}
