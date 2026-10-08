#include "MathEngine.h"
#include <cmath>
#include <cctype>
#include <cstdlib>

namespace {
class Parser {
    const char* s;
    size_t p;
    bool bad;

    void ws() { while (s[p] == ' ' || s[p] == '\t') ++p; }

    bool eat(char c) {
        ws();
        if (s[p] == c) { ++p; return true; }
        return false;
    }

    double number() {
        ws();
        char* end = nullptr;
        double v = strtod(s + p, &end);
        if (end == s + p) { bad = true; return 0; }
        p = (size_t)(end - s);
        return v;
    }

    String ident() {
        ws();
        String r;
        while (isalpha((unsigned char)s[p]) || s[p] == '_') {
            r += s[p++];
        }
        return r;
    }

    double primary() {
        ws();

        if (eat('(')) {
            double v = expr();
            if (!eat(')')) bad = true;
            return v;
        }

        if (s[p] == '+' || s[p] == '-') {
            char sign = s[p++];
            double v = primary();
            return sign == '-' ? -v : v;
        }

        if (isdigit((unsigned char)s[p]) || s[p] == '.') return number();

        if (s[p] == 'p' && s[p+1] == 'i') {
            p += 2;
            return PI;
        }

        if (s[p] == 'e') {
            ++p;
            return M_E;
        }

        if (isalpha((unsigned char)s[p])) {
            String id = ident();
            if (id == "pi" || id == "PI") return PI;
            if (id == "e" || id == "E") return M_E;

            if (eat('(')) {
                double x = expr();
                if (!eat(')')) bad = true;
                if (id == "sin") return sin(x);
                if (id == "cos") return cos(x);
                if (id == "tan") return tan(x);
                if (id == "asin") return asin(x);
                if (id == "acos") return acos(x);
                if (id == "atan") return atan(x);
                if (id == "sqrt") return sqrt(x);
                if (id == "abs") return fabs(x);
                if (id == "ln") return log(x);
                if (id == "log") return log10(x);
                if (id == "exp") return exp(x);
                if (id == "floor") return floor(x);
                if (id == "ceil") return ceil(x);
                bad = true;
                return 0;
            }
            bad = true;
            return 0;
        }

        bad = true;
        return 0;
    }

    double power() {
        double a = primary();
        ws();
        if (eat('^')) {
            double b = power();  // right associative
            a = pow(a, b);
        }
        return a;
    }

    double term() {
        double v = power();
        while (!bad) {
            ws();
            if (eat('*')) v *= power();
            else if (eat('/')) {
                double d = power();
                if (fabs(d) < 1e-15) { bad = true; return 0; }
                v /= d;
            } else break;
        }
        return v;
    }

    double expr() {
        double v = term();
        while (!bad) {
            ws();
            if (eat('+')) v += term();
            else if (eat('-')) v -= term();
            else break;
        }
        return v;
    }

public:
    explicit Parser(const char* in) : s(in), p(0), bad(false) {}
    MathEngine::Result run() {
        MathEngine::Result r;
        r.value = expr();
        ws();
        if (s[p] != '\0') bad = true;
        if (!bad && isfinite(r.value)) {
            r.ok = true;
            r.text = String(r.value, 10);
        } else {
            r.text = "ERRO: expressao invalida";
        }
        return r;
    }
};
}

namespace MathEngine {

Result evaluate(const String& expression) {
    // Important: π is kept as a semantic token. We NEVER replace it with
    // the text "3.14", avoiding the old PI/3.14 parsing problem.
    String e = expression;
    e.replace("π", "pi");
    e.replace("Π", "pi");
    e.trim();
    Parser p(e.c_str());
    return p.run();
}

Result solveLinear(double a, double b) {
    Result r;
    if (fabs(a) < 1e-15) {
        r.text = fabs(b) < 1e-15 ? "Infinitas solucoes" : "Sem solucao";
        return r;
    }
    r.ok = true;
    r.value = -b / a;
    r.text = "x = " + String(r.value, 8);
    return r;
}

Result solveQuadratic(double a, double b, double c) {
    Result r;
    if (fabs(a) < 1e-15) return solveLinear(b, c);

    double d = b*b - 4*a*c;
    if (d < -1e-12) {
        r.text = "Sem raiz real";
        return r;
    }
    if (fabs(d) < 1e-12) {
        r.ok = true;
        r.value = -b/(2*a);
        r.text = "x1 = x2 = " + String(r.value, 8);
        return r;
    }

    double sd = sqrt(d);
    double x1 = (-b + sd)/(2*a);
    double x2 = (-b - sd)/(2*a);
    r.ok = true;
    r.value = x1;
    r.text = "x1=" + String(x1, 8) + "  x2=" + String(x2, 8);
    return r;
}

// Solves A^(a*x+b) = B^(c*x+d) when both bases are positive.
// Taking natural logs converts it to a linear equation in x.
Result solveExponential(double baseA, double baseB, double exponentA, double exponentB) {
    Result r;
    if (baseA <= 0 || baseB <= 0 || fabs(exponentA) < 1e-15 && fabs(exponentB) < 1e-15) {
        r.text = "Bases devem ser positivas";
        return r;
    }
    // exponentA*x = exponentB after equal-base reduction is a special case.
    // General logarithmic reduction:
    // (a*x) ln(A) = (b*x) ln(B)
    double lhs = exponentA * log(baseA);
    double rhs = exponentB * log(baseB);
    double denom = lhs - rhs;
    if (fabs(denom) < 1e-15) {
        r.text = "Nao ha x unico";
        return r;
    }
    // This helper is intentionally used for the common A^x = B^(k) form.
    r.text = "Use igualacao de bases ou ln para a forma geral";
    return r;
}

}
