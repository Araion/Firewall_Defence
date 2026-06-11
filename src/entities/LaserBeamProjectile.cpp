#include "entities/LaserBeamProjectile.h"
#include "util/MathUtils.h"

LaserBeamProjectile::LaserBeamProjectile(sf::Vector2f start, sf::Vector2f end, sf::Color color,
                                         float width, float life)
    : Projectile(0.f), m_start(start), m_end(end), m_color(color),
      m_width(width), m_life(life), m_maxLife(life) {
    m_position = start;
}

void LaserBeamProjectile::update(float dt) {
    m_life -= dt;
    if (m_life <= 0.f) kill();
}

sf::FloatRect LaserBeamProjectile::getBounds() const {
    return sf::FloatRect(m_position.x, m_position.y, 0.f, 0.f);
}

void LaserBeamProjectile::draw(sf::RenderWindow& window) {
    float t = m_maxLife > 0.f ? m_life / m_maxLife : 0.f; // 1 -> 0
    sf::Vector2f d = m_end - m_start;
    float len = MathUtils::length(d);
    float angle = MathUtils::angleDeg(d);

    // Rysuje jedna warstwe promienia
    auto drawLine = [&](float width, sf::Uint8 alpha) {
        sf::RectangleShape beam({len, width});
        beam.setOrigin(0.f, width / 2.f);
        beam.setPosition(m_start);
        beam.setRotation(angle);
        beam.setFillColor(sf::Color(m_color.r, m_color.g, m_color.b, alpha));
        window.draw(beam);
    };

    // Promien sklada sie z poswiaty, rdzenia i jasnego srodka
    drawLine(m_width * 3.f, static_cast<sf::Uint8>(60 * t));
    drawLine(m_width, static_cast<sf::Uint8>(220 * t));
    drawLine(m_width * 0.4f, static_cast<sf::Uint8>(255 * t));

    float r = m_width * 1.6f;
    sf::CircleShape hit(r);
    hit.setOrigin(r, r);
    hit.setPosition(m_end);
    hit.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(200 * t)));
    window.draw(hit);
}
