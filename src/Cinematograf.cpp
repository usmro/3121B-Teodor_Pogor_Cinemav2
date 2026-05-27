#include "Cinematograf.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

Cinematograf::Cinematograf(const std::string& nume) : nome(nume), nextId(1) {}

Cinematograf::~Cinematograf() {
    for (Film* f : filme) delete f;
    for (Sala* s : sali)  delete s;
}

void Cinematograf::adaugaFilm(Film* film) { filme.push_back(film); }
void Cinematograf::adaugaSala(Sala* sala) { sali.push_back(sala); }

int Cinematograf::adaugaRezervare(const Rezervare& rez) {
    rezervari.push_back(rez);
    nextId++;
    return rez.getId();
}

int Cinematograf::getNextId() const { return nextId; }

Film* Cinematograf::getFilm(int index) const {
    if (index < 0 || index >= (int)filme.size())
        throw std::out_of_range("Index film invalid!");
    return filme[index];
}

Sala* Cinematograf::getSala(int index) const {
    if (index < 0 || index >= (int)sali.size())
        throw std::out_of_range("Index sala invalid!");
    return sali[index];
}

Sala* Cinematograf::getSalaForFilm(int filmIndex) const {
    int nrSala = getFilm(filmIndex)->getNrSala();
    for (Sala* s : sali)
        if (s->getNumar() == nrSala) return s;
    throw std::runtime_error("Sala negasita pentru filmul selectat!");
}

Rezervare& Cinematograf::getRezervare(int index) {
    return rezervari[index];
}

std::vector<Rezervare>& Cinematograf::getRezervari() {
    return rezervari;
}

int Cinematograf::getNumarFilme()     const { return (int)filme.size(); }
int Cinematograf::getNumarSali()      const { return (int)sali.size(); }
int Cinematograf::getNumarRezervari() const { return (int)rezervari.size(); }

bool Cinematograf::anuleazaRezervare(int id) {
    for (auto& r : rezervari) {
        if (r.getId() == id && !r.esteAnulata()) {
            r.anuleaza();
            return true;
        }
    }
    return false;
}

int Cinematograf::getTotalIncasari() const {
    int total = 0;
    for (auto& r : rezervari)
        if (!r.esteAnulata()) total += r.getTotal();
    return total;
}

int Cinematograf::getTotalBilete() const {
    int total = 0;
    for (auto& r : rezervari)
        if (!r.esteAnulata()) total += (int)r.getLocuri().size();
    return total;
}

std::map<std::string, int> Cinematograf::incasariPerFilm() const {
    std::map<std::string, int> m;
    for (auto& r : rezervari)
        if (!r.esteAnulata()) m[r.getFilmTitlu()] += r.getTotal();
    return m;
}

std::map<std::string, int> Cinematograf::biletePeTipuri() const {
    std::map<std::string, int> m;
    for (auto& r : rezervari)
        if (!r.esteAnulata()) m[r.getTipBilet()] += (int)r.getLocuri().size();
    return m;
}

void Cinematograf::salveazaDate() const {
    // Salveaza rezervarile
    {
        std::ofstream f("rezervari.txt");
        for (auto& r : rezervari) {
            f << r.getId() << "|"
              << r.getClient() << "|"
              << r.getFilmTitlu() << "|"
              << r.getMetoda() << "|"
              << r.getTipBilet() << "|"
              << r.getTotal() << "|"
              << r.getTimestamp() << "|"
              << r.getNrSala() << "|"
              << (r.esteAnulata() ? 1 : 0) << "|";
            for (int i = 0; i < (int)r.getLocuri().size(); i++) {
                if (i) f << ",";
                f << r.getLocuri()[i];
            }
            f << "\n";
        }
    }
    // Salveaza starea salilor
    {
        std::ofstream f("sali.txt");
        for (Sala* s : sali) {
            f << s->getNumar() << "\n";
            for (int r = 0; r < s->getRanduri(); r++) {
                for (int c = 0; c < s->getColoane(); c++) {
                    f << (int)s->getStare(r, c);
                    if (c < s->getColoane()-1) f << ",";
                }
                f << "\n";
            }
        }
    }
}

void Cinematograf::incarcaDate() {
    // Incarca rezervarile
    {
        std::ifstream f("rezervari.txt");
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok;
            std::vector<std::string> parts;
            while (std::getline(ss, tok, '|')) parts.push_back(tok);
            if (parts.size() < 10) continue;
            int id = std::stoi(parts[0]);
            std::string client    = parts[1];
            std::string filmTitlu = parts[2];
            std::string metoda    = parts[3];
            std::string tipBilet  = parts[4];
            int total             = std::stoi(parts[5]);
            std::string timestamp = parts[6];
            int nrSala            = std::stoi(parts[7]);
            bool anulata          = parts[8] == "1";
            std::vector<std::string> locuri;
            std::istringstream ls(parts[9]);
            std::string loc;
            while (std::getline(ls, loc, ',')) if (!loc.empty()) locuri.push_back(loc);
            Rezervare r(id, client, filmTitlu, metoda, tipBilet, locuri, total, timestamp, nrSala);
            if (anulata) r.anuleaza();
            rezervari.push_back(r);
            if (id >= nextId) nextId = id + 1;
        }
    }
    // Incarca starea salilor
    {
        std::ifstream f("sali.txt");
        if (!f.good()) return;
        for (Sala* s : sali) {
            std::string line;
            if (!std::getline(f, line)) break; // nr sala (ignoram)
            for (int r = 0; r < s->getRanduri(); r++) {
                if (!std::getline(f, line)) break;
                std::istringstream ss(line);
                std::string tok;
                int c = 0;
                while (std::getline(ss, tok, ',') && c < s->getColoane()) {
                    int stare = std::stoi(tok);
                    if (stare == Sala::OCUPAT || stare == Sala::VIP_OCUPAT) {
                        try { s->ocupaLoc(r, c); } catch(...) {}
                    }
                    c++;
                }
            }
        }
    }
}

void Cinematograf::afisaFilme() const {
    std::cout << "\n=== Filme la " << nome << " ===\n";
    for (int i = 0; i < (int)filme.size(); i++) {
        std::cout << i+1 << ". ";
        filme[i]->afiseaza();
    }
}

void Cinematograf::afisaLocuriSala(int i) const {
    getSala(i)->afisaLocuri();
}