#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <stdexcept>

// ============================================================
// CULORI
// ============================================================
const sf::Color C_BG      (15,14,10);
const sf::Color C_SURFACE (26,24,16);
const sf::Color C_SURFACE2(35,31,21);
const sf::Color C_BORDER  (58,52,32);
const sf::Color C_GOLD    (201,168,76);
const sf::Color C_GOLD_L  (242,217,138);
const sf::Color C_TEXT    (232,222,204);
const sf::Color C_MUTED   (122,110,88);
const sf::Color C_RED     (217,75,58);
const sf::Color C_GREEN   (75,173,122);
const sf::Color C_DARK    (10,9,7);

enum Screen { SALA, FILME, PLATA, RAPORT };

// ============================================================
// CLASA FILM
// ============================================================
class Film {
public:
    std::string titlu;
    std::string format; // 2D, 3D, IMAX, 4DX
    std::string gen;
    std::string ora;
    int durata;
    int nrSala;

    Film(const std::string& t, const std::string& f,
         const std::string& g, const std::string& o,
         int d, int s)
        : titlu(t), format(f), gen(g), ora(o), durata(d), nrSala(s) {}

    // Calcul pret in functie de tipul filmului (cerinta facultativa)
    int getPret() const {
        if (format == "IMAX") return 50;
        if (format == "4DX")  return 45;
        if (format == "3D")   return 35;
        return 30; // 2D
    }

    std::string getInfo() const {
        return format + " | " + ora + " | Sala " + std::to_string(nrSala);
    }
};

// ============================================================
// CLASA SALA - matrice de locuri
// ============================================================
class Sala {
public:
    enum StareLoc { LIBER, OCUPAT, SELECTAT };

    int nrSala;
    int randuri;
    int coloane;
    std::vector<std::vector<StareLoc>> locuri; // matrice de locuri

    Sala(int nr, int r, int c) : nrSala(nr), randuri(r), coloane(c) {
        locuri.assign(r, std::vector<StareLoc>(c, LIBER));
    }

    // Ocupa un loc - arunca exceptie daca e invalid sau deja ocupat
    void ocupaLoc(int rand, int col) {
        if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
            throw std::out_of_range("Index loc invalid: R" +
                std::to_string(rand+1) + "C" + std::to_string(col+1));
        if (locuri[rand][col] == OCUPAT)
            throw std::runtime_error("Locul R" + std::to_string(rand+1) +
                "C" + std::to_string(col+1) + " este deja ocupat!");
        locuri[rand][col] = OCUPAT;
    }

    void selecteazaLoc(int rand, int col) {
        if (rand < 0 || rand >= randuri || col < 0 || col >= coloane)
            throw std::out_of_range("Index loc invalid!");
        if (locuri[rand][col] == LIBER)
            locuri[rand][col] = SELECTAT;
        else if (locuri[rand][col] == SELECTAT)
            locuri[rand][col] = LIBER;
    }

    void reseteazaSelectie() {
        for (auto& rand : locuri)
            for (auto& loc : rand)
                if (loc == SELECTAT) loc = LIBER;
    }

    int numarLocuriLibere() const {
        int cnt = 0;
        for (auto& rand : locuri)
            for (auto& loc : rand)
                if (loc == LIBER) cnt++;
        return cnt;
    }

    std::vector<std::pair<int,int>> getLocuriSelectate() const {
        std::vector<std::pair<int,int>> sel;
        for (int r = 0; r < randuri; r++)
            for (int c = 0; c < coloane; c++)
                if (locuri[r][c] == SELECTAT)
                    sel.push_back({r, c});
        return sel;
    }

    StareLoc getStare(int r, int c) const { return locuri[r][c]; }
};

// ============================================================
// CLASA REZERVARE
// ============================================================
class Rezervare {
public:
    std::string client;
    std::string filmTitlu;
    std::string metoda;
    std::vector<std::string> locuri;
    int total;

    Rezervare(const std::string& c, const std::string& f,
              const std::string& m, const std::vector<std::string>& l, int t)
        : client(c), filmTitlu(f), metoda(m), locuri(l), total(t) {}
};

