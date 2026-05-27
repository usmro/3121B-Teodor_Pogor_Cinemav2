#pragma once
#include <string>

class Film {
private:
    std::string titlu;
    std::string format;
    std::string gen;
    std::string ora;
    int durata;
    int nrSala;

public:
    Film(const std::string& titlu, const std::string& format,
         const std::string& gen, const std::string& ora,
         int durata, int nrSala);

    std::string getTitlu()  const;
    std::string getFormat() const;
    std::string getGen()    const;
    std::string getOra()    const;
    int         getDurata() const;
    int         getNrSala() const;

    // Pret de baza in functie de format
    int getPret() const;

    // Pret dinamic: +20% weekend, reduceri tip bilet (copil -30%, senior -20%)
    int getPretDinamic(bool weekend, const std::string& tipBilet) const;

    std::string getInfo()    const;
    void        afiseaza()   const;
};