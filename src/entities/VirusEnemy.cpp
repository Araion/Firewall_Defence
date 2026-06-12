#include "entities/VirusEnemy.h"
#include <cmath>

VirusEnemy::VirusEnemy(ResourceManager& res, const Path* path,
                       float hpScale, float speedScale, float rewardScale)
    : Enemy(res, "assets/textures/virus.png", path,
            35.f * hpScale, 90.f * speedScale,
            static_cast<int>(10 * rewardScale), 10) {
    m_bodyColor = sf::Color(255, 80, 160); // jaskrawy roz/magenta
    m_bodyRadius = 13.f;
}

void VirusEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }

    bool flash = m_hitFlash > 0.f;
    sf::Color body = flash ? sf::Color::White : m_bodyColor;

    // "Glitchowy" wirus: obracajacy sie kwadrat z migajacym rdzeniem.
    float r = m_bodyRadius * (1.f + 0.08f * std::sin(m_animTime * 10.f));
    sf::RectangleShape sq({r * 1.6f, r * 1.6f});
    sq.setOrigin(r * 0.8f, r * 0.8f);
    sq.setPosition(m_position);
    sq.setRotation(m_animTime * 120.f); // szybki obrot
    sq.setFillColor(sf::Color(body.r, body.g, body.b, 90));
    sq.setOutlineThickness(2.f);
    sq.setOutlineColor(body);
    window.draw(sq);

    sf::CircleShape core(r * 0.45f);
    core.setOrigin(r * 0.45f, r * 0.45f);
    core.setPosition(m_position);
    core.setFillColor(body);
    window.draw(core);
}