#pragma once
#include <vector>
#include <string>
#include "Film.h"
#include "Sala.h"
#include "Rezervare.h"

// Cinematograf agregeaza Filme, Sali si Rezervari
class Cinematograf {
private:
    std::string nume;
    std::vector<Film*> filme;
    std::vector<Sala*> sali;
    std::vector<Rezervare> rezervari;

public:
    Cinematograf(const std::string& nume);
    ~Cinematograf();

    void adaugaFilm(Film* film);
    void adaugaSala(Sala* sala);
    void adaugaRezervare(const Rezervare& rez);

    Film* getFilm(int index) const;
    Sala* getSala(int index) const;
    Sala* getSalaForFilm(int filmIndex) const;
    Rezervare& getRezervare(int index);

    int getNumarFilme()    const;
    int getNumarSali()     const;
    int getNumarRezervari() const;

    std::vector<Rezervare>& getRezervari();

    void afisaFilme()           const;
    void afisaLocuriSala(int i) const;
};