#pragma once
#include <Arduino.h>

namespace MathEngine {
    struct Result {
        bool ok;
        double value;
        String text;
        Result() : ok(false), value(0), text("") {}
    };

    Result evaluate(const String& expression);
    Result solveLinear(double a, double b);
    Result solveQuadratic(double a, double b, double c);
    Result solveExponential(double baseA, double baseB, double exponentA, double exponentB);
}
