#pragma once
#include <vector>
#include <string>

class Sala {
public:
    enum StareLoc { LIBER, OCUPAT, SELECTAT };

private:
    int numar;
    int randuri;
    int coloane;
    std::vector<std::vector<StareLoc>> locuri; // matrice de locuri

public:
    Sala(int numar, int randuri, int coloane);

    int getNumar()   const;
    int getRanduri() const;
    int getColoane() const;

    StareLoc getStare(int rand, int col) const;

    // Arunca std::out_of_range daca index invalid
    // Arunca std::runtime_error daca locul e deja ocupat
    void ocupaLoc(int rand, int col);

    void selecteazaLoc(int rand, int col);
    void reseteazaSelectie();

    bool esteLiber(int rand, int col) const;
    int numarLocuriLibere() const;

    std::vector<std::pair<int,int>> getLocuriSelectate() const;

    void afisaLocuri() const;
};