// ============================================================
// CLASA CINEMATOGRAF - agregeaza sali si filme
// ============================================================
class Cinematograf {
public:
    std::string nume;
    std::vector<Film> filme;
    std::vector<Sala> sali;
    std::vector<Rezervare> rezervari;

    Cinematograf(const std::string& n) : nume(n) {}

    void adaugaFilm(const Film& f) { filme.push_back(f); }
    void adaugaSala(const Sala& s) { sali.push_back(s); }

    // Afiseaza filmele disponibile
    std::vector<Film>& getFilme() { return filme; }

    // Returneaza sala pentru un film
    Sala& getSalaForFilm(int filmIdx) {
        int nrSala = filme[filmIdx].nrSala;
        for (auto& s : sali)
            if (s.nrSala == nrSala) return s;
        throw std::runtime_error("Sala negasita pentru filmul selectat!");
    }

    void adaugaRezervare(const Rezervare& r) { rezervari.push_back(r); }
};

// ============================================================
// HELPER UI
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

sf::Text makeText(const std::string& s, const sf::Font& f,
                  unsigned size, sf::Color col, float x, float y) {
    sf::Text t(f); t.setString(s); t.setCharacterSize(size);
    t.setFillColor(col); t.setPosition({x,y}); return t;
}

sf::RectangleShape makeRect(float x,float y,float w,float h,
    sf::Color fill, sf::Color outline=sf::Color::Transparent, float thick=0.f) {
    sf::RectangleShape r; r.setPosition({x,y}); r.setSize({w,h});
    r.setFillColor(fill); r.setOutlineColor(outline); r.setOutlineThickness(thick);
    return r;
}

