#include "Chemistry.h"
#include <Arduino.h>

namespace Chemistry {

static const Element elements[] = {
    {"H",  "Hidrogenio",       1,  "1",  "1", "Nao metal",              "1s1"},
    {"He", "Helio",             2,  "18", "1", "Gas nobre",              "1s2"},

    {"Li", "Litio",             3,  "1",  "2", "Metal alcalino",         "[He] 2s1"},
    {"Be", "Berilio",           4,  "2",  "2", "Metal alcalino-terroso", "[He] 2s2"},
    {"B",  "Boro",               5,  "13", "2", "Semimetal",              "[He] 2s2 2p1"},
    {"C",  "Carbono",            6,  "14", "2", "Nao metal",              "[He] 2s2 2p2"},
    {"N",  "Nitrogenio",         7,  "15", "2", "Nao metal",              "[He] 2s2 2p3"},
    {"O",  "Oxigenio",           8,  "16", "2", "Nao metal",              "[He] 2s2 2p4"},
    {"F",  "Fluor",              9,  "17", "2", "Halogenio",               "[He] 2s2 2p5"},
    {"Ne", "Neonio",            10,  "18", "2", "Gas nobre",              "[He] 2s2 2p6"},

    {"Na", "Sodio",             11,  "1",  "3", "Metal alcalino",         "[Ne] 3s1"},
    {"Mg", "Magnesio",          12,  "2",  "3", "Metal alcalino-terroso", "[Ne] 3s2"},
    {"Al", "Aluminio",          13,  "13", "3", "Metal",                   "[Ne] 3s2 3p1"},
    {"Si", "Silicio",           14,  "14", "3", "Semimetal",              "[Ne] 3s2 3p2"},
    {"P",  "Fosforo",           15,  "15", "3", "Nao metal",              "[Ne] 3s2 3p3"},
    {"S",  "Enxofre",           16,  "16", "3", "Nao metal",              "[Ne] 3s2 3p4"},
    {"Cl", "Cloro",             17,  "17", "3", "Halogenio",               "[Ne] 3s2 3p5"},
    {"Ar", "Argonio",            18,  "18", "3", "Gas nobre",              "[Ne] 3s2 3p6"},

    {"K",  "Potassio",          19,  "1",  "4", "Metal alcalino",         "[Ar] 4s1"},
    {"Ca", "Calcio",            20,  "2",  "4", "Metal alcalino-terroso", "[Ar] 4s2"},
    {"Sc", "Escandio",          21,  "3",  "4", "Metal de transicao",     "[Ar] 3d1 4s2"},
    {"Ti", "Titanio",           22,  "4",  "4", "Metal de transicao",     "[Ar] 3d2 4s2"},
    {"V",  "Vanadio",           23,  "5",  "4", "Metal de transicao",     "[Ar] 3d3 4s2"},
    {"Cr", "Cromo",             24,  "6",  "4", "Metal de transicao",     "[Ar] 3d5 4s1"},
    {"Mn", "Manganes",          25,  "7",  "4", "Metal de transicao",     "[Ar] 3d5 4s2"},
    {"Fe", "Ferro",             26,  "8",  "4", "Metal de transicao",     "[Ar] 3d6 4s2"},
    {"Co", "Cobalto",           27,  "9",  "4", "Metal de transicao",     "[Ar] 3d7 4s2"},
    {"Ni", "Niquel",            28,  "10", "4", "Metal de transicao",     "[Ar] 3d8 4s2"},
    {"Cu", "Cobre",             29,  "11", "4", "Metal de transicao",     "[Ar] 3d10 4s1"},
    {"Zn", "Zinco",             30,  "12", "4", "Metal de transicao",     "[Ar] 3d10 4s2"},
    {"Ga", "Galio",             31,  "13", "4", "Metal",                   "[Ar] 3d10 4s2 4p1"},
    {"Ge", "Germanio",          32,  "14", "4", "Semimetal",              "[Ar] 3d10 4s2 4p2"},
    {"As", "Arsenio",           33,  "15", "4", "Semimetal",              "[Ar] 3d10 4s2 4p3"},
    {"Se", "Selenio",           34,  "16", "4", "Nao metal",              "[Ar] 3d10 4s2 4p4"},
    {"Br", "Bromo",             35,  "17", "4", "Halogenio",               "[Ar] 3d10 4s2 4p5"},
    {"Kr", "Criptonio",         36,  "18", "4", "Gas nobre",              "[Ar] 3d10 4s2 4p6"},

    {"Rb", "Rubidio",           37,  "1",  "5", "Metal alcalino",         "[Kr] 5s1"},
    {"Sr", "Estroncio",         38,  "2",  "5", "Metal alcalino-terroso", "[Kr] 5s2"},
    {"Ag", "Prata",             47,  "11", "5", "Metal de transicao",     "[Kr] 4d10 5s1"},
    {"Cd", "Cadmio",            48,  "12", "5", "Metal de transicao",     "[Kr] 4d10 5s2"},
    {"I",  "Iodo",              53,  "17", "5", "Halogenio",               "[Kr] 4d10 5s2 5p5"},
    {"Xe", "Xenonio",           54,  "18", "5", "Gas nobre",              "[Kr] 4d10 5s2 5p6"},

    {"Cs", "Cesio",             55,  "1",  "6", "Metal alcalino",         "[Xe] 6s1"},
    {"Ba", "Bario",             56,  "2",  "6", "Metal alcalino-terroso", "[Xe] 6s2"},
    {"Au", "Ouro",              79,  "11", "6", "Metal de transicao",     "[Xe] 4f14 5d10 6s1"},
    {"Hg", "Mercurio",          80,  "12", "6", "Metal de transicao",     "[Xe] 4f14 5d10 6s2"},
    {"Pb", "Chumbo",            82,  "14", "6", "Metal",                   "[Xe] 4f14 5d10 6s2 6p2"},
    {"Bi", "Bismuto",           83,  "15", "6", "Metal",                   "[Xe] 4f14 5d10 6s2 6p3"},
    {"Rn", "Radonio",           86,  "18", "6", "Gas nobre",              "[Xe] 4f14 5d10 6s2 6p6"},

    {"Fr", "Francio",            87, "1",  "7", "Metal alcalino",         "[Rn] 7s1"},
    {"Ra", "Radio",              88, "2",  "7", "Metal alcalino-terroso", "[Rn] 7s2"},
    {"U",  "Uranio",             92, "act", "7", "Actinideo",             "[Rn] 5f3 6d1 7s2"}
};

static const size_t ELEMENT_COUNT =
    sizeof(elements) / sizeof(elements[0]);

const Element* byAtomicNumber(uint8_t z) {
    for (size_t i = 0; i < ELEMENT_COUNT; ++i) {
        if (elements[i].z == z) {
            return &elements[i];
        }
    }

    return nullptr;
}

const Element* find(const String& query) {
    String q = query;
    q.trim();
    q.toLowerCase();

    if (q.length() == 0) {
        return nullptr;
    }

    // Primeiro tenta pelo símbolo ou nome.
    for (size_t i = 0; i < ELEMENT_COUNT; ++i) {
        String symbol = elements[i].symbol;
        String name = elements[i].name;

        symbol.toLowerCase();
        name.toLowerCase();

        if (q == symbol || q == name) {
            return &elements[i];
        }
    }

    // Depois tenta pelo número atômico.
    bool numeric = true;

    for (size_t i = 0; i < q.length(); ++i) {
        if (!isDigit(q[i])) {
            numeric = false;
            break;
        }
    }

    if (numeric) {
        int z = q.toInt();

        if (z >= 1 && z <= 118) {
            return byAtomicNumber((uint8_t)z);
        }
    }

    return nullptr;
}

String bondType(const Element& a, const Element& b) {
    String ca = a.category;
    String cb = b.category;

    ca.toLowerCase();
    cb.toLowerCase();

    bool metalA =
        ca.indexOf("metal") >= 0;

    bool metalB =
        cb.indexOf("metal") >= 0;

    bool nonMetalA =
        ca.indexOf("nao metal") >= 0 ||
        ca.indexOf("halogenio") >= 0;

    bool nonMetalB =
        cb.indexOf("nao metal") >= 0 ||
        cb.indexOf("halogenio") >= 0;

    // Metal + metal -> ligação metálica
    if (metalA && metalB) {
        return "Metalica";
    }

    // Metal + não metal -> ligação iônica
    if ((metalA && nonMetalB) ||
        (metalB && nonMetalA)) {
        return "Ionica";
    }

    // Não metal + não metal -> ligação covalente
    if (nonMetalA && nonMetalB) {
        return "Covalente";
    }

    // Semimetais podem participar de ligações covalentes.
    if (!metalA && !metalB) {
        return "Covalente";
    }

    return "Nao determinada";
}

String octetNote(const Element& e) {
    // H e He são exceções importantes:
    // a camada K comporta apenas 2 elétrons.
    if (e.z == 1 || e.z == 2) {
        return "Regra do dueto: a camada K fica estavel com 2 eletrons.";
    }

    // Gases nobres normalmente já possuem a camada de valência completa.
    String category = e.category;
    category.toLowerCase();

    if (category.indexOf("gas nobre") >= 0) {
        return "Gas nobre: camada de valencia completa; alta estabilidade.";
    }

    return "Regra do octeto: muitos atomos tendem a atingir 8 eletrons "
           "na camada de valencia. Existem excecoes.";
}

} // namespace Chemistry
