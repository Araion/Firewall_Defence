#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class VirusEnemy : public Enemy {
public:
    VirusEnemy(ResourceManager& res, const Path* path,
               float hpScale = 1.f, float speedScale = 1.f, float rewardScale = 1.f);

    std::string getTypeName() const override { return "VirusEnemy"; }

protected:
    void drawBody(sf::RenderWindow& window) override;
};