#pragma once
#include "core/GameObject.h"

class Enemy;
class PlayState;

// =============================================================
// Projectile - abstrakcyjna baza pociskow
// Przechowuje obrazenia i okresla reakcje pocisku na trafienie przeciwnika
// =============================================================
class Projectile : public GameObject {
public:
    explicit Projectile(float damage) : m_damage(damage) {}
    virtual ~Projectile() = default;

    float getDamage() const { return m_damage; }

    // Reakcja pocisku po trafieniu przeciwnika
    // Domyslnie zadaje obrazenia i usuwa pocisk
    virtual void onHit(Enemy& enemy, PlayState& state);

    // Okresla, czy pocisk moze trafic danego przeciwnika
    virtual bool canHit(const Enemy&) const { return true; }

    std::string getTypeName() const override { return "Projectile"; }

protected:
    // Obrazenia zadawane przez pocisk
    float m_damage;
};
