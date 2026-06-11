#include "entities/DataCleanerTower.h"
#include "entities/TowerType.h"
#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include "util/MathUtils.h"
#include <string>

DataCleanerTower::DataCleanerTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::DataCleaner);
    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);

    m_baseDamage = 0.f;
    m_baseRange = 130.f;
    m_fireCooldown = 1.2f;
    m_rotationSpeed = 200.f;

    m_headMotion = HeadMotion::Bob;
    m_canShoot = false;
    m_headOffset = {0.f, -20.f};

    finalizeSetup();
}

void DataCleanerTower::update(float dt) {
    Tower::update(dt);
    // Impuls skanujacy ujawnia zaszyfrowanych wrogow w zasiegu
    float r = effectiveRange();
    for (Enemy* e : m_state.enemies())
        if (e->isEncrypted() && !e->isDetected() &&
            MathUtils::distance(m_position, e->getPosition()) <= r)
            e->markDetected();
}

void DataCleanerTower::applyLevelStats() {
    m_damage = 0.f;
    m_range = m_baseRange;
    for (int i = 1; i < m_level; ++i) m_range *= 1.35f;
}

std::string DataCleanerTower::statLine() const {
    return "Wykrywa zaszyfrowanych";
}
