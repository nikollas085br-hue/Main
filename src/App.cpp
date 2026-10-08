#include "App.h"
#include "MathEngine.h"
#include "Physics.h"
#include "Chemistry.h"
#include "SDManager.h"
#include "WifiManager.h"
#include <M5Cardputer.h>

namespace {
enum Screen {
    HOME, MATH, MATH_SOLVER, PHYSICS, CHEMISTRY, PERIODIC, BIOLOGY, FILES, WIFI, NOTES
};

Screen screen = HOME;
int selected = 0;
String input;
String message;
bool dirty = true;
uint32_t lastDraw = 0;

const char* homeItems[] = {
    "MATEMATICA", "FISICA", "QUIMICA", "TABELA PERIODICA",
    "BIOLOGIA / SD", "ARQUIVOS", "CONEXAO", "ANOTACOES"
};

void clear() {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(1);
}

void title(const char* t) {
    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.drawString(t, 8, 8);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.drawFastHLine(8, 23, 224, TFT_DARKGREY);
}

void line(const String& s, int y, bool hi=false) {
    M5Cardputer.Display.setTextColor(hi ? TFT_BLACK : TFT_WHITE);
    if (hi) M5Cardputer.Display.fillRect(5, y-2, 230, 17, TFT_CYAN);
    M5Cardputer.Display.drawString(s, 10, y);
}

void drawHome() {
    clear(); title("CENTRAL DE ESTUDOS");
    M5Cardputer.Display.setTextColor(TFT_DARKGREY);
    M5Cardputer.Display.drawString("Setas/WASD + ENTER | BACKSPACE volta", 8, 28);
    for (int i=0;i<8;i++) line(homeItems[i], 48+i*20, i==selected);
    M5Cardputer.Display.setTextColor(TFT_GREEN);
    M5Cardputer.Display.drawString(SDManager::ready() ? "SD: OK" : "SD: --", 8, 212);
}

void drawMath() {
    clear(); title("MATEMATICA");
    line("CALCULADORA", 42, selected==0);
    line("EQUACAO 1o GRAU", 62, selected==1);
    line("EQUACAO 2o GRAU", 82, selected==2);
    line("EXPONENCIAL", 102, selected==3);
    line("TRIG / LOG / RAIZ", 122, selected==4);
    line("GRAFICO / FUNCAO", 142, selected==5);
    line("Expressao: " + input, 174);
    line(message, 196);
}

void drawGeneric(const char* t, const char* a, const char* b, const char* c) {
    clear(); title(t);
    line(a, 48, selected==0);
    line(b, 70, selected==1);
    line(c, 92, selected==2);
    line("ENTER seleciona | ← volta", 190);
    line(message, 164);
}

void draw() {
    if (!dirty) return;
    dirty=false;
    if (screen==HOME) drawHome();
    else if (screen==MATH || screen==MATH_SOLVER) drawMath();
    else if (screen==PHYSICS) drawGeneric("FISICA - SUPOSICAO",
        "Forca / massa / aceleracao", "Velocidade / distancia / tempo", "Energia / potencia / densidade");
    else if (screen==CHEMISTRY) drawGeneric("QUIMICA",
        "Buscar elemento", "Distribuicao eletronica", "Tipo de ligacao");
    else if (screen==PERIODIC) drawGeneric("TABELA PERIODICA",
        "H  He  Li Be B C N O F Ne", "Na Mg Al Si P S Cl Ar", "Fe Cu Zn Br Ag I Au Hg Pb U");
    else if (screen==BIOLOGY) drawGeneric("BIOLOGIA / SD",
        "Cadernos e notas", "Diagramas e imagens", "Materiais enviados");
    else if (screen==FILES) drawGeneric("ARQUIVOS / SD",
        "Anotacoes", "Imagens", "PDF / PPT / PPTX (armazenamento)");
    else if (screen==WIFI) drawGeneric("CONEXAO",
        "Rede: Cardputer-Estudos", "Acesse: 192.168.4.1", "Envie arquivos para /anotacoes");
    else if (screen==NOTES) drawGeneric("ANOTACOES",
        "Nova anotacao", "Ler anotacoes", "Enviar pelo celular");
}

void inputChar(char c) {
    if (c == '\b') { if (input.length()) input.remove(input.length()-1); }
    else if (c >= 32 && c <= 126) input += c;
    dirty=true;
}

void selectHome() {
    switch(selected) {
        case 0: screen=MATH; selected=0; break;
        case 1: screen=PHYSICS; selected=0; break;
        case 2: screen=CHEMISTRY; selected=0; break;
        case 3: screen=PERIODIC; selected=0; break;
        case 4: screen=BIOLOGY; selected=0; break;
        case 5: screen=FILES; selected=0; break;
        case 6: screen=WIFI; selected=0; break;
        case 7: screen=NOTES; selected=0; break;
    }
    input=""; message=""; dirty=true;
}

void enterMath() {
    if (selected==0) {
        auto r=MathEngine::evaluate(input);
        message=r.text;
    } else if (selected==2) {
        message="Digite na forma a,b,c no teclado e use ENTER.";
        int p1=input.indexOf(',');
        int p2=input.indexOf(',',p1+1);
        if (p1>0 && p2>p1) {
            double a=input.substring(0,p1).toDouble();
            double b=input.substring(p1+1,p2).toDouble();
            double c=input.substring(p2+1).toDouble();
            message=MathEngine::solveQuadratic(a,b,c).text;
        }
    } else if (selected==3) {
        message="A^x = B^y: iguale bases quando possivel; caso contrario use ln.";
    }
    dirty=true;
}

void keyEvent() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
    auto k=M5Cardputer.Keyboard.keysState();

