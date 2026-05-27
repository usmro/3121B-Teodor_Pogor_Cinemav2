#pragma once
#include <string>

class Film {
private:
    std::string titlu;
    std::string format; // 2D, 3D, IMAX, 4DX
    std::string gen;
    std::string ora;
    int durata;
    int nrSala;

public:
    Film(const std::string& titlu, const std::string& format,
         const std::string& gen, const std::string& ora,
         int durata, int nrSala);

    std::string getTitlu() const;
    std::string getFormat() const;
    std::string getGen() const;
    std::string getOra() const;
    int getDurata() const;
    int getNrSala() const;

    // Calcul pret in functie de tipul filmului (cerinta facultativa)
    int getPret() const;

    std::string getInfo() const;
    void afiseaza() const;
};