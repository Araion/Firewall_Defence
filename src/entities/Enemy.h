#pragma once
#include "core/GameObject.h"
#include "util/Path.h"
#include "util/Theme.h"
#include <memory>
#include <string>

class ResourceManager;
class PlayState;

class Enemy : public GameObject {
public:
    Enemy(ResourceManager& res, const std::string& texturePath, const Path* path,
          float maxHp, float speed);
    virtual ~Enemy() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    // Obrazenia od pociskow/lasera (zabicie -> nagroda + wynik w PlayState)
    void takeDamage(float dmg);
    // Spowolnienie: factor < 1 (np. 0.65 = -35%) na czas 'duration'
    void applySlow(float factor, float duration);

    bool  reachedServer() const { return m_reachedServer; }
    int   getServerDamage() const { return m_serverDamage; }
    int   getReward() const { return m_reward; }
    int   getPoints() const { return m_points; }
    float getHp() const { return m_hp; }
    float getMaxHp() const { return m_maxHp; }
    const Path* getPath() const { return m_path; }
    float getDistance() const { return m_distance; }
    float getRadius() const { return m_bodyRadius; }
    sf::Color getBodyColor() const { return m_bodyColor; }

protected:
    const Path* m_path = nullptr;
    float m_distance = 0.f;
    float m_maxHp;
    float m_hp;
    float m_speed;
    int   m_serverDamage = 1;
    int   m_reward = 0;       // kredyty za zabicie przez wieze
    int   m_points = 0;       // punkty wyniku za zabicie

    float m_slowFactor = 1.f; // mnoznik predkosci (1 = pelna)
    float m_slowTimer = 0.f;  // pozostaly czas spowolnienia (s)
    float m_animTime = 0.f;
    float m_facing = 0.f;

    std::shared_ptr<sf::Texture> m_texture;
    sf::Sprite m_sprite;
    bool m_hasTexture = false;
    sf::Color m_bodyColor{Theme::NeonMagenta};
    float m_bodyRadius = 14.f;

    bool m_reachedServer = false;
    PlayState* m_owner = nullptr;

    virtual void drawBody(sf::RenderWindow& window);
    void drawHealthBar(sf::RenderWindow& window);
};