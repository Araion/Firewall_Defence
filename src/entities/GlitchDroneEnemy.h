#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class GlitchDroneEnemy : public Enemy {
public:
    GlitchDroneEnemy(ResourceManager& res, const Path* path);

    std::string getTypeName() const override { return "GlitchDroneEnemy"; }

protected:
    void drawBody(sf::RenderWindow& window) override;
};