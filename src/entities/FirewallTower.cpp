#include "entities/FirewallTower.h"
#include "entities/TowerType.h"
#include "entities/Enemy.h"
#include "entities/SlowEffect.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include "util/MathUtils.h"
#include <cmath>
#include <string>

FirewallTower::FirewallTower(PlayState& state, ResourceManager& res, ConfigManager& cfg)
    : Tower(state, res) {
    const TowerMeta& meta = towerMeta(TowerType::Firewall);

    m_name = meta.name;
    m_color = meta.color;
    m_cost = cfg.getInt(meta.configCostKey, meta.defaultCost);
    m_cpuCost = meta.cpuCost;

    // Statystyki bazowe wiezy
    m_baseDamage = 6.f;
    m_baseRange = 120.f;
    m_fireCooldown = 1.5f;

    // Glowka wiezy wykonuje delikatny ruch gora-dol
    m_headMotion = HeadMotion::Bob;
    m_headOffset = {0.f, -35.f};

    finalizeSetup();
}

void FirewallTower::update(float dt) {
    m_animTime += dt;

    if (m_cooldownTimer > 0.f)
        m_cooldownTimer -= dt;

    bool dealDamage = m_cooldownTimer <= 0.f;
    float slow = slowFraction();

    // Spowalnia wszystkich przeciwnikow znajdujacych sie w zasiegu pola
    for (Enemy* enemy : m_state.enemies()) {
        if (MathUtils::distance(m_position, enemy->getPosition()) > effectiveRange())
            continue;

        enemy->applySlow(1.f - slow, 2.0f);

        if (dealDamage)
            enemy->takeDamage(m_damage);
    }

    if (dealDamage)
        m_cooldownTimer = m_fireCooldown;

    // Okresowo dodaje efekt wizualny pola spowolnienia
    m_pulseTimer -= dt;

    if (m_pulseTimer <= 0.f) {
        m_pulseTimer = 0.75f;
        m_state.spawn(std::make_unique<SlowEffect>(m_position, effectiveRange()));
    }
}

void FirewallTower::drawBaseLayer(sf::RenderWindow& window) {
    // Rysuje pole spowolnienia pod wieza
    float pulse = 1.f + 0.03f * std::sin(m_animTime * 3.f);
    float radius = effectiveRange() * pulse;

    sf::CircleShape field(radius);
    field.setOrigin(radius, radius);
    field.setPosition(m_position);
    field.setFillColor(sf::Color(m_color.r, m_color.g, m_color.b, 30));
    field.setOutlineThickness(1.5f);
    field.setOutlineColor(sf::Color(m_color.r, m_color.g, m_color.b, 90));

    window.draw(field);

    Tower::drawBaseLayer(window);
}

float FirewallTower::slowFraction() const {
    // Oblicza procent spowolnienia na podstawie poziomu wiezy
    float slow = 0.35f + 0.15f * static_cast<float>(m_level - 1);

    return slow > 0.90f ? 0.90f : slow;
}

std::string FirewallTower::statLine() const {
    int percent = static_cast<int>(slowFraction() * 100.f + 0.5f);

    return "Spowolnienie: -" + std::to_string(percent) + "%";
}