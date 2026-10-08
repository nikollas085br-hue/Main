#include "Physics.h"
#include <cmath>

namespace Physics {

Result suppose(Target target, double a, double b, double c) {
    Result r{false, "", "", ""};

    // The "Suposição de cálculo" module is a guided formula selector.
    // Inputs are interpreted by the UI labels before arriving here.
    switch (target) {
        case TARGET_VELOCITY:
            // a = distance, b = time
            if (b != 0) {
                r.ok = true; r.equation = "v = d / t";
                r.answer = "v = " + String(a / b, 8);
                r.explanation = "Use a distancia informada dividida pelo tempo.";
            }
            break;

        case TARGET_ACCELERATION:
            // a = delta-v, b = time
            if (b != 0) {
                r.ok = true; r.equation = "a = Δv / Δt";
                r.answer = "a = " + String(a / b, 8);
                r.explanation = "Aceleracao media: variacao da velocidade pelo tempo.";
            }
            break;

        case TARGET_FORCE:
            // a = mass, b = acceleration
            r.ok = true; r.equation = "F = m * a";
            r.answer = "F = " + String(a * b, 8);
            r.explanation = "Forca resultante pela segunda lei de Newton.";
            break;

        case TARGET_MASS:
            // a = force, b = acceleration
            if (b != 0) {
                r.ok = true; r.equation = "m = F / a";
                r.answer = "m = " + String(a / b, 8);
                r.explanation = "Massa obtida pela forca dividida pela aceleracao.";
            }
            break;

        case TARGET_TIME:
            // a = distance, b = velocity
            if (b != 0) {
                r.ok = true; r.equation = "t = d / v";
                r.answer = "t = " + String(a / b, 8);
                r.explanation = "Tempo de movimento uniforme.";
            }
            break;

        case TARGET_DISTANCE:
            // a = velocity, b = time
            r.ok = true; r.equation = "d = v * t";
            r.answer = "d = " + String(a * b, 8);
            r.explanation = "Distancia em movimento uniforme.";
            break;

        case TARGET_ENERGY:
            // a = mass, b = velocity
            r.ok = true; r.equation = "Ec = 0.5 * m * v^2";
            r.answer = "Ec = " + String(0.5 * a * b * b, 8);
            r.explanation = "Energia cinetica.";
            break;

        case TARGET_POWER:
            // a = energy/work, b = time
            if (b != 0) {
                r.ok = true; r.equation = "P = E / t";
                r.answer = "P = " + String(a / b, 8);
                r.explanation = "Potencia media.";
            }
            break;

        case TARGET_DENSITY:
            // a = mass, b = volume
            if (b != 0) {
                r.ok = true; r.equation = "ρ = m / V";
                r.answer = "ρ = " + String(a / b, 8);
                r.explanation = "Densidade.";
            }
            break;

        case TARGET_GRAVITY:
            // a = force, b = mass
            if (b != 0) {
                r.ok = true; r.equation = "g = F / m";
                r.answer = "g = " + String(a / b, 8);
                r.explanation = "Aceleracao gravitacional inferida pela relacao F=m*g.";
            }
            break;
    }

    if (!r.ok) {
        r.equation = "Nao foi possivel determinar";
        r.answer = "Faltam dados ou ha divisao por zero.";
        r.explanation = "Escolha novamente a grandeza e os dados fornecidos.";
    }
    return r;
}

}
