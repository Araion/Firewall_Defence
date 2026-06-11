#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// LaserTower - wieza z ciaglym promieniem
// =============================================================
class LaserTower : public Tower {
public:
    LaserTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "LaserTower"; }
    void attack() override;
};
