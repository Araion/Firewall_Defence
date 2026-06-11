#pragma once
#include "entities/Effect.h"

// =============================================================
// BoostEffect - wizualizacja aury wzmocnienia szybkostrzelnosci
// =============================================================
class BoostEffect : public Effect {
public:
    BoostEffect(sf::Vector2f pos, float radius, float life = 0.6f);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    std::string getTypeName() const override { return "BoostEffect"; }

private:
    float m_radius;
};
