#include "Chemistry.h"
#include <cstring>

namespace Chemistry {

static const Element elements[] = {
    {1,  "H",  "Hidrogenio",   1,  1,  "nao metal",        "1s1"},
    {2,  "He", "Helio",        18,  1,  "gas nobre",        "1s2"},

    {3,  "Li", "Litio",         1,  2,  "metal alcalino",   "[He] 2s1"},
    {4,  "Be", "Berilio",       2,  2,  "alcalino-terroso", "[He] 2s2"},
    {5,  "B",  "Boro",         13,  2,  "metaloide",        "[He] 2s2 2p1"},
    {6,  "C",  "Carbono",      14,  2,  "nao metal",        "[He] 2s2 2p2"},
    {7,  "N",  "Nitrogenio",   15,  2,  "nao metal",        "[He] 2s2 2p3"},
    {8,  "O",  "Oxigenio",     16,  2,  "nao metal",        "[He] 2s2 2p4"},
    {9,  "F",  "Fluor",        17,  2,  "halogenio",        "[He] 2s2 2p5"},
    {10, "Ne", "Neonio",       18,  2,  "gas nobre",        "[He] 2s2 2p6"},

    {11, "Na", "Sodio",         1,  3,  "metal alcalino",   "[Ne] 3s1"},
    {12, "Mg", "Magnesio",      2,  3,  "alcalino-terroso", "[Ne] 3s2"},
    {13, "Al", "Aluminio",     13,  3,  "metal",            "[Ne] 3s2 3p1"},
    {14, "Si", "Silicio",      14,  3,  "metaloide",        "[Ne] 3s2 3p2"},
    {15, "P",  "Fosforo",       15,  3,  "nao metal",        "[Ne] 3s2 3p3"},
    {16, "S",  "Enxofre",       16,  3,  "nao metal",        "[Ne] 3s2 3p4"},
    {17, "Cl", "Cloro",         17,  3,  "halogenio",        "[Ne] 3s2 3p5"},
    {18, "Ar", "Argonio",       18,  3,  "gas nobre",        "[Ne] 3s2 3p6"},

    {19, "K",  "Potassio",       1,  4,  "metal alcalino",   "[Ar] 4s1"},
    {20, "Ca", "Calcio",         2,  4,  "alcalino-terroso", "[Ar] 4s2"},

    {26, "Fe", "Ferro",          8,  4,  "metal de transicao", "[Ar] 3d6 4s2"},
    {29, "Cu", "Cobre",         11,  4,  "metal de transicao", "[Ar] 3d10 4s1"},
    {30, "Zn", "Zinco",         12,  4,  "metal de transicao", "[Ar] 3d10 4s2"},

    {35, "Br", "Bromo",         17,  4,  "halogenio",        "[Ar] 3d10 4s2 4p5"},

    {47, "Ag", "Prata",         11,  5,  "metal de transicao", "[Kr] 4d10 5s1"},
    {53, "I",  "Iodo",          17,  5,  "halogenio",        "[Kr] 4d10 5s2 5p5"},

    {56, "Ba", "Bario",          2,  6,  "alcalino-terroso", "[Xe] 6s2"},
    {79, "Au", "Ouro",          11,  6,  "metal de transicao", "[Xe] 4f14 5d10 6s1"},
    {80, "Hg", "Mercurio",      12,  6,  "metal de transicao", "[Xe] 4f14 5d10 6s2"},
    {82, "Pb", "Chumbo",        14,  6,  "metal",            "[Xe] 4f14 5d10 6s2 6p2"},

    {92, "U",  "Uranio",         0,  7,  "actinideo",        "[Rn] 5f3 6d1 7s2"}
};

static const size_t elementCount =
    sizeof(elements) / sizeof(elements[0]);

const Element* findBySymbol(const String& symbol) {
    for (size_t i = 0; i < elementCount; ++i) {
        if (symbol.equalsIgnoreCase(elements[i].symbol)) {
            return &elements[i];
        }
    }

    return nullptr;
}

const Element* findByName(const String& name) {
    for (size_t i = 0; i < elementCount; ++i) {
        if (name.equalsIgnoreCase(elements[i].name)) {
            return &elements[i];
        }
    }

    return nullptr;
}

const Element* findByAtomicNumber(int number) {
    for (size_t i = 0; i < elementCount; ++i) {
        if (elements[i].atomicNumber == number) {
            return &elements[i];
        }
    }

    return nullptr;
}

String bondType(const Element& a, const Element& b) {

    // Comparacao segura entre const char* e texto.
    const bool aGasNobre =
        strcmp(a.category, "gas nobre") == 0;

    const bool bGasNobre =
        strcmp(b.category, "gas nobre") == 0;

    // Gases nobres normalmente nao formam ligacoes
    // quimicas comuns nas condicoes escolares.
    if (aGasNobre || bGasNobre) {
        return "Nenhuma ligacao comum";
    }

    const bool aMetal =
        strstr(a.category, "metal") != nullptr ||
        strstr(a.category, "alcalino") != nullptr ||
        strstr(a.category, "terroso") != nullptr ||
        strstr(a.category, "actinideo") != nullptr;

    const bool bMetal =
        strstr(b.category, "metal") != nullptr ||
        strstr(b.category, "alcalino") != nullptr ||
        strstr(b.category, "terroso") != nullptr ||
        strstr(b.category, "actinideo") != nullptr;

    // Metal + nao metal -> ionica.
    if (aMetal != bMetal) {
        return "Ligacao ionica";
    }

    // Metal + metal -> metalica.
    if (aMetal && bMetal) {
        return "Ligacao metalica";
    }

    // Nao metal + nao metal -> covalente.
    return "Ligacao covalente";
}

String octetNote(const Element& e) {

    // Hidrogenio e helio sao excecoes importantes:
    // a estabilidade da primeira camada ocorre com 2 eletrons.
    if (e.atomicNumber == 1) {
        return "H segue a regra do dueto: estabilidade com 2 eletrons.";
    }

    if (e.atomicNumber == 2) {
        return "He possui a primeira camada completa com 2 eletrons.";
    }

    // Alguns elementos nao seguem uma regra simples de octeto.
    if (e.atomicNumber == 5 ||
        e.atomicNumber == 6 ||
        e.atomicNumber == 7 ||
        e.atomicNumber == 8) {
        return "A regra do octeto e uma aproximacao; existem excecoes.";
    }

    return "A regra do octeto ajuda a interpretar muitas ligacoes, mas nao e universal.";
}

} // namespace Chemistry
