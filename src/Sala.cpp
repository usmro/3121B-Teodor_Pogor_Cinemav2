#include "Sala.h"
#include <iostream>
#include <stdexcept>

Sala::Sala(int numar, int randuri, int coloane)
    : numar(numar), randuri(randuri), coloane(coloane) {
    locuri.assign(randuri, std::vector<StareLoc>(coloane, LIBER));
}

int Sala::getNumar()   const { return numar; }
int Sala::getRanduri() const { return randuri; }
int Sala::getColoane() const { return coloane; }

Sala::StareLoc Sala::getStare(int rand, int col) const {
    return locuri[rand][col];
}

void Sala::ocupaLoc(int rand, int col) {
    if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
        throw std::out_of_range("Index loc invalid: R" +
            std::to_string(rand+1) + "C" + std::to_string(col+1));
    if (locuri[rand][col] == OCUPAT)
        throw std::runtime_error("Locul R" + std::to_string(rand+1) +
            "C" + std::to_string(col+1) + " este deja ocupat!");
    locuri[rand][col] = OCUPAT;
}

void Sala::selecteazaLoc(int rand, int col) {
    if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
        throw std::out_of_range("Index loc invalid!");
    if (locuri[rand][col] == LIBER)
        locuri[rand][col] = SELECTAT;
    else if (locuri[rand][col] == SELECTAT)
        locuri[rand][col] = LIBER;
}

void Sala::reseteazaSelectie() {
    for (auto& r : locuri)
        for (auto& loc : r)
            if (loc == SELECTAT) loc = LIBER;
}

bool Sala::esteLiber(int rand, int col) const {
    return locuri[rand][col] == LIBER;
}

int Sala::numarLocuriLibere() const {
    int cnt = 0;
    for (auto& r : locuri)
        for (auto& loc : r)
            if (loc == LIBER) cnt++;
    return cnt;
}

std::vector<std::pair<int,int>> Sala::getLocuriSelectate() const {
    std::vector<std::pair<int,int>> sel;
    for (int r = 0; r < randuri; r++)
        for (int c = 0; c < coloane; c++)
            if (locuri[r][c] == SELECTAT)
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
        std::cout << "  R" << r+1 << " ";
        for (int c = 0; c < coloane; c++)
            std::cout << " [" << (locuri[r][c]==OCUPAT ? "X" : "O") << "]";
        std::cout << "\n";
    }
    std::cout << "  O = liber   X = ocupat\n";
}