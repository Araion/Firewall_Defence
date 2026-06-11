#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
//  FirewallTower - wieza spowalniajaca
// =============================================================
class FirewallTower : public Tower {
public:
    FirewallTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "FirewallTower"; }

    // Aktualizuje dzialanie pola zamiast standardowego celowania
    void update(float dt) override;

    // Wieza nie strzela pociskami, bo jej efekt dziala stale
    void attack() override {}

    // Rysuje pole spowolnienia oraz podstawe wiezy
    void drawBaseLayer(sf::RenderWindow& window) override;

    // Zwraca opis aktualnego spowolnienia do panelu informacji
    std::string statLine() const override;

private:
    // Oblicza aktualny ulamek spowolnienia na podstawie poziomu wiezy
    float slowFraction() const;

    // Odlicza czas do kolejnej emisji efektu wizualnego
    float m_pulseTimer = 0.f;
};