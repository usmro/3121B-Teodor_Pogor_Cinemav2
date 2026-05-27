#pragma once
#include <vector>
#include <string>
#include <map>
#include "Film.h"
#include "Sala.h"
#include "Rezervare.h"

class Cinematograf {
private:
    std::string nome;
    std::vector<Film*>     filme;
    std::vector<Sala*>     sali;
    std::vector<Rezervare> rezervari;
    int nextId;

public:
    Cinematograf(const std::string& nume);
    ~Cinematograf();

    void adaugaFilm(Film* film);
    void adaugaSala(Sala* sala);

    // Adauga rezervare si returneaza id-ul ei
    int adaugaRezervare(const Rezervare& rez);

    // Returneaza urmatorul ID disponibil
    int getNextId() const;

    Film* getFilm(int index) const;
    Sala* getSala(int index) const;
    Sala* getSalaForFilm(int filmIndex) const;

    Rezervare&              getRezervare(int index);
    std::vector<Rezervare>& getRezervari();

    int getNumarFilme()     const;
    int getNumarSali()      const;
    int getNumarRezervari() const;

    // Anuleaza rezervarea cu ID-ul dat; returneaza true daca a reusit
    bool anuleazaRezervare(int id);

    // Statistici raport
    int getTotalIncasari() const;
    int getTotalBilete()   const;
    std::map<std::string, int> incasariPerFilm()  const;
    std::map<std::string, int> biletePeTipuri()   const;

    // Persistenta date
    void salveazaDate()  const;
    void incarcaDate();

    void afisaFilme()           const;
    void afisaLocuriSala(int i) const;
};