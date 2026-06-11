#include "entities/WormEnemy.h"
#include <string>
#include <algorithm>

static int   wormClamp(int g) { return g < 0 ? 0 : (g > 2 ? 2 : g); }
static float wormHp(int g)    { return g == 0 ? 60.f : (g == 1 ? 28.f : 14.f); }
static float wormSpeed(int g) { return g == 0 ? 75.f : (g == 1 ? 110.f : 140.f); }
static float wormRadius(int g){ return g == 0 ? 14.f : (g == 1 ? 10.f : 7.f); }

WormEnemy::WormEnemy(ResourceManager& res, const Path* path, int gen)
    : Enemy(res, "assets/textures/worm_" + std::to_string(wormClamp(gen) + 1) + ".png", path,
            wormHp(wormClamp(gen)),
            wormSpeed(wormClamp(gen))),
    m_gen(wormClamp(gen)) {
    m_bodyColor = sf::Color(80, 230, 120); // jaskrawa zielen
    m_bodyRadius = wormRadius(m_gen);
    m_reward = 15;
    m_points = 15;
}

void WormEnemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) { Enemy::drawBody(window); return; }

    sf::Color body = m_bodyColor;

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