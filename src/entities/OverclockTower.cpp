#include "entities/OverclockTower.h"
#include "entities/TowerType.h"
#include "entities/BoostEffect.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include "util/MathUtils.h"
#include <cmath>
#include <string>

OverclockTower::OverclockTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::Overclock);
    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);
    m_cpuCost = meta.cpuCost;

    m_baseDamage = 0.f;
    m_baseRange = 160.f;
    m_fireCooldown = 1.f;

    m_headMotion = HeadMotion::Bob;
    m_headOffset = {0.f, -40.f};
    m_canShoot = false;
    finalizeSetup();
}

void OverclockTower::applyLevelStats() {
    Tower::applyLevelStats();
    int pct = 20 + 15 * (m_level - 1); // +20% / +35% / +50%
    m_boostFactor = 1.f + static_cast<float>(pct) / 100.f;
}

std::string OverclockTower::statLine() const {
    int pct = static_cast<int>((m_boostFactor - 1.f) * 100.f + 0.5f);
    return "Przyspieszenie: +" + std::to_string(pct) + "%";
}

void OverclockTower::update(float dt) {
    m_animTime += dt;
    m_pulseTimer -= dt;
    if (m_pulseTimer <= 0.f) {
        m_pulseTimer = 0.6f;
        m_state.spawn(std::make_unique<BoostEffect>(m_position, effectiveRange()));
    }
}

void OverclockTower::applyAura(const std::vector<Tower*>& towers) {
    for (Tower* t : towers) {
        if (t == this) continue;
        if (MathUtils::distance(m_position, t->getPosition()) <= effectiveRange())
            t->setFireRateBoost(m_boostFactor);
    }
}