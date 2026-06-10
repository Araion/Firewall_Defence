#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class TrojanEnemy : public Enemy {
public:
    TrojanEnemy(ResourceManager& res, const Path* path);

    std::string getTypeName() const override { return "TrojanEnemy"; }

protected:
    void drawBody(sf::RenderWindow& window) override;
};