#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class GlitchDroneEnemy : public Enemy {
public:
    GlitchDroneEnemy(ResourceManager& res, const Path* path, PlayState& owner,
                     float hpScale = 1.f, float speedScale = 1.f, float rewardScale = 1.f);

    std::string getTypeName() const override { return "GlitchDroneEnemy"; }
    void update(float dt) override;

protected:
    void drawBody(sf::RenderWindow& window) override;

private:
    float m_empTimer = 4.f;   // odstep miedzy impulsami DDoS
};