// ============================================================
// MAIN
// ============================================================
int main() {
    sf::RenderWindow window(sf::VideoMode({960u,640u}), "* Happy Cinema");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf")) return -1;

    // Initializeaza cinematograful
    Cinematograf cinema("Happy Cinema");

    // Adauga filme
    cinema.adaugaFilm(Film("Interstellar",  "IMAX","SF",     "20:00",169,1));
    cinema.adaugaFilm(Film("Dune: Part Two","4DX", "Actiune","18:30",166,2));
    cinema.adaugaFilm(Film("Oppenheimer",   "3D",  "Drama",  "21:45",180,3));
    cinema.adaugaFilm(Film("The Batman",    "2D",  "Actiune","16:00",176,4));

    // Adauga sali
    cinema.adaugaSala(Sala(1, 6, 10));
    cinema.adaugaSala(Sala(2, 6, 10));
    cinema.adaugaSala(Sala(3, 6, 10));
    cinema.adaugaSala(Sala(4, 6, 10));

    // Seteaza locuri ocupate initial pentru fiecare sala
    try {
        auto& s1 = cinema.sali[0];
        for (auto& p : std::vector<std::pair<int,int>>{{0,2},{0,7},{1,4},{2,1},{2,8},{3,0},{4,5},{5,3},{5,9}})
            s1.ocupaLoc(p.first, p.second);

        auto& s2 = cinema.sali[1];
        for (auto& p : std::vector<std::pair<int,int>>{{0,1},{1,3},{2,5},{3,7},{4,2},{5,8},{0,5},{3,3}})
            s2.ocupaLoc(p.first, p.second);

        auto& s3 = cinema.sali[2];
        for (auto& p : std::vector<std::pair<int,int>>{{0,5},{1,1},{2,9},{3,4},{4,0},{5,6},{1,8},{4,7}})
            s3.ocupaLoc(p.first, p.second);

        auto& s4 = cinema.sali[3];
        for (auto& p : std::vector<std::pair<int,int>>{{0,3},{1,7},{2,2},{3,9},{4,4},{5,1},{2,6},{3,5}})
            s4.ocupaLoc(p.first, p.second);
    } catch (const std::exception& e) {
        // Eroare la initializare - nu ar trebui sa se intample
    }

    Screen currentScreen = SALA;
    int selFilmIdx = 0;

    const float GX=310.f,GY=130.f,SW=26.f,SH=22.f,SGX=5.f,SGY=5.f;

    // Stare comanda curenta
    struct ComandaCurenta {
        std::string client, film;
        std::vector<std::string> locuri;
        int total = 0;
        bool activa = false;
    } comanda;

    int metodaPlata = 0;
    bool plataConfirmata = false;
    sf::Clock plataClock;

    std::vector<std::string> navL={"[ SALA ]","[ FILME ]","[ PLATA ]","[ RAPORT ]"};
    std::vector<Button> navBtns;
    for(int i=0;i<4;i++)
        navBtns.emplace_back(navL[i],font,220.f+i*185.f,8.f,175.f,32.f,false);

    Button btnContinua("CONTINUA >>",font,310.f,480.f,150.f,34.f,true);
    Button btnRst("RESETEAZA",font,475.f,480.f,120.f,34.f,false);
    Button btnCash("CASH",font,310.f,320.f,180.f,50.f,false);
    Button btnCard("CARD",font,510.f,320.f,180.f,50.f,false);
    Button btnConfirma("CONFIRMA PLATA",font,360.f,430.f,240.f,44.f,true);
    Button btnInapoi("< INAPOI",font,310.f,490.f,150.f,34.f,false);

    std::string clientName=""; bool typingClient=false;
    auto inputBox=makeRect(310.f,440.f,275.f,28.f,C_SURFACE,C_BORDER,1.f);
    std::string errMsg=""; bool showErr=false;
    sf::Clock errClock;

    while(window.isOpen()){
        while(auto ev=window.pollEvent()){
            if(ev->is<sf::Event::Closed>()) window.close();
            if(auto* mb=ev->getIf<sf::Event::MouseButtonPressed>()){
                if(mb->button==sf::Mouse::Button::Left){
                    sf::Vector2f mp(static_cast<float>(mb->position.x),
                                    static_cast<float>(mb->position.y));

                    for(int i=0;i<4;i++){
                        if(navBtns[i].contains(mp)){
                            if(i==2) break;
                            currentScreen=(Screen)i;
                        }
                    }

                    if(currentScreen==SALA){
                        Sala& salaC = cinema.getSalaForFilm(selFilmIdx);
                        int ROWS = salaC.randuri, COLS = salaC.coloane;

                        // Click pe locuri
                        for(int r=0;r<ROWS;r++)
                            for(int c=0;c<COLS;c++){
                                sf::FloatRect rect(
                                    {GX+c*(SW+SGX), GY+r*(SH+SGY)},
                                    {22.f, 18.f});
                                if(rect.contains(mp) && salaC.getStare(r,c)!=Sala::OCUPAT){
                                    try {
                                        salaC.selecteazaLoc(r, c);
                                    } catch(const std::exception& e) {
                                        errMsg = e.what();
                                        showErr=true; errClock.restart();
                                    }
                                }
                            }

                        typingClient=inputBox.getGlobalBounds().contains(mp);

                        if(btnContinua.contains(mp)){
                            auto sel = salaC.getLocuriSelectate();
                            if(sel.empty()){
                                errMsg="! Selecteaza cel putin un loc!";
                                showErr=true; errClock.restart();
                            } else {
                                std::vector<std::string> locStr;
                                for(auto& p : sel)
                                    locStr.push_back("R"+std::to_string(p.first+1)+"C"+std::to_string(p.second+1));
                                comanda.client = clientName.empty()?"Anonim":clientName;
                                comanda.film   = cinema.getFilme()[selFilmIdx].titlu;
                                comanda.locuri = locStr;
                                comanda.total  = (int)sel.size() * cinema.getFilme()[selFilmIdx].getPret();
                                comanda.activa = true;
                                metodaPlata    = 0;
                                plataConfirmata= false;
                                currentScreen  = PLATA;
                            }
                        }

                        if(btnRst.contains(mp)){
                            salaC.reseteazaSelectie();
                            clientName=""; showErr=false; typingClient=false;
                        }

                        for(int i=0;i<(int)cinema.getFilme().size();i++){
                            auto fb=makeRect(8,200.f+i*38.f,278,32,C_SURFACE);
                            if(fb.getGlobalBounds().contains(mp) && selFilmIdx!=i){
                                cinema.getSalaForFilm(selFilmIdx).reseteazaSelectie();
                                selFilmIdx=i;
                                clientName=""; showErr=false; typingClient=false;
                            }
                        }
                    }

                    else if(currentScreen==PLATA){
                        if(btnCash.contains(mp)) metodaPlata=1;
                        if(btnCard.contains(mp)) metodaPlata=2;

                        if(btnConfirma.contains(mp)){
                            if(metodaPlata==0){
                                errMsg="! Alege metoda de plata!";
                                showErr=true; errClock.restart();
                            } else if(comanda.activa){
                                try {
                                    Sala& salaC = cinema.getSalaForFilm(selFilmIdx);
                                    // Confirma locurile selectate ca ocupate
                                    for(int r=0;r<salaC.randuri;r++)
                                        for(int c=0;c<salaC.coloane;c++)
                                            if(salaC.getStare(r,c)==Sala::SELECTAT)
                                                salaC.ocupaLoc(r,c);

                                    std::string met=(metodaPlata==1)?"cash":"card";
                                    cinema.adaugaRezervare(Rezervare(
                                        comanda.client, comanda.film,
                                        met, comanda.locuri, comanda.total));

                                    comanda.activa=false;
                                    plataConfirmata=true;
                                    plataClock.restart();
                                    clientName=""; typingClient=false; showErr=false;
                                } catch(const std::exception& e){
                                    errMsg = std::string("Eroare: ") + e.what();
                                    showErr=true; errClock.restart();
                                }
                            }
                        }

                        if(btnInapoi.contains(mp)){
                            currentScreen=SALA;
                            comanda.activa=false;
                            metodaPlata=0;
                            showErr=false;
                        }
                    }
                }
            }
            if(auto* te=ev->getIf<sf::Event::TextEntered>()){
                if(typingClient && currentScreen==SALA){
                    if(te->unicode=='\b'){ if(!clientName.empty()) clientName.pop_back(); }
                    else if(te->unicode>=32&&te->unicode<128&&clientName.size()<24)
                        clientName+=(char)te->unicode;
                }
            }
        }

        if(showErr && errClock.getElapsedTime().asSeconds()>3.f) showErr=false;
        if(plataConfirmata && plataClock.getElapsedTime().asSeconds()>2.f){
            plataConfirmata=false;
            currentScreen=RAPORT;
        }

        window.clear(C_BG);

        // HEADER
        window.draw(makeRect(0,0,960,50,C_DARK));
        window.draw(makeRect(0,49,960,2,C_GOLD));
        window.draw(makeText("* HAPPY CINEMA",font,16,C_GOLD,16,10));
        window.draw(makeText("Sistem Rezervari v1.0",font,11,C_MUTED,16,30));
        std::time_t now=std::time(nullptr);
        char tb[16]; std::strftime(tb,sizeof(tb),"%H:%M:%S",std::localtime(&now));
        window.draw(makeText(tb,font,12,C_MUTED,870,18));

        // NAVBAR
        window.draw(makeRect(0,51,960,40,C_SURFACE2));
        window.draw(makeRect(0,90,960,1,C_BORDER));
        for(int i=0;i<4;i++){
            bool act=(currentScreen==(Screen)i);
            navBtns[i].box.setFillColor(act?C_DARK:sf::Color::Transparent);
            if(act) window.draw(makeRect(navBtns[i].box.getPosition().x,88,
                                          navBtns[i].box.getSize().x,3,C_GOLD));
            navBtns[i].draw(window,font);
        }

        // ======== SALA ========
        if(currentScreen==SALA){
            auto& filme = cinema.getFilme();
            Sala& salaC = cinema.getSalaForFilm(selFilmIdx);
            int ROWS=salaC.randuri, COLS=salaC.coloane;

            window.draw(makeRect(0,91,295,549,C_SURFACE,C_BORDER,1.f));
            window.draw(makeText("FILM SELECTAT",font,9,C_MUTED,12,105));
            window.draw(makeRect(8,120,278,50,C_DARK,C_BORDER,1.f));
            window.draw(makeText(filme[selFilmIdx].titlu,font,13,C_GOLD_L,16,128));
            window.draw(makeText(filme[selFilmIdx].getInfo(),font,10,C_MUTED,16,147));
            window.draw(makeText("Pret: "+std::to_string(filme[selFilmIdx].getPret())+" RON/bilet",font,10,C_GOLD,16,158));
            window.draw(makeText("ALEGE FILM",font,9,C_MUTED,12,185));
            for(int i=0;i<(int)filme.size();i++){
                bool isSel=(i==selFilmIdx);
                window.draw(makeRect(8,200.f+i*38.f,278,32,
                    isSel?C_SURFACE2:sf::Color::Transparent,isSel?C_GOLD:C_BORDER,1.f));
                window.draw(makeText(filme[i].titlu,font,12,isSel?C_GOLD:C_TEXT,16,208.f+i*38.f));
                window.draw(makeText(filme[i].format+" | "+std::to_string(filme[i].getPret())+" RON",font,10,isSel?C_GOLD:C_MUTED,16,222.f+i*38.f));
            }
            int nSel=(int)salaC.getLocuriSelectate().size();
            window.draw(makeText("SUMAR",font,9,C_MUTED,12,362));
            window.draw(makeRect(8,375,278,70,C_DARK,C_BORDER,1.f));
            window.draw(makeText("Locuri selectate:",font,11,C_MUTED,16,383));
            window.draw(makeText(std::to_string(nSel),font,11,C_GOLD,220,383));
            window.draw(makeText("Total:",font,11,C_MUTED,16,403));
            window.draw(makeText(std::to_string(nSel*filme[selFilmIdx].getPret())+" RON",font,11,C_GOLD,200,403));

            window.draw(makeText("SALA "+std::to_string(filme[selFilmIdx].nrSala)+
                " - "+filme[selFilmIdx].titlu,font,10,C_GOLD,310,100));
            window.draw(makeRect(GX-10,113,COLS*(SW+SGX)+10,12,C_BORDER));
            window.draw(makeText("E C R A N",font,8,C_MUTED,GX+110,115));
            for(int r=0;r<ROWS;r++)
                window.draw(makeText("R"+std::to_string(r+1),font,9,C_MUTED,GX-28,GY+r*(SH+SGY)+4));

            // Deseneaza locurile din matricea Sala
            for(int r=0;r<ROWS;r++)
                for(int c=0;c<COLS;c++){
                    sf::RectangleShape loc;
                    loc.setSize({22.f,18.f});
                    loc.setPosition({GX+c*(SW+SGX), GY+r*(SH+SGY)});
                    loc.setOutlineThickness(1.f);
                    switch(salaC.getStare(r,c)){
                        case Sala::LIBER:
                            loc.setFillColor({13,43,26}); loc.setOutlineColor(C_GREEN); break;
                        case Sala::OCUPAT:
                            loc.setFillColor({43,13,13}); loc.setOutlineColor(C_RED); break;
                        case Sala::SELECTAT:
                            loc.setFillColor(C_GOLD); loc.setOutlineColor(C_GOLD_L); break;
                    }
                    window.draw(loc);
                }

            float lx=GX,ly=GY+ROWS*(SH+SGY)+10;
            window.draw(makeRect(lx,ly,10,8,{13,43,26},C_GREEN,1));
            window.draw(makeText("liber",font,9,C_MUTED,lx+14,ly));
            window.draw(makeRect(lx+60,ly,10,8,{43,13,13},C_RED,1));
            window.draw(makeText("ocupat",font,9,C_MUTED,lx+74,ly));
            window.draw(makeRect(lx+130,ly,10,8,C_GOLD,C_GOLD_L,1));
            window.draw(makeText("selectat",font,9,C_MUTED,lx+144,ly));

            window.draw(makeText("NUME CLIENT",font,9,C_MUTED,310,428));
            inputBox.setOutlineColor(typingClient?C_GOLD:C_BORDER);
            window.draw(inputBox);
            std::string disp=clientName+(typingClient?"|":"");
            window.draw(makeText(disp.empty()?"ex: Popescu Ion":disp,
                font,12,clientName.empty()?C_MUTED:C_TEXT,318,446));
            btnContinua.draw(window,font);
            btnRst.draw(window,font);
            if(showErr){
                window.draw(makeRect(310,525,620,28,{43,13,13},C_RED,1.f));
                window.draw(makeText(errMsg,font,11,C_RED,318,531));
            }
        }
        // ======== FILME ========
        else if(currentScreen==FILME){
            auto& filme = cinema.getFilme();
            window.draw(makeText("FILME IN PROGRAM",font,10,C_GOLD,20,105));
            window.draw(makeRect(20,120,920,1,C_BORDER));
            std::vector<std::string> hdrs={"#","TITLU","FORMAT","DURATA","GEN","ORA","SALA","PRET"};
            float hx[]={20,60,260,360,440,560,650,740};
            for(int i=0;i<8;i++) window.draw(makeText(hdrs[i],font,9,C_MUTED,hx[i],126));
            window.draw(makeRect(20,140,920,1,C_BORDER));
            for(int i=0;i<(int)filme.size();i++){
                float ry=150.f+i*40.f;
                window.draw(makeRect(20,ry,920,34,i%2==0?C_SURFACE:sf::Color::Transparent));
                window.draw(makeText(std::to_string(i+1),font,12,C_MUTED,hx[0],ry+8));
                window.draw(makeText(filme[i].titlu,font,13,C_TEXT,hx[1],ry+8));
                window.draw(makeText(filme[i].format,font,12,C_GOLD,hx[2],ry+8));
                window.draw(makeText(std::to_string(filme[i].durata)+"min",font,12,C_MUTED,hx[3],ry+8));
                window.draw(makeText(filme[i].gen,font,12,C_MUTED,hx[4],ry+8));
                window.draw(makeText(filme[i].ora,font,12,C_GOLD_L,hx[5],ry+8));
                window.draw(makeText("Sala "+std::to_string(filme[i].nrSala),font,12,C_TEXT,hx[6],ry+8));
                window.draw(makeText(std::to_string(filme[i].getPret())+" RON",font,12,C_GOLD,hx[7],ry+8));
                window.draw(makeRect(20,ry+34,920,1,C_BORDER));
            }
        }
        // ======== PLATA ========
        else if(currentScreen==PLATA){
            if(plataConfirmata){
                window.draw(makeRect(280,220,400,120,C_DARK,C_GREEN,2.f));
                window.draw(makeText("✓ PLATA CONFIRMATA!",font,18,C_GREEN,330,250));
                window.draw(makeText("Redirectionare catre Raport...",font,11,C_MUTED,340,290));
            } else {
                window.draw(makeText("FINALIZARE REZERVARE",font,13,C_GOLD,20,105));
                window.draw(makeRect(20,125,920,1,C_BORDER));
                window.draw(makeRect(20,135,920,120,C_SURFACE,C_BORDER,1.f));
                window.draw(makeText("SUMAR COMANDA",font,9,C_MUTED,35,148));
                window.draw(makeText("Client:",font,11,C_MUTED,35,168));
                window.draw(makeText(comanda.client,font,13,C_TEXT,130,168));
                window.draw(makeText("Film:",font,11,C_MUTED,35,190));
                window.draw(makeText(comanda.film,font,13,C_GOLD_L,130,190));
                std::string locsStr;
                for(int i=0;i<(int)comanda.locuri.size();i++){if(i)locsStr+=", ";locsStr+=comanda.locuri[i];}
                window.draw(makeText("Locuri:",font,11,C_MUTED,35,212));
                window.draw(makeText(locsStr,font,11,C_TEXT,130,212));
                window.draw(makeText("Total de plata:",font,13,C_MUTED,35,234));
                window.draw(makeText(std::to_string(comanda.total)+" RON",font,16,C_GOLD,200,232));

                window.draw(makeText("METODA DE PLATA",font,9,C_MUTED,20,278));
                window.draw(makeRect(20,292,920,1,C_BORDER));
                btnCash.box.setFillColor(metodaPlata==1?C_GOLD:C_SURFACE);
                btnCash.box.setOutlineColor(metodaPlata==1?C_GOLD_L:C_BORDER);
                window.draw(btnCash.box);
                window.draw(makeText("CASH",font,16,metodaPlata==1?C_DARK:C_TEXT,370,333));
                window.draw(makeText("Plata la casa",font,10,metodaPlata==1?C_DARK:C_MUTED,348,355));
                btnCard.box.setFillColor(metodaPlata==2?C_GOLD:C_SURFACE);
                btnCard.box.setOutlineColor(metodaPlata==2?C_GOLD_L:C_BORDER);
                window.draw(btnCard.box);
                window.draw(makeText("CARD",font,16,metodaPlata==2?C_DARK:C_TEXT,568,333));
                window.draw(makeText("Plata cu cardul",font,10,metodaPlata==2?C_DARK:C_MUTED,546,355));
                btnConfirma.draw(window,font);
                btnInapoi.draw(window,font);
                if(showErr){
                    window.draw(makeRect(310,540,340,28,{43,13,13},C_RED,1.f));
                    window.draw(makeText(errMsg,font,11,C_RED,318,546));
                }
            }
        }
        // ======== RAPORT ========
        else if(currentScreen==RAPORT){
            auto& rez = cinema.rezervari;
            window.draw(makeText("RAPORT REZERVARI",font,10,C_GOLD,20,105));
            window.draw(makeRect(20,120,920,1,C_BORDER));
            int totalInc=0,totalBilete=0;
            for(auto& r:rez){totalInc+=r.total;totalBilete+=(int)r.locuri.size();}
            window.draw(makeRect(20,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeRect(240,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeRect(460,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeText(std::to_string(rez.size()),font,22,C_GOLD,30,140));
            window.draw(makeText("REZERVARI",font,9,C_MUTED,30,168));
            window.draw(makeText(std::to_string(totalInc)+" RON",font,18,C_GOLD,250,140));
            window.draw(makeText("INCASARI",font,9,C_MUTED,250,168));
            window.draw(makeText(std::to_string(totalBilete),font,22,C_GOLD,470,140));
            window.draw(makeText("BILETE",font,9,C_MUTED,470,168));
            float ry=210;
            std::vector<std::string> rh={"#","CLIENT","FILM","METODA","LOCURI","TOTAL"};
            float rx[]={20,55,180,370,470,720};
            for(int i=0;i<6;i++) window.draw(makeText(rh[i],font,9,C_MUTED,rx[i],ry));
            window.draw(makeRect(20,ry+16,920,1,C_BORDER));
            if(rez.empty())
                window.draw(makeText("— nicio rezervare inregistrata —",font,12,C_MUTED,280,ry+30));
            for(int i=0;i<(int)rez.size();i++){
                float ty=ry+26.f+i*34.f;
                auto& r=rez[i];
                std::string locs;
                for(int j=0;j<(int)r.locuri.size();j++){if(j)locs+=", ";locs+=r.locuri[j];}
                window.draw(makeText("#"+std::to_string(i+1),font,11,C_MUTED,rx[0],ty));
                window.draw(makeText(r.client,font,12,C_TEXT,rx[1],ty));
                window.draw(makeText(r.filmTitlu,font,12,C_TEXT,rx[2],ty));
                window.draw(makeText(r.metoda,font,12,r.metoda=="cash"?C_GREEN:C_GOLD,rx[3],ty));
                window.draw(makeText(locs,font,11,C_MUTED,rx[4],ty));
                window.draw(makeText(std::to_string(r.total)+" RON",font,12,C_GOLD,rx[5],ty));
                window.draw(makeRect(20,ty+20,920,1,C_BORDER));
            }
        }

        window.display();
    }
    return 0;
}