#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class TrojanEnemy : public Enemy {
public:
    TrojanEnemy(ResourceManager& res, const Path* path,
                float hpScale = 1.f, float speedScale = 1.f, float rewardScale = 1.f);

    std::string getTypeName() const override { return "TrojanEnemy"; }
    void onDeath(PlayState& state) override;

protected:
    void drawBody(sf::RenderWindow& window) override;
};