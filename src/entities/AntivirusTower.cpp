#include "entities/AntivirusTower.h"
#include "entities/TowerType.h"
#include "entities/BulletProjectile.h"
#include "entities/Enemy.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"

AntivirusTower::AntivirusTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::Antivirus);
    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);
    m_cpuCost = meta.cpuCost;

    // Statystyki bazowe wiezy
    m_baseDamage = 20.f;
    m_baseRange = 150.f;
    m_fireCooldown = 0.75f;
    m_rotationSpeed = 320.f;

    // Ustawienie glowki wzgledem tekstury wiezy
    m_headPivot = {0.323f, 0.5f};
    m_headOffset = {0.f, -30.f};

    finalizeSetup();
}

void AntivirusTower::attack() {
    if (!m_target) return;

    // Pocisk startuje z konca lufy i leci w strone aktualnego celu
    sf::Vector2f tip = barrelTip(22.f);
    sf::Vector2f dir = m_target->getPosition() - tip;
    m_state.spawn(std::make_unique<BulletProjectile>(
        m_res, tip, dir, 650.f, m_damage, m_range * 1.5f, m_color,
        "assets/textures/antivirus_shot.png"));
}
