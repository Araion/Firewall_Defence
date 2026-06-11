#include "entities/SlowEffect.h"
#include "util/Theme.h"

SlowEffect::SlowEffect(sf::Vector2f pos, float radius, float life)
    : Effect(pos, life), m_radius(radius) {}

void SlowEffect::update(float dt) {
    advance(dt);
}

void SlowEffect::draw(sf::RenderWindow& window) {
    float progressValue = progress();

    // Rysuje zanikajacy pierscien pola spowolnienia
    float radius = m_radius * (0.4f + 0.6f * progressValue);

    sf::CircleShape ring(radius);
    ring.setOrigin(radius, radius);
    ring.setPosition(m_position);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(3.f);

    sf::Color color = Theme::NeonBlue;
    color.a = static_cast<sf::Uint8>(180 * (1.f - progressValue));

    ring.setOutlineColor(color);
    window.draw(ring);
}