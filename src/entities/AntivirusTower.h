#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// AntivirusTower - wieza strzelajaca pojedyncze pociski
// =============================================================
class AntivirusTower : public Tower {
public:
    AntivirusTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "AntivirusTower"; }
    void attack() override;
};
