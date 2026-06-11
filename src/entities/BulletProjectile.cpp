#include "entities/BulletProjectile.h"
#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "util/MathUtils.h"
#include "util/ArtScale.h"

BulletProjectile::BulletProjectile(ResourceManager& res, sf::Vector2f pos, sf::Vector2f dir,
                                   float speed, float damage, float maxDistance, sf::Color color,
                                   const std::string& texturePath)
    : Projectile(damage), m_speed(speed), m_maxDistance(maxDistance), m_color(color) {
    m_position = pos;

    sf::Vector2f direction = MathUtils::normalize(dir);
    m_velocity = direction * speed;

    if (!texturePath.empty()) {
        m_texture = res.getTexture(texturePath);

        if (m_texture) {
            m_hasTexture = true;
            m_sprite.setTexture(*m_texture);

            auto textureSize = m_texture->getSize();

            m_sprite.setOrigin(textureSize.x / 2.f, textureSize.y / 2.f);
            m_sprite.setRotation(MathUtils::angleDeg(direction));
        }
    }
}

void BulletProjectile::update(float dt) {
    // Zapisuje ostatnie pozycje pocisku do rysowania ogona
    m_trail.push_front(m_position);

    if (m_trail.size() > 6)
        m_trail.pop_back();

    m_position += m_velocity * dt;
    m_traveled += m_speed * dt;

    if (m_traveled >= m_maxDistance)
        kill();
}

sf::FloatRect BulletProjectile::getBounds() const {
    return sf::FloatRect(
        m_position.x - m_radius,
        m_position.y - m_radius,
        m_radius * 2.f,
        m_radius * 2.f
        );
}

void BulletProjectile::onHit(Enemy& enemy, PlayState& state) {
    if (m_splashRadius > 0.f) {
        // Zadaje obrazenia wszystkim przeciwnikom w zasiegu wybuchu
        for (Enemy* e : state.enemies()) {
            if (MathUtils::distance(m_position, e->getPosition()) <= m_splashRadius) {
                e->takeDamage(m_damage);
                if (m_dotDps > 0.f) e->applyDot(m_dotDps, m_dotDuration);
            }
        }

        state.spawnExplosion(m_position, m_color, m_splashRadius / 28.f);
    } else {
        enemy.takeDamage(m_damage);
        if (m_dotDps > 0.f) enemy.applyDot(m_dotDps, m_dotDuration); // DoT
    }

    kill();
}

void BulletProjectile::draw(sf::RenderWindow& window) {
    if (!m_hasTexture)
        return;

    // Rysuje zanikajacy ogon za pociskiem
    for (size_t i = 0; i < m_trail.size(); ++i) {
        float t = 1.f - static_cast<float>(i) / (m_trail.size() + 1);
        float radius = m_radius * t;

        sf::CircleShape trail(radius);
        trail.setOrigin(radius, radius);
        trail.setPosition(m_trail[i]);
        trail.setFillColor(sf::Color(
            m_color.r,
            m_color.g,
            m_color.b,
            static_cast<sf::Uint8>(120 * t)
            ));

        window.draw(trail);
    }

    auto textureSize = m_texture->getSize();
    float targetLength = m_splashRadius > 0.f ? Art::kShotLengthSplash : Art::kShotLength;
    float scale = textureSize.x > 0 ? targetLength / static_cast<float>(textureSize.x) : 1.f;

    m_sprite.setScale(scale, scale);
    m_sprite.setPosition(m_position);
    window.draw(m_sprite);
}