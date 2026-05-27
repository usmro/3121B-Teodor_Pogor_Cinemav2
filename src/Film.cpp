#include "Film.h"
#include <iostream>
#include <cmath>

Film::Film(const std::string& titlu, const std::string& format,
           const std::string& gen, const std::string& ora,
           int durata, int nrSala)
    : titlu(titlu), format(format), gen(gen),
      ora(ora), durata(durata), nrSala(nrSala) {}

std::string Film::getTitlu()  const { return titlu; }
std::string Film::getFormat() const { return format; }
std::string Film::getGen()    const { return gen; }
std::string Film::getOra()    const { return ora; }
int         Film::getDurata() const { return durata; }
int         Film::getNrSala() const { return nrSala; }

int Film::getPret() const {
    if (format == "IMAX") return 50;
    if (format == "4DX")  return 45;
    if (format == "3D")   return 35;
    return 30; // 2D
}

int Film::getPretDinamic(bool weekend, const std::string& tipBilet) const {
    double pret = getPret();
    if (weekend) pret *= 1.20;
    if (tipBilet == "copil")  pret *= 0.70;
    if (tipBilet == "senior") pret *= 0.80;
    return static_cast<int>(std::round(pret));
}

std::string Film::getInfo() const {
    return format + " | " + ora + " | Sala " + std::to_string(nrSala);
}

void Film::afiseaza() const {
    std::cout << "[" << format << "] " << titlu
              << " (" << durata << " min) - "
              << ora << " - " << getPret() << " RON\n";
}