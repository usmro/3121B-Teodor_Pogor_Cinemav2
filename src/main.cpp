#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "Film.h"
#include "Sala.h"
#include "Rezervare.h"
#include "Cinematograf.h"

// ============================================================
// CULORI
// ============================================================
const sf::Color C_BG      (15, 14, 10);
const sf::Color C_SURFACE (26, 24, 16);
const sf::Color C_SURFACE2(35, 31, 21);
const sf::Color C_BORDER  (58, 52, 32);
const sf::Color C_GOLD    (201,168, 76);
const sf::Color C_GOLD_L  (242,217,138);
const sf::Color C_TEXT    (232,222,204);
const sf::Color C_MUTED   (122,110, 88);
const sf::Color C_RED     (217, 75, 58);
const sf::Color C_GREEN   ( 75,173,122);
const sf::Color C_BLUE    ( 80,140,200);
const sf::Color C_PURPLE  (160, 90,200);
const sf::Color C_DARK    ( 10,  9,  7);
const sf::Color C_VIP     (180,120,220);

// ============================================================
// ENUM ECRANE
// ============================================================
enum Screen { MENIU, SALA, FILME, BILET, PLATA, RAPORT, ANULARE };

// ============================================================
// HELPERS
// ============================================================
std::string getCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::setfill('0')
        << (tm->tm_year+1900) << "-"
        << std::setw(2) << (tm->tm_mon+1) << "-"
        << std::setw(2) << tm->tm_mday << " "
        << std::setw(2) << tm->tm_hour << ":"
        << std::setw(2) << tm->tm_min;
    return oss.str();
}

bool esteWeekend() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    return tm->tm_wday == 0 || tm->tm_wday == 6; // 0=dum, 6=sam
}

sf::Text makeText(const std::string& s, const sf::Font& f,
                  unsigned size, sf::Color col, float x, float y) {
    sf::Text t(f);
    t.setString(s); t.setCharacterSize(size);
    t.setFillColor(col); t.setPosition({x, y});
    return t;
}

sf::RectangleShape makeRect(float x, float y, float w, float h,
    sf::Color fill, sf::Color outline = sf::Color::Transparent, float thick = 0.f) {
    sf::RectangleShape r;
    r.setPosition({x, y}); r.setSize({w, h});
    r.setFillColor(fill); r.setOutlineColor(outline);
    r.setOutlineThickness(thick);
    return r;
}

