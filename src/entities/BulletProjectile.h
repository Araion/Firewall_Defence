#pragma once
#include "entities/Projectile.h"
#include <memory>
#include <deque>
#include <string>

class ResourceManager;

// =============================================================
// BulletProjectile - pocisk lecacy po prostej w zadanym kierunku
// Moze zadawac obrazenia pojedynczemu celowi albo obszarowe
// =============================================================
class BulletProjectile : public Projectile {
public:
    BulletProjectile(ResourceManager& res, sf::Vector2f pos, sf::Vector2f dir,
                     float speed, float damage, float maxDistance, sf::Color color,
                     const std::string& texturePath = "");

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    void onHit(Enemy& enemy, PlayState& state) override;
    std::string getTypeName() const override { return "BulletProjectile"; }

    // Ustawia promien obrazen obszarowych
    // Wartosc 0 oznacza obrazenia tylko w trafiony cel
    void setSplashRadius(float r) { m_splashRadius = r; }

private:
    // Obrazenia obszarowe pocisku
    float m_splashRadius = 0.f;

    // Ruch i zasieg pocisku
    sf::Vector2f m_velocity;
    float m_speed;
    float m_maxDistance;
    float m_traveled = 0.f;

    // Wyglad pocisku
    float m_radius = 5.f;
    sf::Color m_color;

    std::shared_ptr<sf::Texture> m_texture;
    sf::Sprite m_sprite;
    bool m_hasTexture = false;

    // Ostatnie pozycje pocisku uzywane do rysowania ogona
    std::deque<sf::Vector2f> m_trail;
};
