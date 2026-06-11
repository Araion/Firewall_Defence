#include "entities/CorruptionTower.h"
#include "entities/TowerType.h"
#include "entities/BulletProjectile.h"
#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include <string>

CorruptionTower::CorruptionTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::Corruption);
    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);

    m_baseDamage = 4.f;
    m_baseRange = 150.f;
    m_fireCooldown = 1.0f;
    m_rotationSpeed = 300.f;

    m_headPivot  = {0.42f, 0.48f};
    m_headOffset = {0.f, -30.f};
    finalizeSetup();
}

void CorruptionTower::attack() {
    if (!m_target) return;
    sf::Vector2f tip = barrelTip(22.f);
    sf::Vector2f dir = m_target->getPosition() - tip;
    auto bullet = std::make_unique<BulletProjectile>(
        m_res, tip, dir, 520.f, m_damage, effectiveRange() * 1.5f, m_color,
        "assets/textures/corruption_shot.png");

    bullet->setDot(dotDps(), m_dotDuration);
    m_state.spawn(std::move(bullet));
}

float CorruptionTower::dotDps() const {
    const float mult[kMaxLevel] = {2.0f, 2.5f, 3.5f};
    int idx = m_level - 1;
    if (idx < 0) idx = 0;
    if (idx >= kMaxLevel) idx = kMaxLevel - 1;
    return m_dotDps * mult[idx];
}

std::string CorruptionTower::statLine() const {
    return "DoT: " + std::to_string(static_cast<int>(dotDps())) + "/s przez " +
           std::to_string(static_cast<int>(m_dotDuration)) + "s";
}
