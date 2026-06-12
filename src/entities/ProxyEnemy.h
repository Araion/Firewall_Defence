#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class ProxyEnemy : public Enemy {
public:
    ProxyEnemy(ResourceManager& res, const Path* path, PlayState& owner,
               float hpScale = 1.f, float speedScale = 1.f, float rewardScale = 1.f);

    std::string getTypeName() const override { return "ProxyEnemy"; }
    void update(float dt) override;

protected:
    void drawBody(sf::RenderWindow& window) override;

private:
    float m_warpTimer = 3.5f;   // odstep miedzy teleportami
    float m_warpRadius = 150.f; // zasieg dzialania na innych wrogow
    float m_warpJump = 150.f;   // dystans teleportu do przodu (px)
};