    // Direct-arrow policy:
    // On the stock Cardputer keyboard, the four physical keycaps that also
    // represent arrows are: ; (UP), , (LEFT), . (DOWN), / (RIGHT).
    // In menus we interpret those physical keys directly, WITHOUT Fn.
    // In text-entry mode they remain normal punctuation so mathematical
    // expressions can still be typed.
    bool textMode = (screen==MATH || screen==MATH_SOLVER) && selected==0;

    if (!textMode) {
        if (k.up)    { selected--; dirty=true; }
        if (k.down)  { selected++; dirty=true; }
        if (k.left)  { selected--; dirty=true; }
        if (k.right) { selected++; dirty=true; }

        for (auto ch : k.word) {
            if (ch==';') { selected--; dirty=true; }
            else if (ch==',') { selected--; dirty=true; }
            else if (ch=='.') { selected++; dirty=true; }
            else if (ch=='/') { selected++; dirty=true; }
        }
    }

    // Keep WASD as a secondary direct navigation option, also without Fn.
    // It is intentionally ignored while entering a math expression.
    if (!textMode) {
        for (auto ch : k.word) {
            if (ch=='w' || ch=='W') { selected--; dirty=true; }
            else if (ch=='s' || ch=='S') { selected++; dirty=true; }
        }
    }

    int max = 7;
    if (screen==MATH) max=5;
    if (selected<0) selected=max;
    if (selected>max) selected=0;

    // Text entry is only active in the calculator.
    if (textMode) {
        for (auto ch : k.word) inputChar(ch);
    }

    if (k.backspace) {
        if (textMode) {
            if (input.length()) input.remove(input.length()-1);
            dirty=true;
        } else {
            screen=HOME; selected=0; input=""; message=""; dirty=true;
        }
    }

    if (k.del) {
        screen=HOME; selected=0; input=""; message=""; dirty=true;
    }

    if (k.enter) {
        if (screen==HOME) selectHome();
        else if (screen==MATH) enterMath();
    }
}
}

namespace App {
void begin() {
    SDManager::begin();
    if (SDManager::ready()) {
        SD.mkdir("/anotacoes");
        SD.mkdir("/conteudos");
    }
    WifiManager::begin();
    dirty=true;
    draw();
}

void update() {
    keyEvent();
    WifiManager::update();
    draw();
}
}
