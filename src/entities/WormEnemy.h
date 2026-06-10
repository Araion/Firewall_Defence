#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class WormEnemy : public Enemy {
public:
    WormEnemy(ResourceManager& res, const Path* path, int gen = 0);

    std::string getTypeName() const override { return "WormEnemy"; }

protected:
    void drawBody(sf::RenderWindow& window) override;

private:
    int m_gen;
};