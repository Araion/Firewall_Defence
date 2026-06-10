#include "entities/TrojanEnemy.h"
#include <cmath>

TrojanEnemy::TrojanEnemy(ResourceManager& res, const Path* path)
    : Enemy(res, "assets/textures/trojan.png", path, 100.f, 50.f)
{
    m_bodyColor = sf::Color(255, 150, 40); // pomaranczowy
    m_bodyRadius = 18.f;
    m_serverDamage = 2; // wytrzymaly - po dotarciu zabiera serwerowi 2 HP
}

void TrojanEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }

    sf::Color body = m_bodyColor;

    // Szesciokatny kon trojanski
    sf::CircleShape hex(m_bodyRadius, 6);
    hex.setOrigin(m_bodyRadius, m_bodyRadius);
    hex.setPosition(m_position);
    hex.setRotation(m_facing);
    hex.setFillColor(sf::Color(body.r, body.g, body.b, 200));
    hex.setOutlineThickness(2.5f);
    hex.setOutlineColor(sf::Color(140, 80, 20));
    window.draw(hex);

    sf::CircleShape core(m_bodyRadius * 0.45f);
    core.setOrigin(m_bodyRadius * 0.45f, m_bodyRadius * 0.45f);
    core.setPosition(m_position);
    core.setFillColor(sf::Color(60, 30, 10));
    window.draw(core);
}