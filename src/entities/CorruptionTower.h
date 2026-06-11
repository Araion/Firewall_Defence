#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// CorruptionTower - wieza zadajaca obrazenia w czasie
// =============================================================
class CorruptionTower : public Tower {
public:
    CorruptionTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "CorruptionTower"; }
    void attack() override;
    std::string statLine() const override;

private:
    float dotDps() const;
    float m_dotDps = 14.f;
    float m_dotDuration = 3.f;
};
