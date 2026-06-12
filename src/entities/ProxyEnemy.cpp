#include "entities/ProxyEnemy.h"
#include "entities/ExplosionEffect.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "util/MathUtils.h"
#include <cmath>

ProxyEnemy::ProxyEnemy(ResourceManager& res, const Path* path, PlayState& owner,
                       float hpScale, float speedScale, float rewardScale)
    : Enemy(res, "assets/textures/proxy.png", path,
            60.f * hpScale, 70.f * speedScale,
            static_cast<int>(24 * rewardScale), 35) {
    setOwner(&owner);
    m_bodyColor = sf::Color(170, 130, 255); // fiolet
    m_bodyRadius = 15.f;
}

void ProxyEnemy::update(float dt) {
    Enemy::update(dt);
    if (!isAlive()) return;
    if (!m_owner || !m_owner->enemyAbilities()) return;

    m_warpTimer -= dt;
    if (m_warpTimer > 0.f) return;
    m_warpTimer = 3.5f;

    // "Port Forward": przerzuca do przodu po sciezce do 3 poblizu wrogow.
    int warped = 0;
    for (Enemy* e : m_owner->enemies()) {
        if (e == this || warped >= 3) continue;
        if (!e->getPath()) continue;
        if (MathUtils::distance(m_position, e->getPosition()) > m_warpRadius) continue;
        sf::Vector2f from = e->getPosition();
        e->teleportForward(m_warpJump);
        m_owner->spawnExplosion(from, m_bodyColor, 0.6f);
        m_owner->spawnExplosion(e->getPosition(), m_bodyColor, 0.6f);
        ++warped;
    }
    if (warped > 0) m_owner->spawnExplosion(m_position, sf::Color(210, 180, 255), 0.5f);
}

void ProxyEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }
    bool flash = m_hitFlash > 0.f;
    sf::Color body = flash ? sf::Color::White : m_bodyColor;

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
    core.setFillColor(flash ? sf::Color::White : sf::Color(220, 200, 255));
    window.draw(core);
}