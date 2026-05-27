#include "Film.h"
#include <iostream>

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

std::string Film::getInfo() const {
    return format + " | " + ora + " | Sala " + std::to_string(nrSala);
}

void Film::afiseaza() const {
    std::cout << "[" << format << "] " << titlu
              << " (" << durata << " min) - "
              << ora << " - " << getPret() << " RON\n";
}