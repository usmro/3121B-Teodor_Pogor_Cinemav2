#include "Cinematograf.h"
#include <iostream>
#include <stdexcept>

Cinematograf::Cinematograf(const std::string& nume) : nume(nume) {}

Cinematograf::~Cinematograf() {
    for (Film* f : filme) delete f;
    for (Sala* s : sali)  delete s;
}

void Cinematograf::adaugaFilm(Film* film) { filme.push_back(film); }
void Cinematograf::adaugaSala(Sala* sala) { sali.push_back(sala); }
void Cinematograf::adaugaRezervare(const Rezervare& rez) {
    rezervari.push_back(rez);
}

Film* Cinematograf::getFilm(int index) const {
    if (index < 0 || index >= (int)filme.size())
        throw std::out_of_range("Index film invalid!");
    return filme[index];
}

Sala* Cinematograf::getSala(int index) const {
    if (index < 0 || index >= (int)sali.size())
        throw std::out_of_range("Index sala invalid!");
    return sali[index];
}

Sala* Cinematograf::getSalaForFilm(int filmIndex) const {
    int nrSala = getFilm(filmIndex)->getNrSala();
    for (Sala* s : sali)
        if (s->getNumar() == nrSala) return s;
    throw std::runtime_error("Sala negasita pentru filmul selectat!");
}

Rezervare& Cinematograf::getRezervare(int index) {
    return rezervari[index];
}

int Cinematograf::getNumarFilme()     const { return filme.size(); }
int Cinematograf::getNumarSali()      const { return sali.size(); }
int Cinematograf::getNumarRezervari() const { return rezervari.size(); }

std::vector<Rezervare>& Cinematograf::getRezervari() { return rezervari; }

void Cinematograf::afisaFilme() const {
    std::cout << "\n=== Filme la " << nume << " ===\n";
    for (int i = 0; i < (int)filme.size(); i++) {
        std::cout << i+1 << ". ";
        filme[i]->afiseaza();
    }
}

void Cinematograf::afisaLocuriSala(int i) const {
    getSala(i)->afisaLocuri();
}