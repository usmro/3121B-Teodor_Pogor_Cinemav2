#include "Sala.h"
#include <iostream>
#include <stdexcept>

Sala::Sala(int numar, int randuri, int coloane, int randuriVIP)
    : numar(numar), randuri(randuri), coloane(coloane), randuriVIP(randuriVIP) {
    locuri.resize(randuri, std::vector<StareLoc>(coloane));
    for (int r = 0; r < randuri; r++) {
        StareLoc stareInitiala = (r < randuriVIP) ? VIP_LIBER : LIBER;
        for (int c = 0; c < coloane; c++)
            locuri[r][c] = stareInitiala;
    }
}

int Sala::getNumar()      const { return numar; }
int Sala::getRanduri()    const { return randuri; }
int Sala::getColoane()    const { return coloane; }
int Sala::getRanduriVIP() const { return randuriVIP; }

bool Sala::esteVIP(int rand) const {
    return rand < randuriVIP;
}

Sala::StareLoc Sala::getStare(int rand, int col) const {
    return locuri[rand][col];
}

void Sala::ocupaLoc(int rand, int col) {
    if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
        throw std::out_of_range("Index loc invalid: R" +
            std::to_string(rand+1) + "C" + std::to_string(col+1));
    StareLoc& loc = locuri[rand][col];
    if (loc == OCUPAT || loc == VIP_OCUPAT)
        throw std::runtime_error("Locul R" + std::to_string(rand+1) +
            "C" + std::to_string(col+1) + " este deja ocupat!");
    loc = esteVIP(rand) ? VIP_OCUPAT : OCUPAT;
}

void Sala::selecteazaLoc(int rand, int col) {
    if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
        throw std::out_of_range("Index loc invalid!");
    StareLoc& loc = locuri[rand][col];
    if (loc == OCUPAT || loc == VIP_OCUPAT)
        throw std::runtime_error("Locul R" + std::to_string(rand+1) +
            "C" + std::to_string(col+1) + " este deja ocupat!");
    if (loc == LIBER)         loc = SELECTAT;
    else if (loc == SELECTAT) loc = LIBER;
    else if (loc == VIP_LIBER)    loc = VIP_SELECTAT;
    else if (loc == VIP_SELECTAT) loc = VIP_LIBER;
}

void Sala::reseteazaSelectie() {
    for (auto& r : locuri)
        for (auto& loc : r) {
            if (loc == SELECTAT)     loc = LIBER;
            if (loc == VIP_SELECTAT) loc = VIP_LIBER;
        }
}

bool Sala::esteLiber(int rand, int col) const {
    return locuri[rand][col] == LIBER || locuri[rand][col] == VIP_LIBER;
}

int Sala::numarLocuriLibere() const {
    int cnt = 0;
    for (auto& r : locuri)
        for (auto& loc : r)
            if (loc == LIBER || loc == VIP_LIBER) cnt++;
    return cnt;
}

std::vector<std::pair<int,int>> Sala::getLocuriSelectate() const {
    std::vector<std::pair<int,int>> sel;
    for (int r = 0; r < randuri; r++)
        for (int c = 0; c < coloane; c++)
            if (locuri[r][c] == SELECTAT || locuri[r][c] == VIP_SELECTAT)
                sel.push_back({r, c});
    return sel;
}

void Sala::afisaLocuri() const {
    std::cout << "\n=== Sala " << numar << " ===\n";
    std::cout << "     ";
    for (int c = 0; c < coloane; c++)
        std::cout << " C" << c+1;
    std::cout << "\n";
    for (int r = 0; r < randuri; r++) {
        std::cout << (esteVIP(r) ? " VIP " : "  R" + std::to_string(r+1) + " ");
        for (int c = 0; c < coloane; c++) {
            StareLoc s = locuri[r][c];
            char ch = (s == OCUPAT || s == VIP_OCUPAT) ? 'X' : 'O';
            std::cout << " [" << ch << "]";
        }
        std::cout << "\n";
    }
    std::cout << "  O = liber   X = ocupat\n";
}