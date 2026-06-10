#include "entities/GlitchDroneEnemy.h"
#include <cmath>

GlitchDroneEnemy::GlitchDroneEnemy(ResourceManager& res, const Path* path)
    : Enemy(res, "assets/textures/glitch.png", path, 45.f, 110.f) {
    m_bodyColor = sf::Color(0, 229, 255);
    m_bodyRadius = 14.f;
}

void GlitchDroneEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }

    float jitter = std::sin(m_animTime * 40.f) * 1.5f;
    sf::Color body = m_bodyColor;
    sf::Uint8 a = static_cast<sf::Uint8>(180 + 75 * std::sin(m_animTime * 30.f));

    sf::CircleShape tri(m_bodyRadius, 3);
    tri.setOrigin(m_bodyRadius, m_bodyRadius);
    tri.setPosition(m_position.x + jitter, m_position.y);
    tri.setRotation(m_animTime * 200.f);
    tri.setFillColor(sf::Color(body.r, body.g, body.b, a));
    tri.setOutlineThickness(1.5f);
    tri.setOutlineColor(sf::Color(200, 255, 255, a));
    window.draw(tri);
}