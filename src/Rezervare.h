#pragma once
#include <string>
#include <vector>

class Rezervare {
private:
    std::string client;
    std::string filmTitlu;
    std::string metoda; // cash sau card
    std::vector<std::string> locuri;
    int total;

public:
    Rezervare(const std::string& client, const std::string& filmTitlu,
              const std::string& metoda,
              const std::vector<std::string>& locuri, int total);

    std::string getClient()    const;
    std::string getFilmTitlu() const;
    std::string getMetoda()    const;
    std::vector<std::string> getLocuri() const;
    int getTotal() const;

    void afiseaza() const;
};