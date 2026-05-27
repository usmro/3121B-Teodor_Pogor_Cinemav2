#pragma once
#include <vector>
#include <string>

class Sala {
public:
    enum StareLoc { LIBER, OCUPAT, SELECTAT, VIP_LIBER, VIP_OCUPAT, VIP_SELECTAT };

private:
    int numar;
    int randuri;
    int coloane;
    int randuriVIP;   // primele N randuri sunt VIP
    std::vector<std::vector<StareLoc>> locuri;

public:
    // randuriVIP = 0 inseamna fara VIP
    Sala(int numar, int randuri, int coloane, int randuriVIP = 0);

    int getNumar()    const;
    int getRanduri()  const;
    int getColoane()  const;
    int getRanduriVIP() const;

    bool esteVIP(int rand) const;

    StareLoc getStare(int rand, int col) const;

    void ocupaLoc(int rand, int col);
    void selecteazaLoc(int rand, int col);
    void reseteazaSelectie();

    bool esteLiber(int rand, int col) const;
    int  numarLocuriLibere() const;

    std::vector<std::pair<int,int>> getLocuriSelectate() const;

    void afisaLocuri() const;
};