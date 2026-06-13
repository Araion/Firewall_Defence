#include "entities/EMPTower.h"
#include "entities/TowerType.h"
#include "entities/Enemy.h"
#include "entities/BulletProjectile.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"

EMPTower::EMPTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::EMP);
    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);
    m_cpuCost = meta.cpuCost;

    // Statystyki bazowe wiezy
    m_baseDamage = 25.f;
    m_baseRange = 135.f;
    m_fireCooldown = 1.5f;
    m_rotationSpeed = 360.f;

    // Ustawienie glowki wzgledem tekstury wiezy
    m_headPivot  = {0.43f, 0.50f};
    m_headOffset = {0.f, -30.f};
    finalizeSetup();
}

void EMPTower::attack() {
    if (!m_target) return;
    // Pocisk EMP zadaje obrazenia obszarowe po trafieniu
    sf::Vector2f tip = barrelTip(20.f);
    sf::Vector2f dir = m_target->getPosition() - tip;
    auto bullet = std::make_unique<BulletProjectile>(
        m_res, tip, dir, 430.f, m_damage, effectiveRange() * 1.6f, m_color,
        "assets/textures/emp_shot.png");
    bullet->setSplashRadius(m_splashRadius);
    m_state.spawn(std::move(bullet));
}
