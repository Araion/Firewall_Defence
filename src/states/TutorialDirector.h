#pragma once
#include "util/Button.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <utility>

class PlayState;
class Enemy;
class Path;

// =============================================================
//  TutorialDirector - liniowy, scisle sterowany samouczek.
//  Prowadzi gracza KROK PO KROKU i blokuje wszystko poza zaplanowana
//  akcja (bramka w PlayState): omawia jedna wieze, podswietla ja i
//  pole, KAZE postawic ja w konkretnym miejscu, potem wysyla wroga,
//  po chwili pokazuje o nim tekst, czeka na jego pokonanie i przechodzi
//  do kolejnej wiezy. Calkowicie deterministyczny.
//  Typy krokow: Info (panel + DALEJ), Build (wymuszona budowa),
//  Combat (spawn + opozniony baner).
// =============================================================
class TutorialDirector {
public:
    explicit TutorialDirector(PlayState& ps);

    void update(float dt);
    void handleEvent(const sf::Event& e);
    void draw(sf::RenderWindow& window);

    bool panelOpen() const { return m_panelOpen; } // Info -> zamraza symulacje
    bool finished() const { return m_finished; }
    void devNext(); // tylko do automatycznych testow

private:
    enum Type { Info, Build, Combat };
    struct Step {
        Type type = Info;
        std::string title;
        std::string text;
        int slot = -1;       // Build: typ wiezy (TowerType)
        int spotIdx = -1;    // Build: indeks wskazanego miejsca budowy
        int kind = -1;       // Combat: typ wroga (0 Virus,1 Trojan,2 Worm,3 Glitch,4 Boss,5 Proxy)
        int count = 0;       // Combat: liczba wrogow (gdy 'seq' puste)
        bool encrypted = false;
        bool breach = false;
        bool freeBuild = false;       // Combat: pozwol stawiac wieze gdziekolwiek (walka z bossem)
        // Jawna sekwencja spawnu (typ wroga, czy zaszyfrowany). Puste = uzyj kind/count/encrypted.
        // Dzieki temu jeden krok moze wyslac mix (np. zwykly + zaszyfrowany, albo horda z Proxy).
        std::vector<std::pair<int, bool>> seq;
    };

    void buildSteps();
    void pickSpots();
    void enterStep(int i);
    void onNext();
    void spawnOne();
    std::unique_ptr<Enemy> makeEnemy(int kind, const Path* path, bool enc);

    PlayState& m_ps;
    std::vector<Step> m_steps;
    std::vector<sf::Vector2f> m_spots;
    static constexpr float kSpotRadius = 64.f; // promien podswietlenia sugestii
    int  m_index = -1;
    bool m_panelOpen = false;        // krok Info
    bool m_waiting = false;
    bool m_finished = false;
    bool m_hasSpot = false;          // czy biezacy krok ma sugerowane miejsce
    sf::Vector2f m_curSpot;          // sugerowane miejsce biezacego kroku Build
    int  m_towersAtStart = 0;        // liczba wiez na poczatku kroku Build (do zaliczenia)

    bool  m_sawEnemies = false; // czy w biezacej walce pojawil sie juz zywy wrog (anty-bug:
    // spawn jest odlozony, wiec enemies() jest puste przez 1 klatke)
    float m_stepDelay = 0.f;    // krotka przerwa miedzy krokami (np. po postawieniu wiezy)

    std::vector<std::pair<int, bool>> m_spawnSeq; // kolejka spawnu biezacego kroku (typ, zaszyfrowany)
    int   m_spawnIdx = 0;     // ile z kolejki juz wyslano
    float m_spawnTimer = 0.f;
    int   m_laneToggle = 0;

    float m_anim = 0.f;
    Button m_btnNext;
};
