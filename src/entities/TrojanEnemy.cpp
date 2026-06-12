#include "entities/TrojanEnemy.h"
#include "entities/VirusEnemy.h"
#include "states/PlayState.h"
#include "util/Rng.h"
#include <cmath>

TrojanEnemy::TrojanEnemy(ResourceManager& res, const Path* path,
                        float hpScale, float speedScale, float rewardScale)
    : Enemy(res, "assets/textures/trojan.png", path,
            100.f * hpScale, 50.f * speedScale,
            static_cast<int>(25 * rewardScale), 25) {
    m_bodyColor = sf::Color(255, 150, 40); // pomaranczowy
    m_bodyRadius = 18.f;
    m_serverDamage = 2; // wytrzymaly - po dotarciu zabiera serwerowi 2 HP
}

void TrojanEnemy::onDeath(PlayState& state) {
    // Payload: rozpad na 2 wirusy w biezacym miejscu na tej samej sciezce.
    if (!state.enemyAbilities()) return;
    for (int i = 0; i < 2; ++i) {
        auto v = std::make_unique<VirusEnemy>(state.resources(), m_path);
        v->setDistance(std::max(0.f, m_distance - Rng::rangef(0.f, 18.f)));
        state.spawn(std::move(v));
    }
}

void TrojanEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }
    bool flash = m_hitFlash > 0.f;
    sf::Color body = flash ? sf::Color::White : m_bodyColor;

    // Szesciokatny "kon trojanski" - masywny ksztalt.
    sf::CircleShape hex(m_bodyRadius, 6);
    hex.setOrigin(m_bodyRadius, m_bodyRadius);
    hex.setPosition(m_position);
    hex.setRotation(m_facing);
    hex.setFillColor(sf::Color(body.r, body.g, body.b, 200));
    hex.setOutlineThickness(2.5f);
    hex.setOutlineColor(flash ? sf::Color::White : sf::Color(140, 80, 20));
    window.draw(hex);

    sf::CircleShape core(m_bodyRadius * 0.45f);
    core.setOrigin(m_bodyRadius * 0.45f, m_bodyRadius * 0.45f);
    core.setPosition(m_position);
    core.setFillColor(sf::Color(60, 30, 10));
    window.draw(core);
}