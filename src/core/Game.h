#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include "managers/ScoreManager.h"
#include "managers/AudioManager.h"

class GameState;

// =============================================================
//  Game - rdzen aplikacji, zarzadza wszystkim
// =============================================================
class Game {
public:
    Game();
    ~Game();

    // Uruchamia glowna petle gry
    void run();

    // --- Zarzadzanie stanami gry ---
    // Stan to aktualny ekran/tryb gry, np. menu, gra
    // Game moze zmieniac stan albo usunac obecny
    void changeState(std::unique_ptr<GameState> state); // zastepuje obecny stan nowym
    void quit();                                        // zamkniecie gry

    // --- Dostep do zasobow wspoldzielonych ---
    sf::RenderWindow& getWindow()    { return m_window; }
    ResourceManager&  getResources() { return m_resources; }
    ConfigManager&    getConfig()    { return m_config; }
    ScoreManager&     getScores()    { return m_scores; }
    AudioManager&     getAudio()     { return m_audio; }

private:
    void processEvents();           // obsluguje zdarzenia na klawiaturze, myszce
    void render();                  // rysuje obecny stan gry w oknie
    void applyPendingStateChange(); // wykonuje zaplanowane zmiany stanow

    // Glowne okno gry
    sf::RenderWindow m_window;

    // Menedzery odpowiadaja za konkretne czesci gry
    // Dzieki nim klasa Game nie musi sama ladowac zasobow,
    // trzymac ustawien, wynikow ani obslugiwac dzwieku
    ResourceManager  m_resources;
    ConfigManager    m_config;
    ScoreManager     m_scores;
    AudioManager     m_audio{m_resources}; // inicjowany referencja do zasobow

    // Aktualny stan gry, np. menu albo rozgrywka
    std::unique_ptr<GameState> m_state;

    // Zaplanowane zmiany stanow wykonujemy pozniej,
    // zeby nie zmieniac stanu w trakcie jego update/draw
    enum class PendingType { None, Change, Quit };

    // Pojedyncza zaplanowana zmiana stanu
    struct PendingOp {
        PendingType type = PendingType::None;
        std::unique_ptr<GameState> state;
    };

    // Zmiana stanow do wykonania w bezpiecznym momencie
    PendingOp m_pending;
};
