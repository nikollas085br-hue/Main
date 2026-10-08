#include "MathEngine.h"
#include <math.h>
#include <stdlib.h>

namespace MathEngine {
class Parser {
    String s; size_t p=0; double xv=0;
    void skip(){while(p<s.length() && s[p]==' ')p++;}
    bool eat(char c){skip(); if(p<s.length()&&s[p]==c){p++;return true;} return false;}
    double number(){skip(); size_t st=p; while(p<s.length()&&(isDigit(s[p])||s[p]=='.'))p++; if(st==p) return NAN; return s.substring(st,p).toDouble();}
    String ident(){skip(); size_t st=p; while(p<s.length()&&(isAlphaNumeric(s[p])||s[p]=='_'))p++; return s.substring(st,p);}
    double primary(){
        skip(); if(eat('(')){double v=expr(); if(!eat(')')) return NAN; return v;}
        if(p<s.length()&&(s[p]=='+'||s[p]=='-')){char c=s[p++]; double v=primary(); return c=='-'?-v:v;}
        if(p<s.length()&&isDigit(s[p])) return number();
        if(p<s.length()&&(isAlpha(s[p])||s[p]=='_')){
            String id=ident(); id.toLowerCase();
            if(id=="x") return xv; if(id=="pi") return PI; if(id=="e") return M_E;
            if(!eat('(')) return NAN; double a=expr(); if(!eat(')')) return NAN;
            if(id=="sin")return sin(a); if(id=="cos")return cos(a); if(id=="tan")return tan(a);
            if(id=="asin")return asin(a); if(id=="acos")return acos(a); if(id=="atan")return atan(a);
            if(id=="sqrt")return sqrt(a); if(id=="abs")return fabs(a); if(id=="ln")return log(a);
            if(id=="log")return log10(a); if(id=="exp")return exp(a); if(id=="floor")return floor(a); if(id=="ceil")return ceil(a);
            if(id=="sinh")return sinh(a); if(id=="cosh")return cosh(a); if(id=="tanh")return tanh(a);
            return NAN;
        }
        return NAN;
    }
    double power(){double v=primary(); while(true){skip(); if(!eat('^'))break; double r=primary(); v=pow(v,r);} return v;}
    double term(){double v=power(); while(true){skip(); if(eat('*'))v*=power(); else if(eat('/')){double d=power();v/=d;} else break;} return v;}
    double expr(){double v=term(); while(true){skip(); if(eat('+'))v+=term(); else if(eat('-'))v-=term(); else break;} return v;}
public: Parser(const String&a,double x):s(a),xv(x){} double run(){double v=expr();skip();return p==s.length()?v:NAN;}
};
static String fmt(double v){ if(!isfinite(v))return "indefinido"; if(fabs(v-round(v))<1e-10)return String((long long)round(v)); return String(v,8); }
Result evaluate(const String& e,double x){ if(e.length()==0)return {false,NAN,"Digite uma expressao."}; Parser p(e,x); double v=p.run(); if(!isfinite(v))return {false,v,"Expressao invalida."}; return {true,v,fmt(v)}; }
Result solveLinear(double a,double b){ if(fabs(a)<1e-12)return {false,NAN,fabs(b)<1e-12?"Infinitas solucoes.":"Sem solucao."}; double x=-b/a; return {true,x,"x = "+fmt(x)}; }
Result solveQuadratic(double a,double b,double c){ if(fabs(a)<1e-12)return solveLinear(b,c); double d=b*b-4*a*c; if(d<0)return {false,NAN,"Sem raizes reais. Delta = "+fmt(d)}; double r=sqrt(d); double x1=(-b+r)/(2*a),x2=(-b-r)/(2*a); if(fabs(x1-x2)<1e-10)return {true,x1,"x = "+fmt(x1)}; return {true,x1,"x1 = "+fmt(x1)+"  x2 = "+fmt(x2)}; }
Result solveExponential(double a,double b,double c,double d){
    if(a<=0||a==1||d<=0||fabs(b)<1e-12)return {false,NAN,"Base deve ser >0 e !=1; d>0; b!=0."};
    double x=(log(d)/log(a)-c)/b;
    String strategy="Use log: x=(ln(d)/ln(a)-c)/b";
    return {true,x,"x = "+fmt(x)+" | "+strategy};
}
String describeExpression(const String& e){return "f(x) = "+e+"\nUse x para variavel.";}
}
