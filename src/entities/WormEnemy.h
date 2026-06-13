#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class WormEnemy : public Enemy {
public:
    WormEnemy(ResourceManager& res, const Path* path, int gen = 0,
              float hpScale = 1.f, float speedScale = 1.f, float rewardScale = 1.f);

    std::string getTypeName() const override { return "WormEnemy"; }
    int generation() const { return m_gen; } // faza worma - do zapisu
    void onDeath(PlayState& state) override;

protected:
    void drawBody(sf::RenderWindow& window) override;

private:
    int m_gen;
};