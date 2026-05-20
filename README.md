# Happy Cinema - Sistem Rezervari

Aplicatie de rezervare bilete cinema scrisa in C++ cu SFML 3.

## Dependinte

```bash
sudo apt update
sudo apt install libsfml-dev cmake build-essential
```

## Font

```bash
mkdir -p assets
cp /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf assets/font.ttf
```

## Compilare si rulare

```bash
mkdir build
cmake -B build
cmake --build build
./build/cinema
```

## Functionalitati

- Selectare locuri in sala de cinema
- 4 filme cu locuri ocupate diferite
- Flux rezervare: SALA -> PLATA (cash/card) -> RAPORT
- Raport rezervari cu incasari totale
