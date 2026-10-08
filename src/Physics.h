#pragma once
#include <Arduino.h>
namespace Physics {
    enum Target {
        TARGET_VELOCITY,
        TARGET_ACCELERATION,
        TARGET_FORCE,
        TARGET_MASS,
        TARGET_TIME,
        TARGET_DISTANCE,
        TARGET_ENERGY,
        TARGET_POWER,
        TARGET_DENSITY,
        TARGET_GRAVITY
    };
    struct Result { bool ok; String equation; String answer; String explanation; };
    Result suppose(Target target, double a, double b, double c);
}
