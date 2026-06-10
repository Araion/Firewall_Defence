#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "util/MathUtils.h"
#include "util/ArtScale.h"
#include <cmath>

Enemy::Enemy(ResourceManager& res, const std::string& texturePath, const Path* path,
             float maxHp, float speed)
    : m_path(path), m_maxHp(maxHp), m_hp(maxHp), m_speed(speed) {
    if (path && !path->empty()) m_position = path->start();

    if (!texturePath.empty()) {
        m_texture = res.getTexture(texturePath);
        if (m_texture) {
            m_hasTexture = true;
            m_sprite.setTexture(*m_texture);
            auto ts = m_texture->getSize();
            m_sprite.setOrigin(ts.x / 2.f, ts.y / 2.f);
        }
    }
}

void Enemy::update(float dt) {
    m_animTime += dt;

    // Prosty ruch po ścieżce
    m_distance += m_speed * dt;

    if (m_path && !m_path->empty()) {
        m_position = m_path->positionAt(m_distance);
        m_facing = m_path->angleAt(m_distance);

        if (m_path->isFinished(m_distance)) {
            m_reachedServer = true;
            kill(); // Znika po dotarciu
        }
    }
}

sf::FloatRect Enemy::getBounds() const {
    return sf::FloatRect(m_position.x - m_bodyRadius, m_position.y - m_bodyRadius,
                         m_bodyRadius * 2.f, m_bodyRadius * 2.f);
}

void Enemy::drawBody(sf::RenderWindow& window) {
    if (m_hasTexture) {
        auto ts = m_texture->getSize();
        float sc = ts.x > 0 ? (m_bodyRadius * Art::kEnemySizeMult) / static_cast<float>(ts.x) : 1.f;
        m_sprite.setScale(sc, sc);
        m_sprite.setPosition(m_position);
        m_sprite.setRotation(0.f);
        m_sprite.setColor(sf::Color::White);
        window.draw(m_sprite);
        return;
    }

    // Zastepczy wyglad: pulsujace cialo + rdzen
    float pulse = m_bodyRadius * (1.f + 0.06f * std::sin(m_animTime * 8.f));

    sf::CircleShape outer(pulse);
    outer.setOrigin(pulse, pulse);
    outer.setPosition(m_position);
    outer.setFillColor(sf::Color(m_bodyColor.r, m_bodyColor.g, m_bodyColor.b, 70));
    window.draw(outer);

    sf::CircleShape core(m_bodyRadius * 0.7f);
    core.setOrigin(m_bodyRadius * 0.7f, m_bodyRadius * 0.7f);
    core.setPosition(m_position);
    core.setFillColor(m_bodyColor);
    core.setOutlineThickness(2.f);
    core.setOutlineColor(sf::Color(m_bodyColor.r / 2, m_bodyColor.g / 2, m_bodyColor.b / 2));
    window.draw(core);
}

void Enemy::drawHealthBar(sf::RenderWindow& window) {
    if (m_hp >= m_maxHp) return;
    const float w = m_bodyRadius * 2.2f, h = 4.f;
    float frac = m_maxHp > 0 ? m_hp / m_maxHp : 0.f;
    float y = m_position.y - m_bodyRadius - 9.f;

    sf::RectangleShape bg({w, h});
    bg.setOrigin(w / 2.f, h / 2.f);
    bg.setPosition(m_position.x, y);
    bg.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(bg);

    sf::RectangleShape fill({w * frac, h});
    fill.setOrigin(w / 2.f, h / 2.f);
    fill.setPosition(m_position.x - w / 2.f + (w * frac) / 2.f, y);
    sf::Color c = frac > 0.5f ? Theme::Ok : (frac > 0.25f ? Theme::Warn : Theme::Danger);
    fill.setFillColor(c);
    window.draw(fill);
}

void Enemy::draw(sf::RenderWindow& window) {
    drawBody(window);
    drawHealthBar(window);
}