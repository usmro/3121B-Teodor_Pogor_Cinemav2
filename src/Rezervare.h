#pragma once
#include <string>
#include <vector>

class Rezervare {
private:
    int         id;
    std::string client;
    std::string filmTitlu;
    std::string metoda;       // "cash" sau "card"
    std::string tipBilet;     // "adult", "copil", "senior"
    std::vector<std::string> locuri;
    int         total;
    std::string timestamp;
    int         nrSala;
    bool        anulata;

public:
    Rezervare(int id,
              const std::string& client,
              const std::string& filmTitlu,
              const std::string& metoda,
              const std::string& tipBilet,
              const std::vector<std::string>& locuri,
              int total,
              const std::string& timestamp,
              int nrSala);

    int                      getId()        const;
    std::string              getClient()    const;
    std::string              getFilmTitlu() const;
    std::string              getMetoda()    const;
    std::string              getTipBilet()  const;
    std::vector<std::string> getLocuri()    const;
    int                      getTotal()     const;
    std::string              getTimestamp() const;
    int                      getNrSala()    const;
    bool                     esteAnulata()  const;

    void anuleaza();
    void afiseaza() const;
};