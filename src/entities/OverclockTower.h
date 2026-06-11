#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// OverclockTower - wieza wsparcia. Nie strzela - roztacza aure, ktora
// zwieksza szybkostrzelnosc sasiednim wiezom w zasiegu
// =============================================================
class OverclockTower : public Tower {
public:
    OverclockTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "OverclockTower"; }
    void update(float dt) override;
    void applyAura(const std::vector<Tower*>& towers) override;
    void attack() override {}
    std::string statLine() const override;

protected:
    void applyLevelStats() override;

private:
    float m_pulseTimer = 0.f;
    float m_boostFactor = 1.20f;
};
