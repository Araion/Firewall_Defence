#include "entities/WormEnemy.h"
#include "states/PlayState.h"
#include "util/Rng.h"
#include "util/MathUtils.h"
#include <cmath>
#include <string>
#include <algorithm>

static int   wormClamp(int g) { return g < 0 ? 0 : (g > 2 ? 2 : g); }
static float wormHp(int g)    { return g == 0 ? 60.f : (g == 1 ? 28.f : 14.f); }
static float wormSpeed(int g) { return g == 0 ? 75.f : (g == 1 ? 110.f : 140.f); }
static int   wormReward(int g){ return g == 0 ? 15 : (g == 1 ? 6 : 3); }
static float wormRadius(int g){ return g == 0 ? 14.f : (g == 1 ? 10.f : 7.f); }

WormEnemy::WormEnemy(ResourceManager& res, const Path* path, int gen,
                     float hpScale, float speedScale, float rewardScale)
    : Enemy(res, "assets/textures/worm_" + std::to_string(wormClamp(gen) + 1) + ".png", path,
            wormHp(wormClamp(gen)) * hpScale,
            wormSpeed(wormClamp(gen)) * speedScale,
            static_cast<int>(wormReward(wormClamp(gen)) * rewardScale),
            wormReward(wormClamp(gen))),
    m_gen(wormClamp(gen)) {
    m_bodyColor = sf::Color(80, 230, 120); // jaskrawa zielen
    m_bodyRadius = wormRadius(m_gen);
}

void WormEnemy::onDeath(PlayState& state) {
    // Replikacja: worm rozpada sie na 2 wormy nastepnej fazy (faza 2 juz nie).
    if (m_gen >= 2 || !state.enemyAbilities()) return;
    for (int i = 0; i < 2; ++i) {
        auto w = std::make_unique<WormEnemy>(state.resources(), m_path, m_gen + 1);
        w->setDistance(std::max(0.f, m_distance - Rng::rangef(0.f, 14.f)));
        state.spawn(std::move(w));
    }
}

void WormEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }
    bool flash = m_hitFlash > 0.f;
    sf::Color body = flash ? sf::Color::White : m_bodyColor;

    // Segmentowane cialo - kilka okregow za glowa (wzdluz sciezki).
    int segs = 4;
    for (int i = segs; i >= 0; --i) {
        float back = static_cast<float>(i) * m_bodyRadius * 0.9f;
        sf::Vector2f p = m_path ? m_path->positionAt(std::max(0.f, m_distance - back))
                                : m_position;
        float s = m_bodyRadius * (1.f - 0.12f * i);
        sf::CircleShape seg(s);
        seg.setOrigin(s, s);
        seg.setPosition(p);
        sf::Uint8 a = static_cast<sf::Uint8>(255 - i * 35);
        seg.setFillColor(sf::Color(body.r, body.g, body.b, a));
        window.draw(seg);
    }
}