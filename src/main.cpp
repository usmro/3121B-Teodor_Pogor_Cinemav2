#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <ctime>

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

struct Seat {
    int row, col;
    enum State { FREE, TAKEN, SELECTED } state = FREE;
    sf::RectangleShape shape;
    Seat(int r, int c, float x, float y) : row(r), col(c) {
        shape.setSize({22.f, 18.f});
        shape.setPosition({x, y});
        shape.setOutlineThickness(1.f);
    }
    void update() {
        switch(state) {
            case FREE:     shape.setFillColor({13,43,26}); shape.setOutlineColor(C_GREEN);  break;
            case TAKEN:    shape.setFillColor({43,13,13}); shape.setOutlineColor(C_RED);    break;
            case SELECTED: shape.setFillColor(C_GOLD);     shape.setOutlineColor(C_GOLD_L); break;
        }
    }
    bool contains(sf::Vector2f p) { return shape.getGlobalBounds().contains(p); }
    void draw(sf::RenderWindow& w) { update(); w.draw(shape); }
};

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

struct Film {
    std::string titlu,format,gen,ora;
    int durata,sala;
    std::vector<std::pair<int,int>> taken;
};

struct Rezervare {
    std::string client,film,metoda;
    std::vector<std::string> locuri;
    int total;
};

