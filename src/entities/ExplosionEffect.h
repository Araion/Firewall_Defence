#pragma once
#include "entities/Effect.h"
#include <vector>

// =============================================================
// ExplosionEffect - prosty efekt eksplozji tworzony z czasteczek
// Uzywany po zniszczeniu przeciwnika albo przy trafieniu obszarowym
// =============================================================
class ExplosionEffect : public Effect {
public:
    ExplosionEffect(sf::Vector2f pos, sf::Color color, float scale = 1.f);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    std::string getTypeName() const override { return "ExplosionEffect"; }

private:
    // Pojedyncza czasteczka eksplozji
    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float size;
    };

    // Czasteczki rozlatujace sie od srodka eksplozji
    std::vector<Particle> m_particles;

    sf::Color m_color;
    float m_scale;
};