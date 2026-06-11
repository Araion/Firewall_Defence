#include "entities/Tower.h"
#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "util/MathUtils.h"
#include "util/Theme.h"
#include "util/Rng.h"
#include <cmath>
#include <cctype>
#include <algorithm>

Tower::Tower(PlayState& state, ResourceManager& res)
    : m_state(state), m_res(res) {
    m_barrelAngle = Rng::rangef(0.f, 360.f);
}

void Tower::applyLevelStats() {
    // Z kazdym poziomem rosna obrazenia i zasieg wiezy
    m_damage = m_baseDamage;
    m_range = m_baseRange;

    for (int i = 1; i < m_level; ++i) {
        m_damage *= 1.35f;
        m_range *= 1.10f;
    }
}

void Tower::finalizeSetup() {
    applyLevelStats();

    m_invested = m_cost;

    // Tworzy klucz grafiki na podstawie nazwy typu wiezy
    std::string key = getTypeName();

    for (char& c : key)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    size_t towerPos = key.find("tower");

    if (towerPos != std::string::npos)
        key.erase(towerPos);

    loadTowerArt(key);
}

void Tower::loadTowerArt(const std::string& key) {
    if (key.empty())
        return;

    m_headTex = m_res.getTexture("assets/textures/" + key + "_head.png");
    m_baseTex = m_res.getTexture("assets/textures/" + key + "_base.png");

    if (!m_baseTex || !m_headTex) {
        m_hasArt = false;
        return;
    }

    m_hasArt = true;

    auto baseSize = m_baseTex->getSize();

    m_artScale = baseSize.x > 0 ? m_baseSizePx / static_cast<float>(baseSize.x) : 1.f;
    m_headScale = m_artScale * m_headScaleMult;

    m_baseSprite.setTexture(*m_baseTex);
    m_baseSprite.setOrigin(baseSize.x / 2.f, baseSize.y / 2.f);
    m_baseSprite.setScale(m_artScale, m_artScale);

    auto headSize = m_headTex->getSize();

    m_headSprite.setTexture(*m_headTex);
    m_headSprite.setOrigin(headSize.x * m_headPivot.x, headSize.y * m_headPivot.y);
    m_headSprite.setScale(m_headScale, m_headScale);
}

void Tower::upgrade() {
    if (!canUpgrade())
        return;

    m_invested += getUpgradeCost();
    ++m_level;

    applyLevelStats();
}

int Tower::getUpgradeCost() const {
    return static_cast<int>(m_cost * (0.5f + 0.4f * m_level));
}

int Tower::getSellValue() const {
    return static_cast<int>(0.7f * m_invested);
}

Enemy* Tower::acquireTarget() const {
    Enemy* best = nullptr;
    float bestProgress = -1.f;
    float range = effectiveRange();

    // Wybiera przeciwnika najdalej przesunietego po sciezce
    for (Enemy* enemy : m_state.enemies()) {
        float distance = MathUtils::distance(m_position, enemy->getPosition());

        if (distance > range)
            continue;

        if (enemy->getDistance() > bestProgress) {
            bestProgress = enemy->getDistance();
            best = enemy;
        }
    }

    return best;
}

void Tower::rotateBarrelTowards(Enemy* target, float dt) {
    if (!target)
        return;

    float desiredAngle = MathUtils::angleDeg(target->getPosition() - m_position);

    m_barrelAngle = MathUtils::rotateTowards(
        m_barrelAngle,
        desiredAngle,
        m_rotationSpeed * dt
        );
}

sf::Vector2f Tower::barrelTip(float length) const {
    float radians = m_barrelAngle * MathUtils::PI / 180.f;

    return m_position + m_headOffset +
           sf::Vector2f(std::cos(radians), std::sin(radians)) * length;
}

sf::Vector2f Tower::headPoint(float along, float perp) const {
    float radians = m_barrelAngle * MathUtils::PI / 180.f;
    float cosAngle = std::cos(radians);
    float sinAngle = std::sin(radians);

    // Dopasowuje punkt glowki przy odbiciu sprite'a w lewa strone
    if (cosAngle < 0.f)
        perp = -perp;

    return m_position + m_headOffset +
           sf::Vector2f(
               along * cosAngle - perp * sinAngle,
               along * sinAngle + perp * cosAngle
               );
}

void Tower::update(float dt) {
    m_animTime += dt;

    if (m_cooldownTimer > 0.f)
        m_cooldownTimer -= dt;

    m_target = acquireTarget();

    if (m_headMotion == HeadMotion::Aim && m_target)
        rotateBarrelTowards(m_target, dt);

    if (m_canShoot && m_target && m_cooldownTimer <= 0.f) {
        attack();
        m_cooldownTimer = m_fireCooldown;
    }
}

sf::FloatRect Tower::getBounds() const {
    return sf::FloatRect(m_position.x - 24.f, m_position.y - 24.f, 48.f, 48.f);
}

void Tower::drawRangeCircle(sf::RenderWindow& window) const {
    float radius = effectiveRange();

    sf::CircleShape range(radius);
    range.setOrigin(radius, radius);
    range.setPosition(m_position);
    range.setFillColor(sf::Color(m_color.r, m_color.g, m_color.b, 26));
    range.setOutlineThickness(2.f);
    range.setOutlineColor(sf::Color(m_color.r, m_color.g, m_color.b, 140));

    window.draw(range);
}

void Tower::draw(sf::RenderWindow& window) {
    drawBaseLayer(window);
    drawHeadLayer(window);
}

void Tower::drawBaseLayer(sf::RenderWindow& window) {
    if (m_selected)
        drawRangeCircle(window);

    if (!m_hasArt)
        return;

    // Rysuje poswiate pod wieza
    sf::CircleShape halo(26.f);
    halo.setOrigin(26.f, 26.f);
    halo.setPosition(m_position);
    halo.setFillColor(sf::Color(m_color.r, m_color.g, m_color.b, 50));

    window.draw(halo);

    m_baseSprite.setPosition(m_position);
    window.draw(m_baseSprite);
}

void Tower::drawHeadLayer(sf::RenderWindow& window) {
    if (m_hasArt) {
        if (m_headMotion == HeadMotion::Bob) {
            float bob = std::sin(m_animTime * 3.f) * m_bobAmp;

            m_headSprite.setScale(m_headScale, m_headScale);
            m_headSprite.setRotation(0.f);
            m_headSprite.setPosition(
                m_position.x + m_headOffset.x,
                m_position.y + m_headOffset.y + bob
                );
        } else {
            float radians = m_barrelAngle * MathUtils::PI / 180.f;
            bool left = std::cos(radians) < 0.f;

            m_headSprite.setScale(m_headScale, left ? -m_headScale : m_headScale);
            m_headSprite.setRotation(m_barrelAngle);
            m_headSprite.setPosition(m_position + m_headOffset);
        }

        window.draw(m_headSprite);
    }

    // Rysuje wskazniki poziomu wiezy
    for (int i = 0; i < m_level; ++i) {
        sf::CircleShape pip(2.5f);
        pip.setOrigin(2.5f, 2.5f);
        pip.setPosition(m_position.x - 8.f + i * 8.f, m_position.y + 32.f);
        pip.setFillColor(Theme::NeonGreen);

        window.draw(pip);
    }
}