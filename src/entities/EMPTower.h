#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// EMPTower - wieza zadajaca obrazenia obszarowe
// =============================================================
class EMPTower : public Tower {
public:
    EMPTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "EMPTower"; }
    void attack() override;

private:
    float m_splashRadius = 78.f;   // promien wybuchu obszarowego
};
