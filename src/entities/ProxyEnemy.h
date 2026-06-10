#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class ProxyEnemy : public Enemy {
public:
    ProxyEnemy(ResourceManager& res, const Path* path);

    std::string getTypeName() const override { return "ProxyEnemy"; }

protected:
    void drawBody(sf::RenderWindow& window) override;
};