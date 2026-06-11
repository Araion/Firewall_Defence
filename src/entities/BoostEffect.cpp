#include "entities/BoostEffect.h"
#include "util/Theme.h"

BoostEffect::BoostEffect(sf::Vector2f pos, float radius, float life)
    : Effect(pos, life), m_radius(radius) {}

void BoostEffect::update(float dt) {
    advance(dt);
}

void BoostEffect::draw(sf::RenderWindow& window) {
    float t = progress();
    // Pierscien kurczy sie do srodka i zanika
    float r = m_radius * (1.0f - 0.6f * t);
    sf::CircleShape ring(r);
    ring.setOrigin(r, r);
    ring.setPosition(m_position);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(2.5f);
    sf::Color c = Theme::Warn;
    c.a = static_cast<sf::Uint8>(170 * (1.f - t));
    ring.setOutlineColor(c);
    window.draw(ring);
}
