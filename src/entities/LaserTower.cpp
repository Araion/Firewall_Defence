#include "entities/LaserTower.h"
#include "entities/TowerType.h"
#include "entities/Enemy.h"
#include "entities/LaserBeamProjectile.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"

LaserTower::LaserTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::Laser);

    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);

    // Statystyki bazowe wiezy
    m_baseDamage = 12.f;
    m_baseRange = 200.f;
    m_fireCooldown = 0.2f;
    m_rotationSpeed = 260.f;

    // Ustawienie glowki wzgledem tekstury wiezy
    m_headPivot = {0.43f, 0.48f};
    m_headOffset = {0.f, -33.f};

    finalizeSetup();
}

void LaserTower::attack() {
    if (!m_target)
        return;

    // Wieza zadaje obrazenia bezposrednio celowi
    m_target->takeDamage(m_damage);

    // Osobny obiekt odpowiada tylko za krotki efekt promienia
    sf::Vector2f tip = headPoint(73.5f * m_headScale, 20.4f * m_headScale);

    m_state.spawn(std::make_unique<LaserBeamProjectile>(
        tip,
        m_target->getPosition(),
        m_color,
        4.f,
        0.16f
        ));
}