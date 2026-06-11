#pragma once
#include "entities/Effect.h"

// =============================================================
//  SlowEffect - wizualizacja pola spowolnienia Firewall
// =============================================================
class SlowEffect : public Effect {
public:
    SlowEffect(sf::Vector2f pos, float radius, float life = 0.7f);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    std::string getTypeName() const override { return "SlowEffect"; }

private:
    float m_radius;
};
