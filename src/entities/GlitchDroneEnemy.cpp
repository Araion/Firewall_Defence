#include "entities/GlitchDroneEnemy.h"
#include "entities/Tower.h"
#include "states/PlayState.h"
#include "util/MathUtils.h"
#include "util/Rng.h"
#include <cmath>

GlitchDroneEnemy::GlitchDroneEnemy(ResourceManager& res, const Path* path, PlayState& owner,
                                   float hpScale, float speedScale, float rewardScale)
    : Enemy(res, "assets/textures/glitch.png", path,
            45.f * hpScale, 110.f * speedScale,
            static_cast<int>(20 * rewardScale), 30) {
    setOwner(&owner);
    m_bodyColor = sf::Color(0, 229, 255); // cyan
    m_bodyRadius = 14.f;
}

void GlitchDroneEnemy::update(float dt) {
    Enemy::update(dt);
    if (!isAlive()) return;
    if (!m_owner || !m_owner->enemyAbilities()) return;

    m_empTimer -= dt;
    if (m_empTimer <= 0.f) {
        m_empTimer = 4.f;
        // Wylacz najblizsza wieze
        Tower* nearest = nullptr;
        float best = 1e9f;
        for (Tower* t : m_owner->towers()) {
            float d = MathUtils::distance(m_position, t->getPosition());
            if (d < best) { best = d; nearest = t; }
        }
        if (nearest) nearest->disableFor(2.0f);
    }
}

void GlitchDroneEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }
    bool flash = m_hitFlash > 0.f;
    // Migotliwy, trojkat
    float jitter = std::sin(m_animTime * 40.f) * 1.5f;
    sf::Color body = flash ? sf::Color::White : m_bodyColor;
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