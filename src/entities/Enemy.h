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
          float maxHp, float speed, int reward = 10, int points = 10);
    virtual ~Enemy() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    virtual void takeDamage(float dmg);
    virtual void applySlow(float factor, float duration);
    void applyDot(float dps, float duration);
    bool isCorrupted() const { return m_dotTimer > 0.f; }
    virtual void onDeath(PlayState& state) { (void)state; }

    bool  reachedServer() const { return m_reachedServer; }
    int   getReward() const { return m_reward; }
    int   getPoints() const { return m_points; }
    int   getServerDamage() const { return m_serverDamage; }
    float getHp() const { return m_hp; }
    float getMaxHp() const { return m_maxHp; }
    const Path* getPath() const { return m_path; }
    float getDistance() const { return m_distance; }
    float getRadius() const { return m_bodyRadius; }
    sf::Color getBodyColor() const { return m_bodyColor; }
    bool  isSlowed() const { return m_slowTimer > 0.f; }

    void setDistance(float d) { m_distance = d; }
    void setHp(float hp) { m_hp = hp; }

    void teleportForward(float d) {
        if (!m_path) return;
        float maxD = m_path->totalLength() * 0.92f;
        float nd = m_distance + d;
        m_distance = (nd < maxD) ? nd : maxD;
    }

    void setOwner(PlayState* owner) { m_owner = owner; }

    virtual bool isTargetableBy(const std::string& towerType) const;

    void setEncrypted(bool e) { m_encrypted = e; }
    bool isEncrypted() const { return m_encrypted; }
    bool isDetected() const { return m_detected; }
    void markDetected() { m_detected = true; }

protected:
    const Path* m_path = nullptr;
    float m_distance = 0.f;
    float m_maxHp;
    float m_hp;
    float m_speed;
    int   m_reward;
    int   m_points;
    int   m_serverDamage = 1;

    float m_slowFactor = 1.f;
    float m_slowTimer = 0.f;
    float m_dotDps = 0.f;
    float m_dotTimer = 0.f;
    float m_hitFlash = 0.f;
    float m_animTime = 0.f;
    float m_facing = 0.f;

    std::shared_ptr<sf::Texture> m_texture;
    sf::Sprite m_sprite;
    bool m_hasTexture = false;
    sf::Color m_bodyColor{Theme::NeonMagenta};
    float m_bodyRadius = 14.f;

    bool m_reachedServer = false;
    bool m_encrypted = false;
    bool m_detected = false;
    PlayState* m_owner = nullptr;

    virtual void drawBody(sf::RenderWindow& window);
    void drawHealthBar(sf::RenderWindow& window);
};