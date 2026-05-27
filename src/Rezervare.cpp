#include "Rezervare.h"
#include <iostream>

Rezervare::Rezervare(const std::string& client, const std::string& filmTitlu,
                     const std::string& metoda,
                     const std::vector<std::string>& locuri, int total)
    : client(client), filmTitlu(filmTitlu), metoda(metoda),
      locuri(locuri), total(total) {}

std::string Rezervare::getClient()    const { return client; }
std::string Rezervare::getFilmTitlu() const { return filmTitlu; }
std::string Rezervare::getMetoda()    const { return metoda; }
std::vector<std::string> Rezervare::getLocuri() const { return locuri; }
int Rezervare::getTotal() const { return total; }

void Rezervare::afiseaza() const {
    std::cout << "Rezervare: " << client
              << " | " << filmTitlu
              << " | " << metoda
              << " | " << total << " RON\n";
    std::cout << "  Locuri: ";
    for (int i = 0; i < (int)locuri.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << locuri[i];
    }
    std::cout << "\n";
}