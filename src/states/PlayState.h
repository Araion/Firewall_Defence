#pragma once
#include "core/GameState.h"
#include "core/GameObject.h"
#include <vector>
#include <memory>

class ServerCore;

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

    // Obiekty gry aktualizowane i rysowane wspolnie
    std::vector<std::unique_ptr<GameObject>> m_objects;

    // Wskaznik do rdzenia serwera trzymanego w m_objects
    ServerCore* m_server = nullptr;

    int   m_serverHealth = 20;
    int   m_serverMaxHealth = 20;
    float m_time = 0.f; // czas uzywany do animacji
};
