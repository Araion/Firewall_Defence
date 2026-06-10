#pragma once
#include "core/GameState.h"
#include "core/GameObject.h"
#include "util/LevelMap.h"
#include <vector>
#include <memory>

class ServerCore;
class Path;

// =========================================+====================
// PlayState - stan rozgrywki
// Odpowiada za aktualizacje i rysowanie obiektow gry
// Obsluguje tez powrot do menu po nacisnieciu ESC
// =============================================================
class PlayState : public GameState {
public:
    explicit PlayState(Game& game);
    ~PlayState() override;

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void drawBackground(sf::RenderWindow& window);
    void drawPaths(sf::RenderWindow& window);

    // Obiekty gry aktualizowane i rysowane wspolnie
    std::vector<std::unique_ptr<GameObject>> m_objects;

    // Wskaznik do rdzenia serwera trzymanego w m_objects
    ServerCore* m_server = nullptr;

    int   m_serverHealth = 20;
    int   m_serverMaxHealth = 20;
    float m_time = 0.f; // czas uzywany do animacji

    float m_spawnTimer = 0.f;                               // Licznik czasu do spawnowania kolejnych wrogów
    float m_nextSpawnDelay = 1.0f;                          // Losowy czas do następnego wroga
    std::vector<std::unique_ptr<Path>> m_paths;        // Wektor przechowujący ścieżki na mapie
    LevelMap m_levelMap;                                // Przechowuje aktualny układ poziomów (pozycje, tory)
};