// Stare pentru comanda curenta (intre SALA si PLATA)
struct ComandaCurenta {
    std::string client;
    std::string film;
    std::vector<std::string> locuri;
    int total = 0;
    bool activa = false;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({960u,640u}), "* Happy Cinema");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf")) return -1;

    Screen currentScreen = SALA;
    int selFilmIdx = 0;

    const int ROWS=6,COLS=10,PRICE=30;
    const float GX=310.f,GY=130.f,SW=26.f,SH=22.f,SGX=5.f,SGY=5.f;

    std::vector<Film> filme = {
        {"Interstellar",  "IMAX","SF",     "20:00",169,1, {{0,2},{0,7},{1,4},{2,1},{2,8},{3,0},{4,5},{5,3},{5,9}}},
        {"Dune: Part Two","4DX", "Actiune","18:30",166,2, {{0,1},{1,3},{2,5},{3,7},{4,2},{5,8},{0,5},{3,3}}},
        {"Oppenheimer",   "3D",  "Drama",  "21:45",180,3, {{0,5},{1,1},{2,9},{3,4},{4,0},{5,6},{1,8},{4,7}}},
        {"The Batman",    "2D",  "Actiune","16:00",176,4, {{0,3},{1,7},{2,2},{3,9},{4,4},{5,1},{2,6},{3,5}}},
    };

    std::vector<Rezervare> rezervari;
    std::vector<Seat> seats;
    ComandaCurenta comanda;

    // Metoda plata selectata: 0=niciuna, 1=cash, 2=card
    int metodaPlata = 0;
    bool plataConfirmata = false;
    sf::Clock plataClock;

    auto loadSeats = [&](int filmIdx) {
        seats.clear();
        for(int r=0;r<ROWS;r++)
            for(int c=0;c<COLS;c++){
                Seat s(r,c,GX+c*(SW+SGX),GY+r*(SH+SGY));
                for(auto& p : filme[filmIdx].taken)
                    if(p.first==r && p.second==c) s.state=Seat::TAKEN;
                seats.push_back(s);
            }
    };

    loadSeats(selFilmIdx);

    std::vector<std::string> navL={"[ SALA ]","[ FILME ]","[ PLATA ]","[ RAPORT ]"};
    std::vector<Button> navBtns;
    for(int i=0;i<4;i++)
        navBtns.emplace_back(navL[i],font,220.f+i*185.f,8.f,175.f,32.f,false);

    // Butoane SALA
    Button btnContinua("CONTINUA >>",font,310.f,480.f,150.f,34.f,true);
    Button btnRst("RESETEAZA",font,475.f,480.f,120.f,34.f,false);

    // Butoane PLATA
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

                    // Navigare navbar (doar SALA, FILME, RAPORT - nu direct PLATA)
                    for(int i=0;i<4;i++){
                        if(navBtns[i].contains(mp)){
                            if(i==2) break; // PLATA nu e accesibila direct
                            currentScreen=(Screen)i;
                        }
                    }

                    // ---- LOGICA SALA ----
                    if(currentScreen==SALA){
                        for(auto& s:seats)
                            if(s.state!=Seat::TAKEN && s.contains(mp))
                                s.state=(s.state==Seat::FREE)?Seat::SELECTED:Seat::FREE;

                        typingClient=inputBox.getGlobalBounds().contains(mp);

                        // CONTINUA >> -> merge la PLATA
                        if(btnContinua.contains(mp)){
                            std::vector<std::string> sel;
                            for(auto& s:seats)
                                if(s.state==Seat::SELECTED)
                                    sel.push_back("R"+std::to_string(s.row+1)+"C"+std::to_string(s.col+1));

                            if(sel.empty()){
                                errMsg="! Selecteaza cel putin un loc!";
                                showErr=true; errClock.restart();
                            } else {
                                // Pregateste comanda si du-te la PLATA
                                comanda.client = clientName.empty()?"Anonim":clientName;
                                comanda.film   = filme[selFilmIdx].titlu;
                                comanda.locuri = sel;
                                comanda.total  = (int)sel.size()*PRICE;
                                comanda.activa = true;
                                metodaPlata    = 0;
                                plataConfirmata= false;
                                currentScreen  = PLATA;
                            }
                        }

                        if(btnRst.contains(mp)){
                            for(auto& s:seats) if(s.state==Seat::SELECTED) s.state=Seat::FREE;
                            clientName=""; showErr=false; typingClient=false;
                        }

                        for(int i=0;i<(int)filme.size();i++){
                            auto fb=makeRect(8,200.f+i*38.f,278,32,C_SURFACE);
                            if(fb.getGlobalBounds().contains(mp) && selFilmIdx!=i){
                                selFilmIdx=i;
                                loadSeats(selFilmIdx);
                                clientName=""; showErr=false; typingClient=false;
                            }
                        }
                    }

                    // ---- LOGICA PLATA ----
                    else if(currentScreen==PLATA){
                        if(btnCash.contains(mp)) metodaPlata=1;
                        if(btnCard.contains(mp)) metodaPlata=2;

                        if(btnConfirma.contains(mp)){
                            if(metodaPlata==0){
                                errMsg="! Alege metoda de plata!";
                                showErr=true; errClock.restart();
                            } else if(comanda.activa){
                                // Finalizeaza rezervarea
                                std::string met = (metodaPlata==1)?"cash":"card";
                                rezervari.push_back({comanda.client,comanda.film,met,comanda.locuri,comanda.total});

                                // Marcheaza locurile ca ocupate
                                for(auto& s:seats)
                                    if(s.state==Seat::SELECTED){
                                        s.state=Seat::TAKEN;
                                        filme[selFilmIdx].taken.push_back({s.row,s.col});
                                    }

                                comanda.activa=false;
                                plataConfirmata=true;
                                plataClock.restart();
                                clientName="";
                                typingClient=false;
                                showErr=false;
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

        // ---- HEADER ----
        window.draw(makeRect(0,0,960,50,C_DARK));
        window.draw(makeRect(0,49,960,2,C_GOLD));
        window.draw(makeText("* HAPPY CINEMA",font,16,C_GOLD,16,10));
        window.draw(makeText("Sistem Rezervari v1.0",font,11,C_MUTED,16,30));
        std::time_t now=std::time(nullptr);
        char tb[16]; std::strftime(tb,sizeof(tb),"%H:%M:%S",std::localtime(&now));
        window.draw(makeText(tb,font,12,C_MUTED,870,18));

        // ---- NAVBAR ----
        window.draw(makeRect(0,51,960,40,C_SURFACE2));
        window.draw(makeRect(0,90,960,1,C_BORDER));
        for(int i=0;i<4;i++){
            bool act=(currentScreen==(Screen)i);
            navBtns[i].box.setFillColor(act?C_DARK:sf::Color::Transparent);
            // PLATA apare activa doar daca esti pe ea
            navBtns[i].box.setOutlineColor(i==2&&currentScreen!=PLATA ? sf::Color::Transparent : C_BORDER);
            if(act) window.draw(makeRect(navBtns[i].box.getPosition().x,88,
                                          navBtns[i].box.getSize().x,3,C_GOLD));
            navBtns[i].draw(window,font);
        }

        // ======== ECRAN SALA ========
        if(currentScreen==SALA){
            window.draw(makeRect(0,91,295,549,C_SURFACE,C_BORDER,1.f));
            window.draw(makeText("FILM SELECTAT",font,9,C_MUTED,12,105));
            window.draw(makeRect(8,120,278,50,C_DARK,C_BORDER,1.f));
            window.draw(makeText(filme[selFilmIdx].titlu,font,13,C_GOLD_L,16,128));
            window.draw(makeText(filme[selFilmIdx].format+" | "+filme[selFilmIdx].ora+
                " | Sala "+std::to_string(filme[selFilmIdx].sala),font,10,C_MUTED,16,147));
            window.draw(makeText("ALEGE FILM",font,9,C_MUTED,12,185));
            for(int i=0;i<(int)filme.size();i++){
                bool isSel=(i==selFilmIdx);
                window.draw(makeRect(8,200.f+i*38.f,278,32,
                    isSel?C_SURFACE2:sf::Color::Transparent,isSel?C_GOLD:C_BORDER,1.f));
                window.draw(makeText(filme[i].titlu,font,12,isSel?C_GOLD:C_TEXT,16,208.f+i*38.f));
                window.draw(makeText(filme[i].format,font,10,isSel?C_GOLD:C_MUTED,16,222.f+i*38.f));
            }
            int nSel=0; for(auto& s:seats) if(s.state==Seat::SELECTED) nSel++;
            window.draw(makeText("SUMAR",font,9,C_MUTED,12,362));
            window.draw(makeRect(8,375,278,70,C_DARK,C_BORDER,1.f));
            window.draw(makeText("Locuri selectate:",font,11,C_MUTED,16,383));
            window.draw(makeText(std::to_string(nSel),font,11,C_GOLD,220,383));
            window.draw(makeText("Total:",font,11,C_MUTED,16,403));
            window.draw(makeText(std::to_string(nSel*PRICE)+" RON",font,11,C_GOLD,220,403));

            window.draw(makeText("SALA "+std::to_string(filme[selFilmIdx].sala)+
                " - "+filme[selFilmIdx].titlu,font,10,C_GOLD,310,100));
            window.draw(makeRect(GX-10,113,COLS*(SW+SGX)+10,12,C_BORDER));
            window.draw(makeText("E C R A N",font,8,C_MUTED,GX+110,115));
            for(int r=0;r<ROWS;r++)
                window.draw(makeText("R"+std::to_string(r+1),font,9,C_MUTED,GX-28,GY+r*(SH+SGY)+4));
            for(auto& s:seats) s.draw(window);

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
        // ======== ECRAN FILME ========
        else if(currentScreen==FILME){
            window.draw(makeText("FILME IN PROGRAM",font,10,C_GOLD,20,105));
            window.draw(makeRect(20,120,920,1,C_BORDER));
            std::vector<std::string> hdrs={"#","TITLU","FORMAT","DURATA","GEN","ORA","SALA"};
            float hx[]={20,60,260,360,440,560,650};
            for(int i=0;i<7;i++) window.draw(makeText(hdrs[i],font,9,C_MUTED,hx[i],126));
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
                window.draw(makeText("Sala "+std::to_string(filme[i].sala),font,12,C_TEXT,hx[6],ry+8));
                window.draw(makeRect(20,ry+34,920,1,C_BORDER));
            }
        }
        // ======== ECRAN PLATA ========
        else if(currentScreen==PLATA){
            if(plataConfirmata){
                // Mesaj de succes
                window.draw(makeRect(280,220,400,120,C_DARK,C_GREEN,2.f));
                window.draw(makeText("✓ PLATA CONFIRMATA!",font,18,C_GREEN,330,250));
                window.draw(makeText("Redirectionare catre Raport...",font,11,C_MUTED,340,290));
            } else {
                window.draw(makeText("FINALIZARE REZERVARE",font,13,C_GOLD,20,105));
                window.draw(makeRect(20,125,920,1,C_BORDER));

                // Sumar comanda
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

                // Alegere metoda plata
                window.draw(makeText("METODA DE PLATA",font,9,C_MUTED,20,278));
                window.draw(makeRect(20,292,920,1,C_BORDER));

                // Buton CASH
                btnCash.box.setFillColor(metodaPlata==1 ? C_GOLD : C_SURFACE);
                btnCash.box.setOutlineColor(metodaPlata==1 ? C_GOLD_L : C_BORDER);
                window.draw(btnCash.box);
                window.draw(makeText("CASH",font,16,metodaPlata==1?C_DARK:C_TEXT,370,333));
                window.draw(makeText("Plata la casa",font,10,metodaPlata==1?C_DARK:C_MUTED,348,355));

                // Buton CARD
                btnCard.box.setFillColor(metodaPlata==2 ? C_GOLD : C_SURFACE);
                btnCard.box.setOutlineColor(metodaPlata==2 ? C_GOLD_L : C_BORDER);
                window.draw(btnCard.box);
                window.draw(makeText("CARD",font,16,metodaPlata==2?C_DARK:C_TEXT,568,333));
                window.draw(makeText("Plata cu cardul",font,10,metodaPlata==2?C_DARK:C_MUTED,546,355));

                // Buton confirma
                btnConfirma.draw(window,font);
                btnInapoi.draw(window,font);

                if(showErr){
                    window.draw(makeRect(310,540,340,28,{43,13,13},C_RED,1.f));
                    window.draw(makeText(errMsg,font,11,C_RED,318,546));
                }
            }
        }
        // ======== ECRAN RAPORT ========
        else if(currentScreen==RAPORT){
            window.draw(makeText("RAPORT REZERVARI",font,10,C_GOLD,20,105));
            window.draw(makeRect(20,120,920,1,C_BORDER));
            int totalInc=0,totalBilete=0;
            for(auto& r:rezervari){totalInc+=r.total;totalBilete+=(int)r.locuri.size();}
            window.draw(makeRect(20,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeRect(240,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeRect(460,130,200,60,C_DARK,C_BORDER,1));
            window.draw(makeText(std::to_string(rezervari.size()),font,22,C_GOLD,30,140));
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
            if(rezervari.empty())
                window.draw(makeText("— nicio rezervare inregistrata —",font,12,C_MUTED,280,ry+30));
            for(int i=0;i<(int)rezervari.size();i++){
                float ty=ry+26.f+i*34.f;
                auto& r=rezervari[i];
                std::string locs;
                for(int j=0;j<(int)r.locuri.size();j++){if(j)locs+=", ";locs+=r.locuri[j];}
                window.draw(makeText("#"+std::to_string(i+1),font,11,C_MUTED,rx[0],ty));
                window.draw(makeText(r.client,font,12,C_TEXT,rx[1],ty));
                window.draw(makeText(r.film,font,12,C_TEXT,rx[2],ty));
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