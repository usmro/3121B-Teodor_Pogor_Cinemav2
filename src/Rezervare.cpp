#include "Rezervare.h"
#include <iostream>

Rezervare::Rezervare(int id,
                     const std::string& client,
                     const std::string& filmTitlu,
                     const std::string& metoda,
                     const std::string& tipBilet,
                     const std::vector<std::string>& locuri,
                     int total,
                     const std::string& timestamp,
                     int nrSala)
    : id(id), client(client), filmTitlu(filmTitlu), metoda(metoda),
      tipBilet(tipBilet), locuri(locuri), total(total),
      timestamp(timestamp), nrSala(nrSala), anulata(false) {}

int         Rezervare::getId()        const { return id; }
std::string Rezervare::getClient()    const { return client; }
std::string Rezervare::getFilmTitlu() const { return filmTitlu; }
std::string Rezervare::getMetoda()    const { return metoda; }
std::string Rezervare::getTipBilet()  const { return tipBilet; }
std::vector<std::string> Rezervare::getLocuri() const { return locuri; }
int         Rezervare::getTotal()     const { return total; }
std::string Rezervare::getTimestamp() const { return timestamp; }
int         Rezervare::getNrSala()    const { return nrSala; }
bool        Rezervare::esteAnulata()  const { return anulata; }

void Rezervare::anuleaza() { anulata = true; }

void Rezervare::afiseaza() const {
    std::cout << "Rezervare #" << id
              << ": " << client
              << " | " << filmTitlu
              << " | " << tipBilet
              << " | " << metoda
              << " | " << total << " RON"
              << (anulata ? " [ANULATA]" : "") << "\n";
    std::cout << "  Locuri: ";
    for (int i = 0; i < (int)locuri.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << locuri[i];
    }
    std::cout << "\n";
}