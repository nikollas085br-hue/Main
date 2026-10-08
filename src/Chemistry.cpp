#include "Chemistry.h"
#include <ctype.h>
#include <string.h>

namespace {
static const Chemistry::Element E[] = {
{"H","Hidrogenio",1,"1","1","nao-metal","1s1"},
{"He","Helio",2,"18","1","gas nobre","1s2"},
{"Li","Litio",3,"1","2","metal alcalino","[He] 2s1"},
{"Be","Berilio",4,"2","2","alcalino-terroso","[He] 2s2"},
{"B","Boro",5,"13","2","metaloide","[He] 2s2 2p1"},
{"C","Carbono",6,"14","2","nao-metal","[He] 2s2 2p2"},
{"N","Nitrogenio",7,"15","2","nao-metal","[He] 2s2 2p3"},
{"O","Oxigenio",8,"16","2","nao-metal","[He] 2s2 2p4"},
{"F","Fluor",9,"17","2","halogenio","[He] 2s2 2p5"},
{"Ne","Neonio",10,"18","2","gas nobre","[He] 2s2 2p6"},
{"Na","Sodio",11,"1","3","metal alcalino","[Ne] 3s1"},
{"Mg","Magnesio",12,"2","3","alcalino-terroso","[Ne] 3s2"},
{"Al","Aluminio",13,"13","3","pos-transicao","[Ne] 3s2 3p1"},
{"Si","Silicio",14,"14","3","metaloide","[Ne] 3s2 3p2"},
{"P","Fosforo",15,"15","3","nao-metal","[Ne] 3s2 3p3"},
{"S","Enxofre",16,"16","3","nao-metal","[Ne] 3s2 3p4"},
{"Cl","Cloro",17,"17","3","halogenio","[Ne] 3s2 3p5"},
{"Ar","Argonio",18,"18","3","gas nobre","[Ne] 3s2 3p6"},
{"K","Potassio",19,"1","4","metal alcalino","[Ar] 4s1"},
{"Ca","Calcio",20,"2","4","alcalino-terroso","[Ar] 4s2"},
{"Fe","Ferro",26,"8","4","metal de transicao","[Ar] 3d6 4s2"},
{"Cu","Cobre",29,"11","4","metal de transicao","[Ar] 3d10 4s1"},
{"Zn","Zinco",30,"12","4","metal de transicao","[Ar] 3d10 4s2"},
{"Br","Bromo",35,"17","4","halogenio","[Ar] 3d10 4s2 4p5"},
{"Ag","Prata",47,"11","5","metal de transicao","[Kr] 4d10 5s1"},
{"I","Iodo",53,"17","5","halogenio","[Kr] 4d10 5s2 5p5"},
{"Ba","Bario",56,"2","6","alcalino-terroso","[Xe] 6s2"},
{"Au","Ouro",79,"11","6","metal de transicao","[Xe] 4f14 5d10 6s1"},
{"Hg","Mercurio",80,"12","6","metal de transicao","[Xe] 4f14 5d10 6s2"},
{"Pb","Chumbo",82,"14","6","pos-transicao","[Xe] 4f14 5d10 6s2 6p2"},
{"U","Uranio",92,"act","7","actinideo","[Rn] 5f3 6d1 7s2"}
};
}

namespace Chemistry {
const Element* byAtomicNumber(uint8_t z) {
    for (auto &x : E) if (x.z == z) return &x;
    return nullptr;
}

const Element* find(const String& q) {
    String s = q; s.trim();
    for (auto &x : E) {
        if (s.equalsIgnoreCase(x.symbol) || s.equalsIgnoreCase(x.name)) return &x;
    }
    int z = s.toInt();
    return z > 0 ? byAtomicNumber((uint8_t)z) : nullptr;
}

String bondType(const Element& a, const Element& b) {
    bool am = (String(a.category).indexOf("metal") >= 0);
    bool bm = (String(b.category).indexOf("metal") >= 0);
    bool ag = a.category == String("gas nobre");
    bool bg = b.category == String("gas nobre");
    if (ag || bg) return "Gas nobre: geralmente nao forma ligacao comum.";
    if (am && bm) return "Metalica (em uma rede metalica).";
    if (am != bm) return "Ionica (modelo por transferencia de eletrons).";
    return "Covalente (modelo por compartilhamento).";
}

String octetNote(const Element& e) {
    if (e.z == 1) return "Excecao importante: H busca estabilidade com 2 eletrons (dueto).";
    if (e.z == 2) return "He ja e estavel com 2 eletrons.";
    if (e.z == 6 || e.z == 7 || e.z == 8 || e.z == 9 || e.z == 17)
        return "Regra do octeto e um modelo util para este elemento em muitos compostos.";
    return "Octeto e uma regra de modelo; ha excecoes e casos de valencia variavel.";
}
}