// ============================================================
// BUTTON
// ============================================================
struct Button {
    sf::RectangleShape box;
    std::string txt;
    bool primary = true;
    float bx=0,by=0,bw=0,bh=0;
    Button() {}
    Button(const std::string& t, const sf::Font&,
           float x, float y, float w, float h, bool p=true)
        : txt(t), primary(p), bx(x), by(y), bw(w), bh(h) {
        box.setPosition({x,y}); box.setSize({w,h});
        box.setFillColor(p ? C_GOLD : sf::Color::Transparent);
        box.setOutlineThickness(1.f);
        box.setOutlineColor(p ? C_GOLD : C_BORDER);
    }
    bool contains(sf::Vector2f p) { return box.getGlobalBounds().contains(p); }
    void draw(sf::RenderWindow& win, const sf::Font& font) {
        win.draw(box);
        sf::Text t(font);
        t.setString(txt); t.setCharacterSize(11);
        t.setFillColor(primary ? C_DARK : C_MUTED);
        auto b = t.getLocalBounds();
        t.setPosition({bx+(bw-b.size.x)/2.f, by+(bh-b.size.y)/2.f-2.f});
        win.draw(t);
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    sf::RenderWindow window(sf::VideoMode({1000u, 680u}), "* Happy Cinema");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/font.ttf")) return -1;

    // ---- Initializare cinematograf ----
    Cinematograf cinema("Happy Cinema");

    cinema.adaugaFilm(new Film("Interstellar",   "IMAX", "SF",      "20:00", 169, 1));
    cinema.adaugaFilm(new Film("Dune: Part Two", "4DX",  "Actiune", "18:30", 166, 2));
    cinema.adaugaFilm(new Film("Oppenheimer",    "3D",   "Drama",   "21:45", 180, 3));
    cinema.adaugaFilm(new Film("The Batman",     "2D",   "Actiune", "16:00", 176, 4));

    // Sali cu 1 rand VIP fiecare
    cinema.adaugaSala(new Sala(1, 6, 10, 1));
    cinema.adaugaSala(new Sala(2, 6, 10, 1));
    cinema.adaugaSala(new Sala(3, 6, 10, 1));
    cinema.adaugaSala(new Sala(4, 6, 10, 1));

    // Locuri ocupate initial (doar daca nu exista fisier salvat)
    auto setOcupat = [&](int salaIdx, std::vector<std::pair<int,int>> poz){
        Sala* s = cinema.getSala(salaIdx);
        for (auto& p : poz) try { s->ocupaLoc(p.first, p.second); } catch(...) {}
    };

    // Incarca date salvate (suprascrie locurile de mai jos daca exista)
    bool existaDate = false;
    {
        std::ifstream f("rezervari.txt");
        existaDate = f.good();
    }

    if (!existaDate) {
        setOcupat(0, {{0,2},{0,7},{1,4},{2,1},{2,8},{3,0},{4,5},{5,3}});
        setOcupat(1, {{0,1},{1,3},{2,5},{3,7},{4,2},{5,8},{0,5}});
        setOcupat(2, {{0,5},{1,1},{2,9},{3,4},{4,0},{5,6},{1,8}});
        setOcupat(3, {{0,3},{1,7},{2,2},{3,9},{4,4},{5,1},{2,6}});
    } else {
        cinema.incarcaDate();
    }

    // ---- State ----
    Screen currentScreen = MENIU;
    int selFilmIdx = 0;
    bool weekend = esteWeekend();

    // Grila locuri
    const float GX=330.f, GY=140.f, SW=28.f, SH=22.f, SGX=4.f, SGY=5.f;

    // Tip bilet selectat
    int tipBiletIdx = 0; // 0=adult, 1=copil, 2=senior
    std::vector<std::string> tipuriBilet = {"adult", "copil", "senior"};

    struct ComandaCurenta {
        std::string client, film;
        std::vector<std::string> locuri;
        int total = 0;
        bool activa = false;
        int nrSala = 0;
        std::string tipBilet = "adult";
    } comanda;

    int metodaPlata = 0;
    bool plataConfirmata = false;
    sf::Clock plataClock;
    int lastRezId = -1;

    // Nav
    std::vector<std::string> navL = {"[ MENIU ]","[ SALA ]","[ FILME ]","[ PLATA ]","[ RAPORT ]","[ ANULARE ]"};
    std::vector<Button> navBtns;
    for (int i = 0; i < 6; i++)
        navBtns.emplace_back(navL[i], font, 10.f + i*165.f, 8.f, 158.f, 28.f, false);

    // Butoane SALA
    Button btnContinua("CONTINUA >>", font, 330.f, 500.f, 150.f, 34.f, true);
    Button btnRst("RESETEAZA", font, 495.f, 500.f, 120.f, 34.f, false);

    // Butoane TIP BILET
    std::vector<Button> tipBtns;
    for (int i = 0; i < 3; i++)
        tipBtns.emplace_back(tipuriBilet[i], font, 330.f + i*130.f, 455.f, 120.f, 28.f, false);

    // Butoane PLATA
    Button btnCash("CASH", font, 330.f, 330.f, 180.f, 50.f, false);
    Button btnCard("CARD", font, 530.f, 330.f, 180.f, 50.f, false);
    Button btnConfirma("CONFIRMA PLATA", font, 380.f, 440.f, 240.f, 44.f, true);
    Button btnInapoi("< INAPOI", font, 330.f, 510.f, 150.f, 34.f, false);

    // Buton BILET
    Button btnNouaRez("NOUA REZERVARE", font, 350.f, 520.f, 200.f, 34.f, true);

    // Butoane ANULARE
    std::string inputAnulare = "";
    bool typingAnulare = false;
    Button btnAnuleaza("ANULEAZA REZERVAREA", font, 350.f, 360.f, 220.f, 36.f, false);

    std::string clientName = "";
    bool typingClient = false;
    auto inputBox = makeRect(330.f, 450.f, 260.f, 26.f, C_SURFACE, C_BORDER, 1.f);
    auto inputAnulareBox = makeRect(350.f, 310.f, 200.f, 28.f, C_SURFACE, C_BORDER, 1.f);

    std::string errMsg = "";
    bool showErr = false;
    sf::Clock errClock;

    auto setErr = [&](const std::string& msg) {
        errMsg = msg; showErr = true; errClock.restart();
    };

    // ============================================================
    // GAME LOOP
    // ============================================================
    while (window.isOpen()) {
        // Stinge eroarea dupa 3s
        if (showErr && errClock.getElapsedTime().asSeconds() > 3.f)
            showErr = false;

        // Redirect dupa plata confirmata
        if (plataConfirmata && plataClock.getElapsedTime().asSeconds() > 2.f) {
            plataConfirmata = false;
            currentScreen = BILET;
        }

        // ---- EVENTS ----
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                cinema.salveazaDate();
                window.close();
            }

            // Tastatura
            if (auto* kt = ev->getIf<sf::Event::TextEntered>()) {
                uint32_t ch = kt->unicode;
                if (typingClient) {
                    if (ch == 8 && !clientName.empty()) clientName.pop_back();
                    else if (ch >= 32 && ch < 128 && clientName.size() < 30)
                        clientName += (char)ch;
                }
                if (typingAnulare) {
                    if (ch == 8 && !inputAnulare.empty()) inputAnulare.pop_back();
                    else if (ch >= '0' && ch <= '9' && inputAnulare.size() < 6)
                        inputAnulare += (char)ch;
                }
            }

            // Click
            if (auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mp(static_cast<float>(mb->position.x),
                                    static_cast<float>(mb->position.y));

                    // Nav
                    for (int i = 0; i < 6; i++) {
                        if (navBtns[i].contains(mp)) {
                            if (i == 3 && !comanda.activa) { setErr("Selecteaza locuri inainte de plata!"); break; }
                            currentScreen = (Screen)i;
                            typingClient = false; typingAnulare = false;
                        }
                    }

                    // ---- MENIU ----
                    if (currentScreen == MENIU) {
                        // Click pe card film
                        for (int i = 0; i < cinema.getNumarFilme(); i++) {
                            float fx = 20.f, fy = 120.f + i*110.f;
                            sf::FloatRect cardR({fx, fy}, {960.f, 100.f});
                            if (cardR.contains(mp)) {
                                selFilmIdx = i;
                                currentScreen = SALA;
                            }
                        }
                    }

                    // ---- SALA ----
                    else if (currentScreen == SALA) {
                        Sala* salaC = cinema.getSalaForFilm(selFilmIdx);
                        int ROWS = salaC->getRanduri(), COLS = salaC->getColoane();

                        // Click pe loc
                        for (int r = 0; r < ROWS; r++) {
                            for (int c = 0; c < COLS; c++) {
                                sf::FloatRect lb({GX+c*(SW+SGX), GY+r*(SH+SGY)}, {SW-2, SH-2});
                                if (lb.contains(mp)) {
                                    try { salaC->selecteazaLoc(r, c); }
                                    catch (std::exception& e) { setErr(e.what()); }
                                }
                            }
                        }

                        // Click tip bilet
                        for (int i = 0; i < 3; i++) {
                            if (tipBtns[i].contains(mp)) tipBiletIdx = i;
                        }

                        // Click input client
                        if (inputBox.getGlobalBounds().contains(mp))
                            typingClient = true;
                        else typingClient = false;

                        // Continua
                        if (btnContinua.contains(mp)) {
                            auto sel = salaC->getLocuriSelectate();
                            if (sel.empty()) { setErr("Selecteaza cel putin un loc!"); }
                            else if (clientName.empty()) { setErr("Introdu numele clientului!"); }
                            else {
                                Film* f = cinema.getFilm(selFilmIdx);
                                comanda.client = clientName;
                                comanda.film = f->getTitlu();
                                comanda.nrSala = f->getNrSala();
                                comanda.tipBilet = tipuriBilet[tipBiletIdx];
                                comanda.locuri.clear();
                                for (auto& p : sel) {
                                    std::string locStr = "R"+std::to_string(p.first+1)
                                                       + "C"+std::to_string(p.second+1);
                                    comanda.locuri.push_back(locStr);
                                }
                                comanda.total = (int)sel.size() *
                                    f->getPretDinamic(weekend, comanda.tipBilet);
                                comanda.activa = true;
                                currentScreen = PLATA;
                            }
                        }

                        // Reseteaza
                        if (btnRst.contains(mp)) {
                            salaC->reseteazaSelectie();
                            clientName = "";
                            comanda.activa = false;
                        }
                    }

                    // ---- PLATA ----
                    else if (currentScreen == PLATA && !plataConfirmata) {
                        if (btnCash.contains(mp)) metodaPlata = 1;
                        if (btnCard.contains(mp)) metodaPlata = 2;
                        if (btnInapoi.contains(mp)) currentScreen = SALA;
                        if (btnConfirma.contains(mp)) {
                            if (!comanda.activa) { setErr("Nu ai o comanda activa!"); }
                            else if (metodaPlata == 0) { setErr("Alege metoda de plata!"); }
                            else {
                                // Ocupa locurile definitiv
                                Sala* salaC = cinema.getSalaForFilm(selFilmIdx);
                                auto sel = salaC->getLocuriSelectate();
                                for (auto& p : sel) {
                                    try { salaC->ocupaLoc(p.first, p.second); }
                                    catch (...) {}
                                }

                                std::string metStr = (metodaPlata==1) ? "cash" : "card";
                                Rezervare rez(cinema.getNextId(), comanda.client,
                                    comanda.film, metStr, comanda.tipBilet,
                                    comanda.locuri, comanda.total,
                                    getCurrentTimestamp(), comanda.nrSala);
                                lastRezId = cinema.adaugaRezervare(rez);

                                plataConfirmata = true;
                                plataClock.restart();
                                comanda.activa = false;
                                clientName = "";
                                metodaPlata = 0;
                            }
                        }
                    }

                    // ---- BILET ----
                    else if (currentScreen == BILET) {
                        if (btnNouaRez.contains(mp)) {
                            currentScreen = MENIU;
                            selFilmIdx = 0;
                        }
                    }

                    // ---- ANULARE ----
                    else if (currentScreen == ANULARE) {
                        if (inputAnulareBox.getGlobalBounds().contains(mp))
                            typingAnulare = true;
                        else typingAnulare = false;

                        if (btnAnuleaza.contains(mp)) {
                            if (inputAnulare.empty()) { setErr("Introdu ID-ul rezervarii!"); }
                            else {
                                int id = std::stoi(inputAnulare);
                                if (cinema.anuleazaRezervare(id)) {
                                    setErr("Rezervarea #" + inputAnulare + " a fost anulata!");
                                    inputAnulare = "";
                                } else {
                                    setErr("Rezervarea #" + inputAnulare + " nu a fost gasita sau e deja anulata!");
                                }
                            }
                        }
                    }
                }
            }
        }

        // ============================================================
        // DRAW
        // ============================================================
        window.clear(C_BG);

        // Header bar
        window.draw(makeRect(0, 0, 1000, 45, C_SURFACE, C_BORDER, 0));
        window.draw(makeRect(0, 43, 1000, 1, C_BORDER));

        // Logo
        window.draw(makeText("* HAPPY CINEMA", font, 14, C_GOLD, 10, 15));

        // Weekend badge
        if (weekend)
            window.draw(makeText("[WEEKEND +20%]", font, 9, C_GOLD, 860, 17));

        // Nav buttons
        for (int i = 0; i < 6; i++) {
            bool active = (currentScreen == (Screen)i);
            navBtns[i].box.setFillColor(active ? C_GOLD : sf::Color::Transparent);
            navBtns[i].box.setOutlineColor(active ? C_GOLD_L : C_BORDER);
            navBtns[i].draw(window, font);
        }

        // ---- MENIU ----
        if (currentScreen == MENIU) {
            window.draw(makeText("SELECTEAZA UN FILM", font, 11, C_MUTED, 20, 60));
            window.draw(makeText("Click pe un film pentru a rezerva locuri", font, 10, C_MUTED, 20, 78));
            window.draw(makeRect(20, 100, 960, 1, C_BORDER));

            for (int i = 0; i < cinema.getNumarFilme(); i++) {
                Film* f = cinema.getFilm(i);
                float fy = 110.f + i * 110.f;
                bool hov = (selFilmIdx == i);

                window.draw(makeRect(20, fy, 960, 98, hov ? C_SURFACE2 : C_SURFACE, C_BORDER, 1));

                // Format badge culoare
                sf::Color fmtCol = C_MUTED;
                if (f->getFormat()=="IMAX") fmtCol = C_BLUE;
                else if (f->getFormat()=="4DX") fmtCol = C_PURPLE;
                else if (f->getFormat()=="3D")  fmtCol = C_GREEN;
                else fmtCol = C_MUTED;

                window.draw(makeRect(30, fy+10, 60, 22, fmtCol));
                window.draw(makeText(f->getFormat(), font, 10, C_DARK, 35, fy+13));

                window.draw(makeText(f->getTitlu(), font, 18, C_TEXT, 105, fy+10));
                window.draw(makeText(f->getGen(), font, 11, C_MUTED, 105, fy+35));
                window.draw(makeText(std::to_string(f->getDurata())+" min", font, 11, C_MUTED, 105, fy+55));

                window.draw(makeText("Ora: "+f->getOra(), font, 12, C_GOLD_L, 400, fy+20));
                window.draw(makeText("Sala "+std::to_string(f->getNrSala()), font, 12, C_TEXT, 400, fy+45));

                int pretAdult  = f->getPretDinamic(weekend, "adult");
                int pretCopil  = f->getPretDinamic(weekend, "copil");
                int pretSenior = f->getPretDinamic(weekend, "senior");
                window.draw(makeText("Adult: "+std::to_string(pretAdult)+" RON", font, 11, C_GOLD, 600, fy+15));
                window.draw(makeText("Copil: "+std::to_string(pretCopil)+" RON",  font, 11, C_MUTED, 600, fy+35));
                window.draw(makeText("Senior: "+std::to_string(pretSenior)+" RON",font, 11, C_MUTED, 600, fy+55));

                Sala* sala = cinema.getSalaForFilm(i);
                int libere = sala->numarLocuriLibere();
                sf::Color locCol = libere > 10 ? C_GREEN : (libere > 0 ? C_GOLD : C_RED);
                window.draw(makeText(std::to_string(libere)+" locuri libere", font, 12, locCol, 800, fy+30));

                window.draw(makeText("REZERVA >", font, 10, C_GOLD, 890, fy+38));
            }
        }

        // ---- SALA ----
        else if (currentScreen == SALA) {
            Sala* salaC = cinema.getSalaForFilm(selFilmIdx);
            Film* filmC = cinema.getFilm(selFilmIdx);
            int ROWS = salaC->getRanduri(), COLS = salaC->getColoane();

            // Panou stanga
            window.draw(makeRect(0, 46, 305, 634, C_SURFACE));
            window.draw(makeRect(304, 46, 1, 634, C_BORDER));

            window.draw(makeText("FILM SELECTAT", font, 9, C_MUTED, 12, 60));
            window.draw(makeText(filmC->getTitlu(), font, 14, C_TEXT, 12, 75));
            window.draw(makeText(filmC->getInfo(), font, 10, C_GOLD, 12, 96));
            window.draw(makeRect(12, 114, 280, 1, C_BORDER));

            // Info preturi tip bilet
            window.draw(makeText("PRET / LOC", font, 9, C_MUTED, 12, 120));
            std::vector<std::string> tip3 = {"adult","copil","senior"};
            std::vector<sf::Color>   col3 = {C_GOLD, C_GREEN, C_BLUE};
            for (int i = 0; i < 3; i++) {
                int p = filmC->getPretDinamic(weekend, tip3[i]);
                window.draw(makeText(tip3[i]+": "+std::to_string(p)+" RON", font, 11, col3[i], 12, 136+i*18));
            }
            window.draw(makeRect(12, 194, 280, 1, C_BORDER));

            // Sumar selectie
            auto sel = salaC->getLocuriSelectate();
            int nSel = (int)sel.size();
            std::string tipCurent = tipuriBilet[tipBiletIdx];
            int pretLoc = filmC->getPretDinamic(weekend, tipCurent);

            window.draw(makeRect(12, 200, 280, 90, C_DARK, C_BORDER, 1.f));
            window.draw(makeText("SUMAR COMANDA", font, 9, C_MUTED, 20, 208));
            window.draw(makeText("Locuri: ", font, 11, C_MUTED, 20, 226));
            window.draw(makeText(std::to_string(nSel), font, 11, C_GOLD, 200, 226));
            window.draw(makeText("Tip bilet: ", font, 11, C_MUTED, 20, 244));
            window.draw(makeText(tipCurent, font, 11, C_GOLD, 200, 244));
            window.draw(makeText("Total: ", font, 11, C_MUTED, 20, 262));
            window.draw(makeText(std::to_string(nSel*pretLoc)+" RON", font, 13, C_GOLD, 190, 260));

            window.draw(makeRect(12, 298, 280, 1, C_BORDER));

            // Selectii locuri
            window.draw(makeText("LOCURI SELECTATE", font, 9, C_MUTED, 12, 305));
            std::string selStr;
            for (auto& p : sel) {
                if (!selStr.empty()) selStr += ", ";
                selStr += "R"+std::to_string(p.first+1)+"C"+std::to_string(p.second+1);
            }
            // wrap text locuri
            if (selStr.empty()) selStr = "—";
            window.draw(makeText(selStr.substr(0, 35), font, 10, C_TEXT, 12, 320));
            if (selStr.size() > 35)
                window.draw(makeText(selStr.substr(35, 35), font, 10, C_TEXT, 12, 336));

            // ---- Grila ----
            window.draw(makeText("SALA "+std::to_string(filmC->getNrSala())+" - "+filmC->getTitlu(),
                font, 11, C_GOLD, 330, 58));

            // Ecran
            window.draw(makeRect(GX-10, 82, COLS*(SW+SGX)+4, 14, C_SURFACE2, C_BORDER, 1));
            window.draw(makeText("E C R A N", font, 8, C_MUTED, GX+90, 85));

            // Numere coloane
            for (int c = 0; c < COLS; c++)
                window.draw(makeText(std::to_string(c+1), font, 8, C_MUTED,
                    GX+c*(SW+SGX)+10, 100));

            for (int r = 0; r < ROWS; r++) {
                window.draw(makeText("R"+std::to_string(r+1), font, 9, C_MUTED,
                    GX-24, GY+r*(SH+SGY)+3));
                if (salaC->esteVIP(r))
                    window.draw(makeText("V", font, 8, C_VIP, GX-10, GY+r*(SH+SGY)+3));
            }

            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    sf::RectangleShape loc;
                    loc.setSize({SW-2.f, SH-2.f});
                    loc.setPosition({GX+c*(SW+SGX), GY+r*(SH+SGY)});
                    loc.setOutlineThickness(1.f);
                    switch (salaC->getStare(r, c)) {
                        case Sala::LIBER:
                            loc.setFillColor({13,43,26}); loc.setOutlineColor(C_GREEN); break;
                        case Sala::OCUPAT:
                            loc.setFillColor({43,13,13}); loc.setOutlineColor(C_RED); break;
                        case Sala::SELECTAT:
                            loc.setFillColor(C_GOLD); loc.setOutlineColor(C_GOLD_L); break;
                        case Sala::VIP_LIBER:
                            loc.setFillColor({20,15,40}); loc.setOutlineColor(C_VIP); break;
                        case Sala::VIP_OCUPAT:
                            loc.setFillColor({43,13,43}); loc.setOutlineColor({120,50,120}); break;
                        case Sala::VIP_SELECTAT:
                            loc.setFillColor(C_VIP); loc.setOutlineColor({220,180,255}); break;
                    }
                    window.draw(loc);
                }
            }

            // Legenda
            float lx = GX, ly = GY + ROWS*(SH+SGY) + 12;
            window.draw(makeRect(lx,    ly, 10, 8, {13,43,26},  C_GREEN,  1)); window.draw(makeText("liber",    font, 9, C_MUTED, lx+14,  ly));
            window.draw(makeRect(lx+70, ly, 10, 8, {43,13,13},  C_RED,    1)); window.draw(makeText("ocupat",   font, 9, C_MUTED, lx+84,  ly));
            window.draw(makeRect(lx+145,ly, 10, 8, C_GOLD,      C_GOLD_L, 1)); window.draw(makeText("selectat", font, 9, C_MUTED, lx+159, ly));
            window.draw(makeRect(lx+230,ly, 10, 8, {20,15,40},  C_VIP,    1)); window.draw(makeText("VIP",      font, 9, C_VIP,   lx+244, ly));

            // Tip bilet
            window.draw(makeText("TIP BILET", font, 9, C_MUTED, 330, 440));
            for (int i = 0; i < 3; i++) {
                bool sel2 = (tipBiletIdx == i);
                tipBtns[i].box.setFillColor(sel2 ? C_GOLD : C_SURFACE);
                tipBtns[i].box.setOutlineColor(sel2 ? C_GOLD_L : C_BORDER);
                tipBtns[i].box.setOutlineThickness(1.f);
                tipBtns[i].draw(window, font);
            }

            // Input client
            window.draw(makeText("NUME CLIENT", font, 9, C_MUTED, 330, 438));
            // shift down inputs
            auto ib2 = makeRect(330.f, 490.f, 260.f, 26.f, C_SURFACE,
                typingClient ? C_GOLD : C_BORDER, 1.f);
            window.draw(ib2);
            std::string disp = clientName + (typingClient ? "|" : "");
            window.draw(makeText(disp.empty() ? "ex: Popescu Ion" : disp,
                font, 11, clientName.empty() ? C_MUTED : C_TEXT, 338, 495));

            // reposition input and tip buttons
            // tip bilet line
            window.draw(makeText("TIP BILET", font, 9, C_MUTED, 330, 458));
            for (int i = 0; i < 3; i++) {
                tipBtns[i].bx = 330.f + i*130.f; tipBtns[i].by = 470.f;
                tipBtns[i].box.setPosition({tipBtns[i].bx, tipBtns[i].by});
                bool sel2 = (tipBiletIdx == i);
                tipBtns[i].box.setFillColor(sel2 ? C_GOLD : C_SURFACE);
                tipBtns[i].draw(window, font);
            }
            window.draw(makeText("NUME CLIENT", font, 9, C_MUTED, 330, 503));
            inputBox.setPosition({330.f, 515.f});
            inputBox.setOutlineColor(typingClient ? C_GOLD : C_BORDER);
            window.draw(inputBox);
            disp = clientName + (typingClient ? "|" : "");
            window.draw(makeText(disp.empty() ? "ex: Popescu Ion" : disp,
                font, 11, clientName.empty() ? C_MUTED : C_TEXT, 338, 519));

            btnContinua.bx=330.f; btnContinua.by=550.f;
            btnContinua.box.setPosition({330.f,550.f});
            btnContinua.draw(window, font);
            btnRst.bx=495.f; btnRst.by=550.f;
            btnRst.box.setPosition({495.f,550.f});
            btnRst.draw(window, font);

            if (showErr) {
                window.draw(makeRect(330, 595, 640, 24, {43,13,13}, C_RED, 1.f));
                window.draw(makeText(errMsg, font, 10, C_RED, 338, 599));
            }
        }

        // ---- FILME ----
        else if (currentScreen == FILME) {
            window.draw(makeText("FILME IN PROGRAM", font, 11, C_GOLD, 20, 58));
            window.draw(makeRect(20, 76, 960, 1, C_BORDER));
            std::vector<std::string> hdrs = {"#","TITLU","FORMAT","DURATA","GEN","ORA","SALA","ADULT","COPIL","SENIOR"};
            float hx[] = {20,60,250,350,430,540,630,710,790,870};
            for (int i = 0; i < 10; i++)
                window.draw(makeText(hdrs[i], font, 9, C_MUTED, hx[i], 82));
            window.draw(makeRect(20, 96, 960, 1, C_BORDER));
            for (int i = 0; i < cinema.getNumarFilme(); i++) {
                Film* f = cinema.getFilm(i);
                float ry = 106.f + i*44.f;
                window.draw(makeRect(20, ry, 960, 38, i%2==0 ? C_SURFACE : sf::Color::Transparent));
                window.draw(makeText(std::to_string(i+1),            font, 12, C_MUTED,  hx[0], ry+10));
                window.draw(makeText(f->getTitlu(),                   font, 13, C_TEXT,   hx[1], ry+10));
                window.draw(makeText(f->getFormat(),                  font, 12, C_GOLD,   hx[2], ry+10));
                window.draw(makeText(std::to_string(f->getDurata())+"min", font, 12, C_MUTED, hx[3], ry+10));
                window.draw(makeText(f->getGen(),                     font, 12, C_MUTED,  hx[4], ry+10));
                window.draw(makeText(f->getOra(),                     font, 12, C_GOLD_L, hx[5], ry+10));
                window.draw(makeText("Sala "+std::to_string(f->getNrSala()), font, 12, C_TEXT, hx[6], ry+10));
                window.draw(makeText(std::to_string(f->getPretDinamic(weekend,"adult")), font, 12, C_GOLD, hx[7], ry+10));
                window.draw(makeText(std::to_string(f->getPretDinamic(weekend,"copil")), font, 11, C_MUTED, hx[8], ry+10));
                window.draw(makeText(std::to_string(f->getPretDinamic(weekend,"senior")),font, 11, C_MUTED, hx[9], ry+10));
                window.draw(makeRect(20, ry+38, 960, 1, C_BORDER));
            }
            if (weekend) {
                window.draw(makeRect(20, 300, 960, 30, {30,25,5}, C_GOLD, 1));
                window.draw(makeText("WEEKEND ACTIV — Toate preturile au majorare de +20%", font, 11, C_GOLD, 30, 308));
            }
        }

        // ---- PLATA ----
        else if (currentScreen == PLATA) {
            if (plataConfirmata) {
                window.draw(makeRect(280, 240, 440, 140, C_DARK, C_GREEN, 2.f));
                window.draw(makeText("PLATA CONFIRMATA!", font, 20, C_GREEN, 340, 270));
                window.draw(makeText("Se genereaza biletul...", font, 12, C_MUTED, 360, 315));
            } else {
                window.draw(makeText("FINALIZARE REZERVARE", font, 13, C_GOLD, 20, 58));
                window.draw(makeRect(20, 78, 960, 1, C_BORDER));

                // Sumar
                window.draw(makeRect(20, 88, 960, 130, C_SURFACE, C_BORDER, 1.f));
                window.draw(makeText("SUMAR COMANDA", font, 9, C_MUTED, 35, 100));
                window.draw(makeText("Client:",    font, 11, C_MUTED, 35, 120)); window.draw(makeText(comanda.client,       font, 13, C_TEXT,  150, 120));
                window.draw(makeText("Film:",      font, 11, C_MUTED, 35, 142)); window.draw(makeText(comanda.film,         font, 13, C_GOLD_L,150, 142));
                window.draw(makeText("Tip bilet:", font, 11, C_MUTED, 35, 163)); window.draw(makeText(comanda.tipBilet,     font, 12, C_GOLD,  150, 163));
                std::string locsStr;
                for (int i = 0; i < (int)comanda.locuri.size(); i++) { if(i) locsStr+=", "; locsStr+=comanda.locuri[i]; }
                window.draw(makeText("Locuri:",    font, 11, C_MUTED, 35, 183)); window.draw(makeText(locsStr,              font, 11, C_TEXT,  150, 183));
                window.draw(makeText("TOTAL DE PLATA:", font, 14, C_MUTED, 35, 200));
                window.draw(makeText(std::to_string(comanda.total)+" RON", font, 20, C_GOLD, 250, 197));

                // Metoda plata
                window.draw(makeText("METODA DE PLATA", font, 9, C_MUTED, 20, 250));
                window.draw(makeRect(20, 264, 960, 1, C_BORDER));

                btnCash.box.setFillColor(metodaPlata==1 ? C_GOLD : C_SURFACE);
                btnCash.box.setOutlineColor(metodaPlata==1 ? C_GOLD_L : C_BORDER);
                btnCash.box.setOutlineThickness(1.f);
                window.draw(btnCash.box);
                window.draw(makeText("CASH",         font, 16, metodaPlata==1?C_DARK:C_TEXT,  390, 343));
                window.draw(makeText("Plata la casa",font, 10, metodaPlata==1?C_DARK:C_MUTED, 368, 365));

                btnCard.box.setFillColor(metodaPlata==2 ? C_GOLD : C_SURFACE);
                btnCard.box.setOutlineColor(metodaPlata==2 ? C_GOLD_L : C_BORDER);
                btnCard.box.setOutlineThickness(1.f);
                window.draw(btnCard.box);
                window.draw(makeText("CARD",          font, 16, metodaPlata==2?C_DARK:C_TEXT,  590, 343));
                window.draw(makeText("Plata cu cardul",font,10, metodaPlata==2?C_DARK:C_MUTED, 566, 365));

                btnConfirma.draw(window, font);
                btnInapoi.draw(window, font);

                if (showErr) {
                    window.draw(makeRect(330, 570, 360, 26, {43,13,13}, C_RED, 1.f));
                    window.draw(makeText(errMsg, font, 10, C_RED, 338, 575));
                }
            }
        }

        // ---- BILET ----
        else if (currentScreen == BILET) {
            // Gaseste ultima rezervare activa
            Rezervare* lastRez = nullptr;
            auto& rezv = cinema.getRezervari();
            for (int i = (int)rezv.size()-1; i >= 0; i--) {
                if (!rezv[i].esteAnulata() && rezv[i].getId() == lastRezId) {
                    lastRez = &rezv[i]; break;
                }
            }

            if (!lastRez && !rezv.empty()) {
                for (int i = (int)rezv.size()-1; i >= 0; i--) {
                    if (!rezv[i].esteAnulata()) { lastRez = &rezv[i]; break; }
                }
            }

            window.draw(makeText("BILET / CONFIRMARE", font, 13, C_GOLD, 20, 58));
            window.draw(makeRect(20, 78, 960, 1, C_BORDER));

            if (!lastRez) {
                window.draw(makeText("Nu exista rezervari active.", font, 14, C_MUTED, 350, 300));
            } else {
                // Bilet vizual
                window.draw(makeRect(200, 100, 600, 340, C_SURFACE, C_GOLD, 2.f));
                // Linie perforate
                window.draw(makeRect(200, 260, 600, 1, C_BORDER));

                window.draw(makeText("* HAPPY CINEMA *", font, 18, C_GOLD, 320, 115));
                window.draw(makeRect(215, 142, 570, 1, C_BORDER));

                window.draw(makeText("FILM:", font, 10, C_MUTED, 220, 155));
                window.draw(makeText(lastRez->getFilmTitlu(), font, 16, C_TEXT, 340, 152));

                window.draw(makeText("CLIENT:", font, 10, C_MUTED, 220, 185));
                window.draw(makeText(lastRez->getClient(), font, 14, C_TEXT, 340, 183));

                window.draw(makeText("TIP BILET:", font, 10, C_MUTED, 220, 210));
                window.draw(makeText(lastRez->getTipBilet(), font, 13, C_GOLD, 340, 208));

                std::string locsStr;
                for (int i = 0; i < (int)lastRez->getLocuri().size(); i++) {
                    if(i) locsStr += ", ";
                    locsStr += lastRez->getLocuri()[i];
                }
                window.draw(makeText("LOCURI:", font, 10, C_MUTED, 220, 232));
                window.draw(makeText(locsStr, font, 13, C_TEXT, 340, 230));

                window.draw(makeText("METODA:", font, 10, C_MUTED, 220, 270));
                window.draw(makeText(lastRez->getMetoda(), font, 13,
                    lastRez->getMetoda()=="cash"?C_GREEN:C_GOLD, 340, 268));

                window.draw(makeText("TOTAL:", font, 10, C_MUTED, 220, 292));
                window.draw(makeText(std::to_string(lastRez->getTotal())+" RON", font, 16, C_GOLD, 340, 289));

                window.draw(makeText("ID REZ:", font, 10, C_MUTED, 220, 318));
                window.draw(makeText("#"+std::to_string(lastRez->getId()), font, 13, C_MUTED, 340, 316));

                window.draw(makeText(lastRez->getTimestamp(), font, 10, C_MUTED, 600, 318));
            }

            btnNouaRez.bx = 390.f; btnNouaRez.by = 470.f;
            btnNouaRez.box.setPosition({390.f, 470.f});
            btnNouaRez.draw(window, font);
        }

        // ---- RAPORT ----
        else if (currentScreen == RAPORT) {
            auto& rez = cinema.getRezervari();
            window.draw(makeText("RAPORT REZERVARI", font, 11, C_GOLD, 20, 58));
            window.draw(makeRect(20, 76, 960, 1, C_BORDER));

            int totalInc     = cinema.getTotalIncasari();
            int totalBilete  = cinema.getTotalBilete();
            int totalRez     = 0, totalAnulate = 0;
            for (auto& r : rez) { if (!r.esteAnulata()) totalRez++; else totalAnulate++; }

            // KPI boxes
            auto drawKPI = [&](float x, float y, const std::string& val, const std::string& lbl, sf::Color col) {
                window.draw(makeRect(x, y, 180, 60, C_DARK, C_BORDER, 1));
                window.draw(makeText(val, font, 20, col, x+10, y+8));
                window.draw(makeText(lbl, font, 9, C_MUTED, x+10, y+38));
            };
            drawKPI( 20, 86, std::to_string(totalRez),             "REZERVARI ACTIVE", C_GOLD);
            drawKPI(210, 86, std::to_string(totalAnulate),         "ANULATE",           C_RED);
            drawKPI(400, 86, std::to_string(totalBilete),          "BILETE VANDUTE",    C_GREEN);
            drawKPI(590, 86, std::to_string(totalInc)+" RON",      "INCASARI TOTALE",   C_GOLD);

            // Incasari per film
            window.draw(makeText("INCASARI PER FILM", font, 9, C_MUTED, 20, 162));
            window.draw(makeRect(20, 176, 460, 1, C_BORDER));
            auto incFilm = cinema.incasariPerFilm();
            int fi = 0;
            for (auto& kv : incFilm) {
                float ry = 182.f + fi*22.f;
                window.draw(makeText(kv.first, font, 11, C_TEXT, 25, ry));
                window.draw(makeText(std::to_string(kv.second)+" RON", font, 11, C_GOLD, 360, ry));
                fi++;
            }

            // Bilete pe tipuri
            window.draw(makeText("BILETE PE TIPURI", font, 9, C_MUTED, 500, 162));
            window.draw(makeRect(500, 176, 460, 1, C_BORDER));
            auto tipuriMap = cinema.biletePeTipuri();
            int ti = 0;
            std::vector<sf::Color> tipCols = {C_GOLD, C_GREEN, C_BLUE};
            for (auto& kv : tipuriMap) {
                float ry = 182.f + ti*22.f;
                window.draw(makeText(kv.first, font, 11, C_TEXT, 505, ry));
                window.draw(makeText(std::to_string(kv.second)+" bilete", font, 11, tipCols[ti%3], 680, ry));
                ti++;
            }

            // Tabel rezervari
            window.draw(makeRect(20, 270, 960, 1, C_BORDER));
            window.draw(makeText("LISTA REZERVARI", font, 9, C_MUTED, 20, 278));
            std::vector<std::string> rh = {"#","ID","CLIENT","FILM","TIP","METODA","LOCURI","TOTAL","STATUS"};
            float rx[] = {20,50,90,230,420,510,590,760,860};
            float hry = 296.f;
            for (int i = 0; i < 9; i++)
                window.draw(makeText(rh[i], font, 9, C_MUTED, rx[i], hry));
            window.draw(makeRect(20, hry+16, 960, 1, C_BORDER));

            if (rez.empty())
                window.draw(makeText("— nicio rezervare —", font, 12, C_MUTED, 380, hry+30));

            int drawn = 0;
            for (int i = 0; i < (int)rez.size() && drawn < 10; i++) {
                float ty = hry + 26.f + drawn*28.f;
                auto& r = rez[i];
                std::string locs;
                for (int j = 0; j < (int)r.getLocuri().size(); j++) { if(j) locs+=","; locs+=r.getLocuri()[j]; }
                sf::Color rowC = r.esteAnulata() ? C_MUTED : C_TEXT;
                window.draw(makeText(std::to_string(i+1),         font, 10, C_MUTED, rx[0], ty));
                window.draw(makeText("#"+std::to_string(r.getId()),font, 10, C_MUTED, rx[1], ty));
                window.draw(makeText(r.getClient(),               font, 11, rowC,    rx[2], ty));
                window.draw(makeText(r.getFilmTitlu(),            font, 11, rowC,    rx[3], ty));
                window.draw(makeText(r.getTipBilet(),             font, 10, C_GOLD,  rx[4], ty));
                window.draw(makeText(r.getMetoda(),               font, 10,
                    r.getMetoda()=="cash"?C_GREEN:C_GOLD,          rx[5], ty));
                window.draw(makeText(locs.substr(0,18),           font, 10, C_MUTED, rx[6], ty));
                window.draw(makeText(std::to_string(r.getTotal())+" RON", font, 11, C_GOLD, rx[7], ty));
                window.draw(makeText(r.esteAnulata()?"ANULATA":"ACTIVA", font, 10,
                    r.esteAnulata()?C_RED:C_GREEN, rx[8], ty));
                window.draw(makeRect(20, ty+22, 960, 1, C_BORDER));
                drawn++;
            }
        }

        // ---- ANULARE ----
        else if (currentScreen == ANULARE) {
            window.draw(makeText("ANULARE REZERVARE", font, 13, C_GOLD, 20, 58));
            window.draw(makeRect(20, 78, 960, 1, C_BORDER));
            window.draw(makeText("Introdu ID-ul rezervarii pe care doresti sa o anulezi.", font, 12, C_MUTED, 20, 100));
            window.draw(makeText("ID-ul rezervarii se gaseste pe bilet sau in raport.", font, 11, C_MUTED, 20, 120));

            window.draw(makeRect(300, 200, 400, 220, C_SURFACE, C_BORDER, 1));
            window.draw(makeText("ID REZERVARE", font, 9, C_MUTED, 360, 240));

            inputAnulareBox.setPosition({350.f, 256.f});
            inputAnulareBox.setOutlineColor(typingAnulare ? C_GOLD : C_BORDER);
            window.draw(inputAnulareBox);
            std::string dispA = inputAnulare + (typingAnulare ? "|" : "");
            window.draw(makeText(dispA.empty() ? "ex: 3" : dispA, font, 14,
                inputAnulare.empty() ? C_MUTED : C_TEXT, 420, 260));

            btnAnuleaza.box.setPosition({350.f, 316.f});
            btnAnuleaza.bx = 350.f; btnAnuleaza.by = 316.f;
            btnAnuleaza.box.setFillColor(sf::Color::Transparent);
            btnAnuleaza.box.setOutlineColor(C_RED);
            btnAnuleaza.box.setOutlineThickness(1.f);
            window.draw(btnAnuleaza.box);
            window.draw(makeText("ANULEAZA REZERVAREA", font, 11, C_RED, 380, 325));

            // Lista rezervari active
            window.draw(makeText("REZERVARI ACTIVE", font, 9, C_MUTED, 20, 450));
            window.draw(makeRect(20, 466, 960, 1, C_BORDER));
            auto& rez = cinema.getRezervari();
            int dr = 0;
            for (auto& r : rez) {
                if (r.esteAnulata()) continue;
                float ry = 474.f + dr*26.f;
                window.draw(makeText("#"+std::to_string(r.getId()), font, 11, C_GOLD,  20, ry));
                window.draw(makeText(r.getClient(),                  font, 11, C_TEXT,  70, ry));
                window.draw(makeText(r.getFilmTitlu(),               font, 11, C_TEXT, 240, ry));
                window.draw(makeText(r.getTipBilet(),                font, 10, C_MUTED,430, ry));
                window.draw(makeText(std::to_string(r.getTotal())+" RON", font, 11, C_GOLD, 560, ry));
                window.draw(makeText(r.getTimestamp(),               font, 10, C_MUTED,700, ry));
                dr++;
                if (dr >= 6) break;
            }
            if (dr == 0)
                window.draw(makeText("— nicio rezervare activa —", font, 12, C_MUTED, 360, 490));

            if (showErr) {
                sf::Color ec = errMsg.find("anulata") != std::string::npos ? C_GREEN : C_RED;
                window.draw(makeRect(20, 620, 960, 28, {10,5,5}, ec, 1.f));
                window.draw(makeText(errMsg, font, 11, ec, 30, 626));
            }
        }

        window.display();
    }

    cinema.salveazaDate();
    return 0;
}