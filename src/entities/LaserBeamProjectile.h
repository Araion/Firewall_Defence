#pragma once
#include "entities/Projectile.h"

// =============================================================
// LaserBeamProjectile - krotkotrwaly efekt promienia lasera
// Sam nie trafia przeciwnikow, bo obrazenia nalicza bezposrednio LaserTower
// =============================================================
class LaserBeamProjectile : public Projectile {
public:
    LaserBeamProjectile(sf::Vector2f start, sf::Vector2f end, sf::Color color,
                        float width = 4.f, float life = 0.18f);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    bool canHit(const Enemy&) const override { return false; }
    std::string getTypeName() const override { return "LaserBeamProjectile"; }

private:
    // Punkty poczatku i konca promienia
    sf::Vector2f m_start;
    sf::Vector2f m_end;

    // Wyglad promienia
    sf::Color m_color;
    float m_width;

    // Czas zycia efektu
    float m_life;
    float m_maxLife;
};
