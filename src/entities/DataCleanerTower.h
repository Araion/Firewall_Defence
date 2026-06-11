#pragma once
#include "entities/Tower.h"

class ConfigManager;

// =============================================================
// DataCleanerTower - wieza-detektor. Nie zadaje obrazen ani nie
// strzela - tylko skanuje i ujawnia ZASZYFROWANYCH wrogow w zasiegu
// (po wykryciu moga je atakowac Antivirus/Laser)
// =============================================================
class DataCleanerTower : public Tower {
public:
    DataCleanerTower(PlayState& state, ResourceManager& res, ConfigManager& cfg);

    std::string getTypeName() const override { return "DataCleanerTower"; }
    void update(float dt) override;   // skan zaszyfrowanych (nie strzela)
    void attack() override {}
    void applyLevelStats() override;   // bez obrazen, mocny wzrost zasiegu
    std::string statLine() const override;
};
