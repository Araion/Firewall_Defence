#include "entities/ProxyEnemy.h"
#include <cmath>

ProxyEnemy::ProxyEnemy(ResourceManager& res, const Path* path)
    : Enemy(res, "assets/textures/proxy.png", path, 60.f, 70.f) {
    m_bodyColor = sf::Color(170, 130, 255);
    m_bodyRadius = 15.f;
}

void ProxyEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }

    sf::Color body = m_bodyColor;

    sf::CircleShape outer(m_bodyRadius, 4);
    outer.setOrigin(m_bodyRadius, m_bodyRadius);
    outer.setPosition(m_position);
    outer.setRotation(m_animTime * 60.f);
    outer.setFillColor(sf::Color(body.r, body.g, body.b, 90));
    outer.setOutlineThickness(2.f);
    outer.setOutlineColor(body);
    window.draw(outer);

    float pr = m_bodyRadius * 0.55f * (1.f + 0.12f * std::sin(m_animTime * 7.f));
    sf::CircleShape core(pr, 6);
    core.setOrigin(pr, pr);
    core.setPosition(m_position);
    core.setRotation(-m_animTime * 120.f);
    core.setFillColor(sf::Color(220, 200, 255));
    window.draw(core);
}