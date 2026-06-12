#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "util/MathUtils.h"
#include "util/ArtScale.h"
#include <cmath>

Enemy::Enemy(ResourceManager& res, const std::string& texturePath, const Path* path,
             float maxHp, float speed, int reward, int points)
    : m_path(path), m_maxHp(maxHp), m_hp(maxHp), m_speed(speed),
    m_reward(reward), m_points(points) {
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

void Enemy::takeDamage(float dmg) {
    if (dmg <= 0.f) return;
    m_hp -= dmg;

    m_hitFlash = 0.12f;

    if (m_hp <= 0.f) {
        m_hp = 0.f;
        kill(); // zginal od obrazen
    }
}

void Enemy::applySlow(float factor, float duration) {
    // Bierzemy silniejsze spowolnienie i odswiezamy czas trwania
    if (factor < m_slowFactor || m_slowTimer <= 0.f) m_slowFactor = factor;
    if (duration > m_slowTimer) m_slowTimer = duration;
}

void Enemy::applyDot(float dps, float duration) {
    // Bierzemy silniejszy DoT i odswiezamy czas trwania
    m_dotDps = std::max(m_dotDps, dps);
    m_dotTimer = std::max(m_dotTimer, duration);
}

bool Enemy::isTargetableBy(const std::string& towerType) const {
    if (m_encrypted && !m_detected) {
        bool heuristics = m_owner && m_owner->heuristics();

        if (!heuristics && (towerType == "AntivirusTower" || towerType == "LaserTower"))
            return false;
    }
    return true;
}

void Enemy::update(float dt) {
    m_animTime += dt;
    if (m_hitFlash > 0.f) m_hitFlash -= dt;

    // Spowolnienie - mija po czasie
    if (m_slowTimer > 0.f) {
        m_slowTimer -= dt;
        if (m_slowTimer <= 0.f) { m_slowTimer = 0.f; m_slowFactor = 1.f; }
    }

    // Obrazenia w czasie
    if (m_dotTimer > 0.f) {
        m_hp -= m_dotDps * dt;
        m_dotTimer -= dt;
        if (m_dotTimer <= 0.f) { m_dotTimer = 0.f; m_dotDps = 0.f; }
        if (m_hp <= 0.f) { m_hp = 0.f; kill(); return; } // zginal od DoT
    }

    // Prosty ruch po ścieżce
    m_distance += m_speed * m_slowFactor * dt;

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
    bool flash = m_hitFlash > 0.f;
    if (m_hasTexture) {
        auto ts = m_texture->getSize();
        float sc = ts.x > 0 ? (m_bodyRadius * Art::kEnemySizeMult) / static_cast<float>(ts.x) : 1.f;
        m_sprite.setScale(sc, sc);
        m_sprite.setPosition(m_position);
        m_sprite.setRotation(0.f);
        m_sprite.setColor(flash ? sf::Color(255, 255, 255) : sf::Color::White);
        window.draw(m_sprite);
        return;
    }

    // Zastepczy wyglad
    sf::Color body = flash ? sf::Color::White : m_bodyColor;
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
    core.setOutlineColor(flash ? sf::Color::White
                               : sf::Color(body.r / 2, body.g / 2, body.b / 2));
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
    // Aura spowolnienia
    if (m_slowTimer > 0.f) {
        sf::CircleShape aura(m_bodyRadius + 6.f);
        aura.setOrigin(m_bodyRadius + 6.f, m_bodyRadius + 6.f);
        aura.setPosition(m_position);
        aura.setFillColor(sf::Color(80, 160, 255, 60));
        window.draw(aura);
    }
    // Aura "Corrupted" (DoT)
    if (m_dotTimer > 0.f) {
        for (int i = 0; i < 4; ++i) {
            float ang = m_animTime * 4.f + i * 1.57f;
            float rad = m_bodyRadius + 4.f + 2.f * std::sin(m_animTime * 9.f + i);
            sf::CircleShape p(2.5f);
            p.setOrigin(2.5f, 2.5f);
            p.setPosition(m_position.x + std::cos(ang) * rad,
                          m_position.y + std::sin(ang) * rad);
            p.setFillColor(sf::Color(150, 255, 60, 200));
            window.draw(p);
        }
    }

    drawBody(window);

    // Zaszyfrowany i niewykryty
    if (m_encrypted && !m_detected) {
        float r = m_bodyRadius + 5.f;
        sf::CircleShape lock(r, 6);
        lock.setOrigin(r, r);
        lock.setPosition(m_position);
        lock.setRotation(m_animTime * 30.f);
        lock.setFillColor(sf::Color(120, 120, 160, 50));
        lock.setOutlineThickness(2.f);
        lock.setOutlineColor(sf::Color(180, 180, 220, 200));
        window.draw(lock);
    }

    drawHealthBar(window);
}