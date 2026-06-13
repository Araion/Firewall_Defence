#include "states/PlayState.h"
#include "core/Game.h"
#include "states/MainMenuState.h"
#include "states/GameOverState.h"
#include "states/TutorialDirector.h"
#include "entities/ServerCore.h"
#include "entities/SlowEffect.h"
#include "entities/Enemy.h"
#include "entities/VirusEnemy.h"
#include "entities/TrojanEnemy.h"
#include "entities/WormEnemy.h"
#include "entities/GlitchDroneEnemy.h"
#include "entities/ProxyEnemy.h"
#include "entities/BossMalwareEnemy.h"
#include "entities/Tower.h"
#include "entities/AntivirusTower.h"
#include "entities/FirewallTower.h"
#include "entities/LaserTower.h"
#include "entities/DataCleanerTower.h"
#include "entities/OverclockTower.h"
#include "entities/CorruptionTower.h"
#include "entities/EMPTower.h"
#include "entities/ExplosionEffect.h"
#include "managers/ConfigManager.h"
#include "managers/ResourceManager.h"
#include "managers/WaveManager.h"
#include "managers/SaveManager.h"
#include "managers/AudioManager.h"
#include "util/Theme.h"
#include "util/MathUtils.h"
#include "util/MapFactory.h"
#include "util/Rng.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Wymiary paskow UI
static constexpr float HUD_H  = 56.f;
static constexpr float SHOP_H = 72.f;
static constexpr float SHOP_Y = 720.f - SHOP_H;

// Geometria panelu wiezy
static constexpr float PANEL_X = 994.f, PANEL_Y = 70.f, PANEL_W = 272.f, PANEL_H = 264.f;

PlayState::PlayState(Game& game, bool tutorial) : GameState(game), m_tutorial(tutorial) {
    auto& cfg = m_game.getConfig();
    m_credits = cfg.getInt("startingCredits", 300);
    m_serverHealth = m_serverMaxHealth = cfg.getInt("serverHealth", 20);
    m_totalWaves = cfg.getInt("totalWaves", 20);
    m_cpuCapacity = cfg.getInt("cpuCapacity", 100);
    m_heatSoftThreshold = cfg.getFloat("heatSoftThreshold", 0.7f);
    m_overclockHeatPerSec = cfg.getFloat("overclockHeatPerSecond", 8.f);
    m_enemyAbilities = cfg.getBool("enableEnemyAbilities", true);
    m_towerCostPercent = cfg.getInt("towerCostPercent", 100);
    m_systemUpgradeInterval = cfg.getInt("systemUpgradeInterval", 5);
    m_breachChance = cfg.getFloat("breachEventChance", 0.25f);
    m_breachWormOnly = cfg.getBool("breachWormOnly", false);

    float diff = cfg.getFloat("difficultyMultiplier", 1.f);
    m_difficultyLevel = (diff <= 0.9f) ? 0 : (diff >= 1.15f ? 2 : 1);
    m_mapSeed = static_cast<unsigned>(Rng::range(1, 2000000000));

    if (m_tutorial) {
        m_credits = 1000;
        m_serverHealth = m_serverMaxHealth = 100;
        m_breachChance = 0.f;
        m_systemUpgradeInterval = 0;
        for (bool& u : m_towerUnlocked) u = false;
    } else {
        for (bool& u : m_towerUnlocked) u = true;
    }

    auto& res = m_game.getResources();
    const sf::Font& font = res.getFont();
    auto initText = [&](sf::Text& t, unsigned size, sf::Color c) {
        t.setFont(font); t.setCharacterSize(size); t.setFillColor(c);
    };
    initText(m_txtWave, 20, Theme::TextMain);
    initText(m_txtCredits, 20, Theme::NeonGreen);
    initText(m_txtHealth, 20, Theme::NeonCyan);
    initText(m_txtScore, 20, Theme::TextMain);
    initText(m_txtHint, 15, Theme::TextDim);
    m_txtHint.setString("LPM: sklep / pad / wieza   |   1-7: wybor wiezy   |   SPACE: pauza   |   ESC: panel / menu");

    m_btnUpgrade.setup(font, "ULEPSZ", {PANEL_X + 10.f, PANEL_Y + 184.f}, {PANEL_W - 20.f, 32.f}, 18);
    m_btnUpgrade.setColors(Theme::PanelSolid, Theme::NeonGreen, Theme::TextMain, Theme::NeonGreen);
    m_btnSell.setup(font, "SPRZEDAJ", {PANEL_X + 10.f, PANEL_Y + 222.f}, {PANEL_W - 20.f, 32.f}, 18);
    m_btnSell.setColors(Theme::PanelSolid, Theme::Warn, Theme::TextMain, Theme::Warn);

    m_btnSpeed.setup(font, ">> x1", {848.f, 661.f}, {76.f, 48.f}, 18);
    m_btnSpeed.setColors(Theme::PanelSolid, Theme::NeonCyan, Theme::TextMain, Theme::NeonCyan);
    m_btnSaveGame.setup(font, "ZAPISZ (F5)", {934.f, 661.f}, {158.f, 48.f}, 18);
    m_btnSaveGame.setColors(Theme::PanelSolid, Theme::NeonGreen, Theme::TextMain, Theme::NeonGreen);
    m_btnMenu.setup(font, "MENU (ESC)", {1102.f, 661.f}, {158.f, 48.f}, 18);
    m_btnMenu.setColors(Theme::PanelSolid, Theme::Warn, Theme::TextMain, Theme::Warn);

    buildBoardFromSeed(m_mapSeed);

    m_waves = std::make_unique<WaveManager>(*this, cfg, m_pathPtrs);
    m_totalWaves = m_waves->totalWaves();

    if (m_tutorial) m_tutorialDirector = std::make_unique<TutorialDirector>(*this);

    initAbilities();
    playMusic("music_game");
}

PlayState::~PlayState() = default;

void PlayState::buildBoardFromSeed(unsigned seed) {
    auto& res = m_game.getResources();

    m_objects.clear();
    m_pendingSpawns.clear();
    m_paths.clear();
    m_pathPtrs.clear();
    m_breaches.clear();
    m_server = nullptr;

    LevelMap map = m_tutorial ? MapFactory::generate(0, 12345)
                              : MapFactory::generate(m_difficultyLevel, seed);

    for (const auto& lane : map.lanes)
        m_paths.push_back(std::make_unique<Path>(lane));
    for (auto& p : m_paths) m_pathPtrs.push_back(p.get());

    // Wypelnianie tuneli puste, otworza sie losowo podczas gry z istniejacych metod
    /*
    for (const auto& bl : map.breachLanes) {
        BreachLane lane;
        lane.path = std::make_unique<Path>(bl);
        m_breaches.push_back(std::move(lane));
    }
    */

    auto server = std::make_unique<ServerCore>(res, map.serverPos);
    m_server = server.get();
    m_server->setHealth(m_serverHealth, m_serverMaxHealth);
    m_objects.push_back(std::move(server));
}

bool PlayState::canPlaceAt(sf::Vector2f pos) const {
    const float R = kTowerFootprint;
    if (pos.x < 24.f + R || pos.x > 1180.f || pos.y < HUD_H + 18.f || pos.y > SHOP_Y - 18.f)
        return false;
    if (m_server && MathUtils::distance(pos, m_server->getPosition()) < 70.f + R)
        return false;

    const float pathClear = 15.f + R + 2.f;
    for (auto& path : m_paths) {
        const auto& pts = path->points();
        for (size_t i = 1; i < pts.size(); ++i)
            if (MathUtils::pointSegmentDistance(pos, pts[i - 1], pts[i]) < pathClear)
                return false;
    }

    for (const auto& b : m_breaches) {
        if (!b.active || !b.path) continue;
        const auto& pts = b.path->points();
        for (size_t i = 1; i < pts.size(); ++i)
            if (MathUtils::pointSegmentDistance(pos, pts[i - 1], pts[i]) < pathClear)
                return false;
    }

    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (const Tower* t = dynamic_cast<const Tower*>(o.get()))
            if (MathUtils::distance(pos, t->getPosition()) < 2.f * R) return false;
    }
    return true;
}

bool PlayState::towerNear(sf::Vector2f pos, float radius) const {
    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (const Tower* t = dynamic_cast<const Tower*>(o.get()))
            if (MathUtils::distance(pos, t->getPosition()) <= radius) return true;
    }
    return false;
}

Tower* PlayState::towerAt(sf::Vector2f pos) const {
    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (Tower* t = dynamic_cast<Tower*>(o.get()))
            if (t->getBounds().contains(pos)) return t;
    }
    return nullptr;
}

void PlayState::initAbilities() {
    float cd = m_game.getConfig().getFloat("abilityCooldown", 40.f);
    m_abilities.clear();

    Ability coolant; coolant.name = "COOLANT"; coolant.kind = 0;
    coolant.pos = {1236.f, 96.f}; coolant.cooldownMax = cd; coolant.color = Theme::NeonCyan;

    Ability surge; surge.name = "SURGE"; surge.kind = 1;
    surge.pos = {1236.f, 168.f}; surge.cooldownMax = cd; surge.color = Theme::Warn;

    m_abilities.push_back(coolant);
    m_abilities.push_back(surge);
    m_abilitiesEnabled = !m_tutorial;
}

int PlayState::abilityAt(sf::Vector2f pos) const {
    if (!m_abilitiesEnabled) return -1;
    for (int i = 0; i < static_cast<int>(m_abilities.size()); ++i) {
        const Ability& a = m_abilities[i];
        if (MathUtils::distance(pos, a.pos) <= a.radius) return i;
    }
    return -1;
}

void PlayState::activateAbility(int idx) {
    if (idx < 0 || idx >= static_cast<int>(m_abilities.size())) return;
    Ability& a = m_abilities[idx];
    if (a.cooldown > 0.f) { showMessage("Moc sie laduje...", Theme::Warn); playSfx("denied", 60.f); return; }

    if (a.kind == 0) {
        for (Enemy* e : m_frameEnemies) e->applySlow(0.40f, 4.0f);
        m_heatFreezeTimer = 8.0f;
        spawn(std::make_unique<SlowEffect>(sf::Vector2f(640.f, 360.f), 700.f, 1.0f));
        showMessage("COOLANT: spowolnienie + TEMP nie rosnie przez 8s!", Theme::NeonCyan);
    } else {
        for (Enemy* e : m_frameEnemies) {
            e->takeDamage(45.f);
            spawnExplosion(e->getPosition(), Theme::Warn, 0.7f);
        }
        showMessage("SURGE: impuls EMP uderza we wszystkich!", Theme::Warn);
    }
    a.cooldown = a.cooldownMax;
    playSfx(a.kind == 0 ? "ability_coolant" : "ability_surge", 95.f);
}

void PlayState::spawn(std::unique_ptr<GameObject> obj) {
    m_pendingSpawns.push_back(std::move(obj));
}

void PlayState::flushPending() {
    for (auto& o : m_pendingSpawns) m_objects.push_back(std::move(o));
    m_pendingSpawns.clear();
}

bool PlayState::spendCredits(int c) {
    if (m_credits < c) return false;
    m_credits -= c;
    return true;
}

void PlayState::showMessage(const std::string& msg, sf::Color color) {
    m_message = msg;
    m_messageColor = color;
    m_messageTimer = 2.5f;
}

void PlayState::playSfx(const std::string& name, float volume) {
    m_game.getAudio().play(name, volume);
}

void PlayState::playMusic(const std::string& name) {
    m_game.getAudio().playMusic(name);
}

ResourceManager& PlayState::resources() {
    return m_game.getResources();
}

void PlayState::spawnExplosion(sf::Vector2f pos, sf::Color color, float scale) {
    spawn(std::make_unique<ExplosionEffect>(pos, color, scale));
}

void PlayState::rebuildCaches() {
    m_frameEnemies.clear();
    m_frameTowers.clear();
    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (Enemy* e = dynamic_cast<Enemy*>(o.get())) m_frameEnemies.push_back(e);
        else if (Tower* t = dynamic_cast<Tower*>(o.get())) {
            t->resetFireRateBoost();
            m_frameTowers.push_back(t);
        }
    }
}

void PlayState::updateHeat(float dt) {
    float cpuFrac = m_cpuCapacity > 0 ? static_cast<float>(m_cpuUsage) / m_cpuCapacity : 0.f;

    float delta;
    if (cpuFrac > m_heatSoftThreshold) {
        float over = (cpuFrac - m_heatSoftThreshold) / std::max(0.01f, 1.f - m_heatSoftThreshold);
        delta = over * 12.f;
    } else {
        delta = -14.f;
    }

    int overclocks = 0;
    for (Tower* t : m_frameTowers)
        if (t->getTypeName() == "OverclockTower") ++overclocks;
    delta += overclocks * m_overclockHeatPerSec;

    if (delta > 0.f) delta *= m_heatReduction;

    if (m_heatFreezeTimer > 0.f) {
        m_heatFreezeTimer -= dt;
        if (delta > 0.f) delta = 0.f;
    }

    m_heat += delta * dt;
    m_heat = std::max(0.f, std::min(100.f, m_heat));
}

void PlayState::updateHover() {
    sf::Vector2i mi = sf::Mouse::getPosition(m_game.getWindow());
    m_mouse = sf::Vector2f(static_cast<float>(mi.x), static_cast<float>(mi.y));
}

void PlayState::handleDeaths() {
    for (auto& o : m_objects) {
        if (o->isAlive()) continue;
        if (Enemy* e = dynamic_cast<Enemy*>(o.get())) {
            if (e->reachedServer()) {
                m_serverHealth = std::max(0, m_serverHealth - e->getServerDamage());
                playSfx("server_hit", 90.f);
            } else {
                m_credits += static_cast<int>(e->getReward() * m_bountyMult);
                m_score += e->getPoints();
                spawnExplosion(e->getPosition(), e->getBodyColor(), 1.0f);
                m_game.getAudio().play("explosion", 55.f);
                e->onDeath(*this);
            }
        }
    }
}

void PlayState::removeDead() {
    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
                       [](const std::unique_ptr<GameObject>& o) { return !o->isAlive(); }),
        m_objects.end());
}

void PlayState::update(float dt) {
    if (m_messageTimer > 0.f) m_messageTimer -= dt;
    m_uiTime += dt;

    sf::Vector2i mi = sf::Mouse::getPosition(m_game.getWindow());
    m_mouse = sf::Vector2f(static_cast<float>(mi.x), static_cast<float>(mi.y));

    m_btnSpeed.update(dt, m_mouse);
    m_btnSaveGame.update(dt, m_mouse);
    m_btnMenu.update(dt, m_mouse);
    if (m_selectedTower) {
        m_btnUpgrade.update(dt, m_mouse);
        m_btnSell.update(dt, m_mouse);
    }

    if (m_paused || m_draftActive) return;

    if (m_tutorial && m_tutorialDirector && m_tutorialDirector->panelOpen()) {
        m_tutorialDirector->update(dt);
        return;
    }

    float sdt = dt * m_speed;

    flushPending();
    rebuildCaches();

    if (m_tutorial && m_tutorialDirector) {
        m_tutorialDirector->update(sdt);
        if (m_tutorialDirector->finished()) {
            m_game.changeState(std::make_unique<MainMenuState>(m_game));
            return;
        }
    } else {
        m_waves->update(sdt);
        int newWave = m_waves->currentWave();
        if (newWave != m_wave) onWaveChanged(newWave);
        m_wave = newWave;
        int completed = m_waves->takeCompletedWave();
        if (completed > 0) maybeTriggerDraft(completed);
    }

    updateHover();

    for (Tower* t : m_frameTowers) t->applyAura(m_frameTowers);
    for (auto& o : m_objects) o->update(sdt);

    m_collision.resolve(*this, m_objects);

    handleDeaths();
    removeDead();
    updateHeat(sdt);

    for (auto& a : m_abilities) if (a.cooldown > 0.f) a.cooldown -= sdt;

    if (m_server) m_server->setHealth(m_serverHealth, m_serverMaxHealth);

    if (!m_tutorial) {
        bool lost = (m_serverHealth <= 0) || (m_heat >= 100.f);
        bool won = m_waves->victory();
        if (lost || won) {
            int finalScore = m_score + std::max(0, m_serverHealth) * 100 + m_credits;
            m_game.changeState(std::make_unique<GameOverState>(m_game, finalScore, m_wave, m_difficultyLevel));
        }
    }
}

static sf::FloatRect draftCardRect(int i) {
    return sf::FloatRect(150.f + i * 340.f, 200.f, 300.f, 300.f);
}

void PlayState::handleEvent(const sf::Event& e) {
    if (m_tutorial && m_tutorialDirector && m_tutorialDirector->panelOpen()) {
        if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
            m_game.changeState(std::make_unique<MainMenuState>(m_game));
            return;
        }
        m_tutorialDirector->handleEvent(e);
        return;
    }

    if (m_draftActive) {
        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f m(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y));
            for (int i = 0; i < static_cast<int>(m_draftCards.size()); ++i)
                if (draftCardRect(i).contains(m)) { applyUpgrade(m_draftCards[i]); m_draftActive = false; break; }
        }
        return;
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        onLeftClick(sf::Vector2f(static_cast<float>(e.mouseButton.x),
                                 static_cast<float>(e.mouseButton.y)));
        return;
    }

    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Escape) {
            if (m_selectedTower) deselectAll();
            else if (m_buildSelection >= 0) setBuildSelection(-1);
            else { playSfx("ui_back", 80.f); m_game.changeState(std::make_unique<MainMenuState>(m_game)); }
            return;
        }
        if (e.key.code == sf::Keyboard::Space) { m_paused = !m_paused; playSfx("ui_click", 70.f); return; }
        if (e.key.code == sf::Keyboard::F5) { saveGame(); return; }
        if (e.key.code == sf::Keyboard::F9) { loadGame(); return; }

        if (e.key.code == sf::Keyboard::Q) { activateAbility(0); return; }
        if (e.key.code == sf::Keyboard::W) { activateAbility(1); return; }

        int idx = -1;
        if (e.key.code >= sf::Keyboard::Num1 && e.key.code <= sf::Keyboard::Num7)
            idx = e.key.code - sf::Keyboard::Num1;
        else if (e.key.code >= sf::Keyboard::Numpad1 && e.key.code <= sf::Keyboard::Numpad7)
            idx = e.key.code - sf::Keyboard::Numpad1;

        if (idx >= 0 && idx < static_cast<int>(TowerType::Count)) {
            if (m_tutorial && (m_tutBuildLocked || (m_tutOnlySlot >= 0 && idx != m_tutOnlySlot))) return;
            if (m_towerUnlocked[idx]) { deselectAll(); setBuildSelection(idx); }
            else { showMessage("Ta wieza jest zablokowana", Theme::Warn); playSfx("denied", 60.f); }
        }
    }
}

void PlayState::onLeftClick(sf::Vector2f mouse) {
    if (m_btnSpeed.contains(mouse)) {
        m_speed = (m_speed >= 2.f) ? 1.f : 2.f;
        m_btnSpeed.setLabel(m_speed >= 2.f ? ">> x2" : ">> x1");
        playSfx("ui_click", 70.f);
        return;
    }
    if (m_btnSaveGame.contains(mouse)) { saveGame(); return; }
    if (m_btnMenu.contains(mouse)) { playSfx("ui_back", 80.f); m_game.changeState(std::make_unique<MainMenuState>(m_game)); return; }

    if (mouse.y >= SHOP_Y) {
        for (int i = 0; i < static_cast<int>(TowerType::Count); ++i) {
            if (shopSlotRect(i).contains(mouse)) {
                if (!m_towerUnlocked[i]) { showMessage("Ta wieza jest zablokowana", Theme::Warn); playSfx("denied", 60.f); return; }
                if (m_tutorial && (m_tutBuildLocked || (m_tutOnlySlot >= 0 && i != m_tutOnlySlot))) return;
                deselectAll();
                setBuildSelection((m_buildSelection == i) ? -1 : i);
                return;
            }
        }
        return;
    }

    if (m_selectedTower) {
        if (m_btnUpgrade.contains(mouse)) { upgradeSelected(); return; }
        if (m_btnSell.contains(mouse)) { sellSelected(); return; }
    }

    int ab = abilityAt(mouse);
    if (ab >= 0) { activateAbility(ab); return; }

    if (m_buildSelection >= 0) {
        tryBuildAt(static_cast<TowerType>(m_buildSelection), mouse);
        return;
    }

    if (Tower* t = towerAt(mouse)) { selectTower(t); return; }

    deselectAll();
    setBuildSelection(-1);
}

void PlayState::unlockTower(int idx) {
    if (idx >= 0 && idx < static_cast<int>(TowerType::Count)) m_towerUnlocked[idx] = true;
}

void PlayState::setBuildSelection(int idx) {
    m_buildSelection = idx;
    if (idx >= 0 && idx < static_cast<int>(TowerType::Count)) {
        m_preview = createTower(static_cast<TowerType>(idx));
        playSfx("ui_adjust", 60.f);
    } else {
        m_preview.reset();
    }
}

void PlayState::setTutorialGate(bool buildLocked, int onlySlot,
                                sf::Vector2f target, bool hasTarget, float radius) {
    m_tutBuildLocked = buildLocked;
    m_tutOnlySlot = onlySlot;
    m_tutTarget = target;
    m_tutHasTarget = hasTarget;
    m_tutTargetRadius = radius;
}

void PlayState::selectTower(Tower* t) {
    deselectAll();
    m_selectedTower = t;
    if (t) t->setSelected(true);
    m_buildSelection = -1;
}

void PlayState::deselectAll() {
    if (m_selectedTower) m_selectedTower->setSelected(false);
    m_selectedTower = nullptr;
}

std::unique_ptr<Tower> PlayState::createTower(TowerType type) {
    auto& res = m_game.getResources();
    auto& cfg = m_game.getConfig();
    std::unique_ptr<Tower> t;
    switch (type) {
    case TowerType::Antivirus:   t = std::make_unique<AntivirusTower>(*this, res, cfg); break;
    case TowerType::Firewall:    t = std::make_unique<FirewallTower>(*this, res, cfg); break;
    case TowerType::Laser:       t = std::make_unique<LaserTower>(*this, res, cfg); break;
    case TowerType::DataCleaner: t = std::make_unique<DataCleanerTower>(*this, res, cfg); break;
    case TowerType::Overclock:   t = std::make_unique<OverclockTower>(*this, res, cfg); break;
    case TowerType::Corruption:  t = std::make_unique<CorruptionTower>(*this, res, cfg); break;
    case TowerType::EMP:         t = std::make_unique<EMPTower>(*this, res, cfg); break;
    default: return nullptr;
    }
    if (t) t->setGlobalRangeMult(m_globalRangeMult);
    return t;
}

// ---------------------------- ZAPIS / WCZYTANIE ----------------------------

int PlayState::pathIndexOf(const Path* p) const {
    for (int i = 0; i < static_cast<int>(m_pathPtrs.size()); ++i)
        if (m_pathPtrs[i] == p) return i;
    return 0;
}

static TowerType towerTypeFromName(const std::string& name) {
    if (name == "FirewallTower")    return TowerType::Firewall;
    if (name == "LaserTower")       return TowerType::Laser;
    if (name == "DataCleanerTower") return TowerType::DataCleaner;
    if (name == "OverclockTower")   return TowerType::Overclock;
    if (name == "CorruptionTower")  return TowerType::Corruption;
    if (name == "EMPTower")         return TowerType::EMP;
    return TowerType::Antivirus;
}

std::vector<PlayState::TowerSave> PlayState::snapshotTowers() const {
    std::vector<TowerSave> out;
    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (Tower* t = dynamic_cast<Tower*>(o.get())) {
            sf::Vector2f p = t->getPosition();
            out.push_back({t->getTypeName(), t->getLevel(), p.x, p.y});
        }
    }
    return out;
}

std::vector<PlayState::EnemySave> PlayState::snapshotEnemies() const {
    std::vector<EnemySave> out;
    for (const auto& o : m_objects) {
        if (!o->isAlive()) continue;
        if (Enemy* e = dynamic_cast<Enemy*>(o.get())) {
            if (e->reachedServer()) continue;
            out.push_back({e->getTypeName(), e->getHp(),
                           pathIndexOf(e->getPath()), e->getDistance()});
        }
    }
    return out;
}

std::vector<PlayState::BreachSave> PlayState::snapshotBreaches() const {
    std::vector<BreachSave> out;
    for (const Path* p : m_pathPtrs) {
        for (int i = 0; i < static_cast<int>(m_breaches.size()); ++i)
            if (m_breaches[i].active && m_breaches[i].path.get() == p) {
                out.push_back({i, m_breaches[i].wavesLeft});
                break;
            }
    }
    return out;
}

void PlayState::activateBreachFromSave(int index, int wavesLeft) {
    if (index < 0 || index >= static_cast<int>(m_breaches.size())) return;
    BreachLane& b = m_breaches[index];
    if (b.active || !b.path) return;
    b.active = true;
    b.wavesLeft = wavesLeft;
    m_pathPtrs.push_back(b.path.get());
}

void PlayState::buildBoard(unsigned seed, int difficulty) {
    deselectAll();
    m_buildSelection = -1;
    m_difficultyLevel = difficulty;
    m_mapSeed = seed;
    buildBoardFromSeed(seed);
    m_cpuUsage = 0;
    m_frameEnemies.clear();
    m_frameTowers.clear();
}

bool PlayState::placeTowerFromSave(const std::string& type, int level, sf::Vector2f pos) {
    auto tower = createTower(towerTypeFromName(type));
    if (!tower) return false;
    tower->setLevelDirect(level);
    tower->setPosition(pos);
    m_cpuUsage += tower->getCpuCost();
    m_objects.push_back(std::move(tower));
    return true;
}

void PlayState::addEnemyFromSave(const std::string& type, float hp, int pathIndex, float distance) {
    if (m_pathPtrs.empty()) return;
    int idx = std::max(0, std::min(pathIndex, static_cast<int>(m_pathPtrs.size()) - 1));
    const Path* path = m_pathPtrs[idx];
    auto& res = m_game.getResources();

    std::unique_ptr<Enemy> e;
    if (type == "TrojanEnemy")          e = std::make_unique<TrojanEnemy>(res, path);
    else if (type == "WormEnemy")       e = std::make_unique<WormEnemy>(res, path, 0);
    else if (type == "GlitchDroneEnemy") e = std::make_unique<GlitchDroneEnemy>(res, path, *this);
    else if (type == "ProxyEnemy")      e = std::make_unique<ProxyEnemy>(res, path, *this);
    else if (type == "BossMalwareEnemy") e = std::make_unique<BossMalwareEnemy>(res, path, *this);
    else                                e = std::make_unique<VirusEnemy>(res, path);

    e->setOwner(this);
    e->setHp(hp);
    e->setDistance(distance);
    m_objects.push_back(std::move(e));
}

void PlayState::applyLoadedUpgrades(float range, float bounty, float heat, bool heur,
                                    float slow, int cpuCap) {
    m_globalRangeMult = range;
    m_bountyMult = bounty;
    m_heatReduction = heat;
    m_heuristics = heur;
    m_slowStrength = slow;
    if (cpuCap > 0) m_cpuCapacity = cpuCap;
}

void PlayState::applyLoadedMeta(int waveNum, int credits, int serverHp, float heat, int scoreVal) {
    m_wave = waveNum;
    m_credits = credits;
    m_serverHealth = serverHp;
    m_heat = heat;
    m_score = scoreVal;
    if (m_server) m_server->setHealth(m_serverHealth, m_serverMaxHealth);
}

void PlayState::applyLoadedMeta(int credits, int serverHp, int scoreVal) {
    applyLoadedMeta(0, credits, serverHp, 0.f, scoreVal);
}

void PlayState::finishLoad() {
    bool enemiesPresent = false;
    for (const auto& o : m_objects)
        if (dynamic_cast<Enemy*>(o.get())) { enemiesPresent = true; break; }
    if (m_waves) m_waves->resumeAt(m_wave, enemiesPresent);
}

bool PlayState::saveGame() {
    SaveManager sm;
    bool ok = sm.save(*this);
    showMessage(ok ? "Gra zapisana (F5)" : "Blad zapisu!",
                ok ? Theme::NeonGreen : Theme::Danger);
    playSfx(ok ? "ui_click" : "denied", ok ? 80.f : 70.f);
    return ok;
}

bool PlayState::loadGame() {
    SaveManager sm;
    bool ok = sm.load(*this);
    if (ok) { showMessage("Gra wczytana (F9)", Theme::NeonGreen); playSfx("ui_click", 80.f); }
    else { showMessage("Brak zapisu / blad wczytania", Theme::Warn); playSfx("denied", 70.f); }
    return ok;
}

// ---------------------------- KREATYWNE MECHANIKI ----------------------------

namespace {
struct UpgradeMeta { const char* title; const char* desc; };
const UpgradeMeta kUpgrades[] = {
    {"Patch v2.1",          "+10% zasiegu\nwszystkich wiez"},
    {"Heurystyka",          "Wieze wykrywaja\nzaszyfrowanych wrogow"},
    {"Radiator",            "-20% generowanego\nciepla (TEMP)"},
    {"Podkrecony CPU",      "+20 do pojemnosci\nCPU"},
    {"Bounty",              "+25% kredytow\nza pokonanych wrogow"},
    {"Hartowany firewall",  "Spowolnienie\nfirewalla +15%"},
    };
const int kUpgradeCount = 6;
}

bool PlayState::anyBreachActive() const {
    for (const auto& b : m_breaches) if (b.active) return true;
    return false;
}

bool PlayState::isBreachPath(const Path* p) const {
    for (const auto& b : m_breaches) if (b.active && b.path.get() == p) return true;
    return false;
}

void PlayState::onWaveChanged(int newWave) {
    tickBreaches();
    if (newWave <= 1) return;
    if (Rng::chance(m_breachChance)) openBreach();
    if (Rng::chance(m_breachChance * 0.5f)) openBreach();
}

void PlayState::tickBreaches() {
    for (auto& b : m_breaches) {
        if (!b.active) continue;
        if (--b.wavesLeft <= 0) {
            auto it = std::find(m_pathPtrs.begin(), m_pathPtrs.end(), b.path.get());
            if (it != m_pathPtrs.end()) m_pathPtrs.erase(it);
            b.active = false;
            showMessage("Luka zalatana - wektor zamkniety", Theme::NeonGreen);
            playSfx("breach_closed", 80.f);
        }
    }
}

void PlayState::openBreach() {
    std::vector<int> inactive;
    for (int i = 0; i < static_cast<int>(m_breaches.size()); ++i)
        if (!m_breaches[i].active) inactive.push_back(i);
    if (inactive.empty()) return;

    int idx = inactive[Rng::range(0, static_cast<int>(inactive.size()) - 1)];
    m_breaches[idx].active = true;
    m_breaches[idx].wavesLeft = 3;
    m_pathPtrs.push_back(m_breaches[idx].path.get());
    showMessage("ALERT: Wykryto nowy wektor ataku!", Theme::Danger);
    playSfx("alarm_breach", 90.f);
}

void PlayState::maybeTriggerDraft(int completedWave) {
    if (m_systemUpgradeInterval <= 0) return;
    if (completedWave % m_systemUpgradeInterval == 0 && completedWave < m_totalWaves)
        openDraft();
}

void PlayState::openDraft() {
    m_draftCards.clear();
    std::vector<int> pool;
    for (int i = 0; i < kUpgradeCount; ++i) pool.push_back(i);
    for (int k = 0; k < 3 && !pool.empty(); ++k) {
        int j = Rng::range(0, static_cast<int>(pool.size()) - 1);
        m_draftCards.push_back(pool[j]);
        pool.erase(pool.begin() + j);
    }
    m_draftActive = true;
    m_messageTimer = 0.f;
    playSfx("ui_click", 85.f);
}

void PlayState::applyUpgrade(int card) {
    switch (card) {
    case 0:
        m_globalRangeMult *= 1.10f;
        for (auto& o : m_objects)
            if (Tower* t = dynamic_cast<Tower*>(o.get())) t->setGlobalRangeMult(m_globalRangeMult);
        break;
    case 1: m_heuristics = true; break;
    case 2: m_heatReduction *= 0.80f; break;
    case 3: m_cpuCapacity += 20; break;
    case 4: m_bountyMult *= 1.25f; break;
    case 5: m_slowStrength = std::min(0.85f, m_slowStrength + 0.15f); break;
    default: break;
    }
    if (card >= 0 && card < kUpgradeCount) {
        showMessage(std::string("Aktywowano: ") + kUpgrades[card].title, Theme::NeonGreen);
        playSfx("upgrade", 90.f);
    }
}

void PlayState::drawDraft(sf::RenderWindow& window) {
    sf::RectangleShape dim({1280.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(dim);

    const sf::Font& title = m_game.getResources().getFont();
    const sf::Font& body = m_game.getResources().getFont();

    sf::Text head("SYSTEM UPDATE", title, 52);
    head.setStyle(sf::Text::Bold);
    head.setFillColor(Theme::NeonMagenta);
    sf::FloatRect hb = head.getLocalBounds();
    head.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
    head.setPosition(640.f, 120.f);
    window.draw(head);

    sf::Text sub("Wybierz jedno trwale ulepszenie:", body, 22);
    sf::FloatRect sbb = sub.getLocalBounds();
    sub.setOrigin(sbb.left + sbb.width / 2.f, 0.f);
    sub.setPosition(640.f, 160.f);
    sub.setFillColor(Theme::TextDim);
    window.draw(sub);

    sf::Vector2i mi = sf::Mouse::getPosition(m_game.getWindow());
    sf::Vector2f mouse(static_cast<float>(mi.x), static_cast<float>(mi.y));

    for (int i = 0; i < static_cast<int>(m_draftCards.size()); ++i) {
        sf::FloatRect r = draftCardRect(i);
        bool hover = r.contains(mouse);
        sf::RectangleShape card({r.width, r.height});
        card.setPosition(r.left, r.top);
        card.setFillColor(hover ? sf::Color(40, 20, 50) : Theme::PanelSolid);
        card.setOutlineThickness(hover ? 4.f : 2.f);
        card.setOutlineColor(Theme::NeonMagenta);
        window.draw(card);

        const UpgradeMeta& meta = kUpgrades[m_draftCards[i]];
        sf::Text t(meta.title, body, 26);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(Theme::NeonCyan);
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width / 2.f, 0.f);
        t.setPosition(r.left + r.width / 2.f, r.top + 36.f);
        window.draw(t);

        sf::Text d(meta.desc, body, 19);
        d.setFillColor(Theme::TextMain);
        sf::FloatRect db = d.getLocalBounds();
        d.setOrigin(db.left + db.width / 2.f, 0.f);
        d.setPosition(r.left + r.width / 2.f, r.top + 130.f);
        window.draw(d);
    }
}

bool PlayState::tryBuildAt(TowerType type, sf::Vector2f pos) {
    if (m_tutorial) {
        if (m_tutBuildLocked) return false;
        if (m_tutOnlySlot >= 0 && static_cast<int>(type) != m_tutOnlySlot) return false;
    }

    if (!canPlaceAt(pos)) { showMessage("Nie mozna tu postawic", Theme::Warn); playSfx("denied", 60.f); return false; }

    const TowerMeta& meta = towerMeta(type);
    int cost = m_game.getConfig().getInt(meta.configCostKey, meta.defaultCost) * m_towerCostPercent / 100;

    auto tower = createTower(type);
    if (!tower) { showMessage("Wieza niedostepna", Theme::Warn); playSfx("denied", 60.f); return false; }

    int cpu = tower->getCpuCost();

    if (m_cpuUsage + cpu > m_cpuCapacity) { showMessage("Brak mocy CPU!", Theme::Danger); playSfx("denied", 60.f); return false; }
    if (m_credits < cost) { showMessage("Brak kredytow!", Theme::Danger); playSfx("denied", 60.f); return false; }

    m_credits -= cost;
    m_cpuUsage += cpu;
    tower->setPosition(pos);
    sf::Color tColor = tower->getColor();
    m_objects.push_back(std::move(tower));
    showMessage(std::string(meta.name) + " postawiony", tColor);
    m_game.getAudio().play("build");
    setBuildSelection(-1);
    return true;
}

void PlayState::sellSelected() {
    if (!m_selectedTower) return;
    Tower* t = m_selectedTower;
    int refund = t->getSellValue();
    m_credits += refund;
    m_cpuUsage = std::max(0, m_cpuUsage - t->getCpuCost());
    t->kill();
    m_selectedTower = nullptr;
    showMessage("Sprzedano (+" + std::to_string(refund) + ")", Theme::NeonGreen);
    m_game.getAudio().play("sell");
}

void PlayState::upgradeSelected() {
    if (!m_selectedTower) return;
    Tower* t = m_selectedTower;
    if (!t->canUpgrade()) { showMessage("Maksymalny poziom", Theme::Warn); playSfx("denied", 60.f); return; }
    int cost = t->getUpgradeCost();
    if (m_credits < cost) { showMessage("Brak kredytow!", Theme::Danger); playSfx("denied", 60.f); return; }
    m_credits -= cost;
    t->upgrade();
    showMessage("Ulepszono do poziomu " + std::to_string(t->getLevel()), Theme::NeonGreen);
    m_game.getAudio().play("upgrade");
}

void PlayState::drawThickLine(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b,
                              float thickness, sf::Color color) {
    sf::Vector2f d = b - a;
    float len = MathUtils::length(d);
    if (len < 1e-3f) return;
    sf::RectangleShape rect({len, thickness});
    rect.setOrigin(0.f, thickness / 2.f);
    rect.setPosition(a);
    rect.setRotation(MathUtils::angleDeg(d));
    rect.setFillColor(color);
    window.draw(rect);
}

void PlayState::drawBackground(sf::RenderWindow& window) {
    sf::RectangleShape bg({1280.f, 720.f});
    bg.setFillColor(Theme::Background);
    window.draw(bg);

    sf::VertexArray grid(sf::Lines);
    for (int x = 0; x <= 1280.f; x += 40.f) {
        grid.append(sf::Vertex({static_cast<float>(x), HUD_H}, Theme::GridLine));
        grid.append(sf::Vertex({static_cast<float>(x), SHOP_Y}, Theme::GridLine));
    }
    for (int y = HUD_H; y <= SHOP_Y; y += 40.f) {
        grid.append(sf::Vertex({0.f, static_cast<float>(y)}, Theme::GridLine));
        grid.append(sf::Vertex({1280.f, static_cast<float>(y)}, Theme::GridLine));
    }
    window.draw(grid);
}

void PlayState::drawPaths(sf::RenderWindow& window) {
    const sf::Color darkBase(22, 34, 52), darkBreach(60, 20, 28);
    const sf::Color lineBase(0, 120, 150);

    auto drawBacking = [&](const std::vector<sf::Vector2f>& pts, sf::Color dark) {
        for (size_t i = 1; i < pts.size(); ++i)
            drawThickLine(window, pts[i - 1], pts[i], 30.f, dark);
        for (const auto& p : pts) {
            sf::CircleShape joint(15.f);
            joint.setOrigin(15.f, 15.f);
            joint.setPosition(p);
            joint.setFillColor(dark);
            window.draw(joint);
        }
    };
    auto drawCenter = [&](const std::vector<sf::Vector2f>& pts, sf::Color line) {
        for (size_t i = 1; i < pts.size(); ++i)
            drawThickLine(window, pts[i - 1], pts[i], 3.f, line);
    };

    for (auto& path : m_paths) drawBacking(path->points(), darkBase);
    for (const auto& b : m_breaches)
        if (b.active && b.path) drawBacking(b.path->points(), darkBreach);

    for (auto& path : m_paths) drawCenter(path->points(), lineBase);
    for (const auto& b : m_breaches)
        if (b.active && b.path) drawCenter(b.path->points(), Theme::Danger);
}

void PlayState::drawBar(sf::RenderWindow& window, float x, float y, float w, float h,
                        float frac, sf::Color color, const std::string& label) {
    frac = std::max(0.f, std::min(1.f, frac));
    sf::RectangleShape bg({w, h});
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(0, 0, 0, 120));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(60, 80, 100));
    window.draw(bg);

    sf::RectangleShape fill({w * frac, h});
    fill.setPosition(x, y);
    fill.setFillColor(color);
    window.draw(fill);

    const sf::Font& font = m_game.getResources().getFont();
    sf::Text t(label, font, 13);
    t.setFillColor(Theme::TextMain);
    sf::FloatRect tb = t.getLocalBounds();
    t.setPosition(x + w / 2.f - tb.width / 2.f, y + h / 2.f - 10.f);
    window.draw(t);
}

void PlayState::drawHud(sf::RenderWindow& window) {
    sf::RectangleShape bar({1280.f, HUD_H});
    bar.setFillColor(Theme::Panel);
    bar.setOutlineThickness(1.f);
    bar.setOutlineColor(sf::Color(0, 90, 120));
    window.draw(bar);

    if (m_tutorial) m_txtWave.setString("SAMOUCZEK");
    else m_txtWave.setString("FALA  " + std::to_string(m_wave) + "/" + std::to_string(m_totalWaves));

    m_txtCredits.setString("KREDYTY  " + std::to_string(m_credits));
    m_txtHealth.setString("SERWER  " + std::to_string(m_serverHealth));
    m_txtScore.setString("WYNIK  " + std::to_string(m_score));

    m_txtWave.setPosition(24.f, 16.f);
    m_txtCredits.setPosition(190.f, 16.f);
    m_txtHealth.setPosition(400.f, 16.f);
    m_txtScore.setPosition(560.f, 16.f);
    window.draw(m_txtWave);
    window.draw(m_txtCredits);
    window.draw(m_txtHealth);
    window.draw(m_txtScore);

    float cpuFrac = m_cpuCapacity > 0 ? static_cast<float>(m_cpuUsage) / m_cpuCapacity : 0.f;
    sf::Color cpuCol = cpuFrac < 0.7f ? Theme::NeonGreen
                                      : (cpuFrac < 0.9f ? Theme::Warn : Theme::Danger);
    drawBar(window, 770.f, 16.f, 200.f, 22.f, cpuFrac, cpuCol,
            "CPU  " + std::to_string(m_cpuUsage) + "/" + std::to_string(m_cpuCapacity));

    float heatFrac = m_heat / 100.f;
    bool frozen = m_heatFreezeTimer > 0.f;
    sf::Color heatCol = frozen ? Theme::NeonCyan
                               : Theme::lerp(Theme::NeonGreen, Theme::Danger, heatFrac);
    char heatLbl[28];
    if (frozen) std::snprintf(heatLbl, sizeof(heatLbl), "TEMP  %d%% (COOLANT)", static_cast<int>(m_heat));
    else        std::snprintf(heatLbl, sizeof(heatLbl), "TEMP  %d%%", static_cast<int>(m_heat));
    drawBar(window, 1000.f, 16.f, 200.f, 22.f, heatFrac, heatCol, heatLbl);

    if (m_waves && m_waves->inCooldown() && !m_draftActive && !m_tutorial) {
        const sf::Font& font = m_game.getResources().getFont();
        int secs = static_cast<int>(std::ceil(m_waves->cooldownRemaining()));
        sf::Text t("NASTEPNA FALA ZA " + std::to_string(secs) + "s", font, 18);
        t.setFillColor(Theme::Warn);
        sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(640.f - tb.width / 2.f, 66.f);
        window.draw(t);
    }

    m_txtHint.setPosition(24.f, SHOP_Y - 22.f);
    window.draw(m_txtHint);
}

sf::FloatRect PlayState::shopSlotRect(int i) const {
    const float w = 106.f, h = 54.f, gap = 6.f, x0 = 12.f;
    return sf::FloatRect(x0 + i * (w + gap), SHOP_Y + 9.f, w, h);
}

void PlayState::drawShop(sf::RenderWindow& window) {
    sf::RectangleShape bar({1280.f, SHOP_H});
    bar.setPosition(0.f, SHOP_Y);
    bar.setFillColor(Theme::Panel);
    bar.setOutlineThickness(1.f);
    bar.setOutlineColor(sf::Color(0, 90, 120));
    window.draw(bar);

    const sf::Font& font = m_game.getResources().getFont();
    auto& cfg = m_game.getConfig();

    for (int i = 0; i < static_cast<int>(TowerType::Count); ++i) {
        const TowerMeta& meta = towerMeta(static_cast<TowerType>(i));
        sf::FloatRect r = shopSlotRect(i);
        bool unlocked = m_towerUnlocked[i];
        bool selected = (m_buildSelection == i);
        int cost = cfg.getInt(meta.configCostKey, meta.defaultCost) * m_towerCostPercent / 100;
        bool afford = m_credits >= cost;

        sf::RectangleShape slot({r.width, r.height});
        slot.setPosition(r.left, r.top);
        slot.setFillColor(selected ? sf::Color(meta.color.r, meta.color.g, meta.color.b, 55)
                                   : Theme::PanelSolid);
        slot.setOutlineThickness(selected ? 3.f : 2.f);
        slot.setOutlineColor(unlocked ? meta.color : sf::Color(70, 80, 94));
        window.draw(slot);

        sf::CircleShape dot(7.f);
        dot.setOrigin(7.f, 7.f);
        dot.setPosition(r.left + 15.f, r.top + 16.f);
        dot.setFillColor(unlocked ? meta.color : sf::Color(70, 80, 94));
        window.draw(dot);

        sf::Text name(meta.shortName, font, 14);
        name.setFillColor(unlocked ? Theme::TextMain : Theme::TextDim);
        name.setPosition(r.left + 28.f, r.top + 7.f);
        window.draw(name);

        if (unlocked) {
            sf::Text info("$" + std::to_string(cost), font, 12);
            info.setFillColor(afford ? Theme::NeonGreen : Theme::Danger);
            info.setPosition(r.left + 10.f, r.top + 33.f);
            window.draw(info);
        } else {
            sf::Text lock("ZABLOKOWANE", font, 11);
            lock.setFillColor(Theme::TextDim);
            lock.setPosition(r.left + 10.f, r.top + 34.f);
            window.draw(lock);
        }

        sf::Text key(std::to_string(i + 1), font, 12);
        key.setFillColor(Theme::TextDim);
        key.setPosition(r.left + r.width - 13.f, r.top + 3.f);
        window.draw(key);

        if (m_tutorial && i == m_tutOnlySlot) {
            float a = 120.f + 120.f * std::sin(m_uiTime * 5.f);
            sf::RectangleShape hl({r.width + 6.f, r.height + 6.f});
            hl.setPosition(r.left - 3.f, r.top - 3.f);
            hl.setFillColor(sf::Color::Transparent);
            hl.setOutlineThickness(3.f);
            hl.setOutlineColor(sf::Color(57, 255, 136, static_cast<sf::Uint8>(a)));
            window.draw(hl);
        }
    }
}

void PlayState::drawTowerPanel(sf::RenderWindow& window) {
    if (!m_selectedTower) return;
    Tower* t = m_selectedTower;
    const sf::Font& font = m_game.getResources().getFont();

    sf::RectangleShape panel({PANEL_W, PANEL_H});
    panel.setPosition(PANEL_X, PANEL_Y);
    panel.setFillColor(Theme::Panel);
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(t->getColor());
    window.draw(panel);

    auto line = [&](const std::string& s, float y, unsigned size, sf::Color c) {
        sf::Text txt(s, font, size);
        txt.setFillColor(c);
        txt.setPosition(PANEL_X + 14.f, PANEL_Y + y);
        window.draw(txt);
    };

    line(t->displayName(), 12.f, 20, t->getColor());
    line("Poziom: " + std::to_string(t->getLevel()) + " / 3", 44.f, 16, Theme::TextMain);

    float yy = 70.f;
    if (t->getDamage() >= 1.f) {
        line("Obrazenia: " + std::to_string(static_cast<int>(t->getDamage())), yy, 15, Theme::TextDim);
        yy += 22.f;
    }
    line("Zasieg: " + std::to_string(static_cast<int>(t->getRange())), yy, 15, Theme::TextDim);
    yy += 22.f;
    if (t->canShoot()) {
        char rate[32]; std::snprintf(rate, sizeof(rate), "Szybkostrzelnosc: %.2f/s", t->getFireRate());
        line(rate, yy, 15, Theme::TextDim);
        yy += 22.f;
    }
    line("CPU: " + std::to_string(t->getCpuCost()), yy, 15, Theme::TextDim);
    yy += 22.f;
    std::string extra = t->statLine();
    if (!extra.empty()) line(extra, yy, 16, t->getColor());

    if (t->canUpgrade())
        m_btnUpgrade.setLabel("ULEPSZ  ($" + std::to_string(t->getUpgradeCost()) + ")");
    else
        m_btnUpgrade.setLabel("POZIOM MAKS.");
    m_btnSell.setLabel("SPRZEDAJ  (+$" + std::to_string(t->getSellValue()) + ")");
    m_btnUpgrade.draw(window);
    m_btnSell.draw(window);
}

void PlayState::drawMessage(sf::RenderWindow& window) {
    if (m_messageTimer <= 0.f) return;
    const sf::Font& font = m_game.getResources().getFont();
    sf::Text t(m_message, font, 22);
    float alpha = std::min(1.f, m_messageTimer / 0.5f);
    sf::Color c = m_messageColor;
    c.a = static_cast<sf::Uint8>(255 * alpha);
    t.setFillColor(c);
    t.setStyle(sf::Text::Bold);
    sf::FloatRect tb = t.getLocalBounds();
    t.setPosition(640.f - tb.width / 2.f, 90.f);
    window.draw(t);
}

void PlayState::drawTutorialHints(sf::RenderWindow& window) {
    (void)window;
}

void PlayState::draw(sf::RenderWindow& window) {
    drawBackground(window);
    drawPaths(window);

    for (auto& o : m_objects) {
        if (o.get() == static_cast<GameObject*>(m_server)) continue;
        if (Tower* t = dynamic_cast<Tower*>(o.get())) t->drawBaseLayer(window);
        else o->draw(window);
    }
    for (auto& o : m_objects)
        if (Tower* t = dynamic_cast<Tower*>(o.get())) t->drawHeadLayer(window);
    if (m_server) m_server->draw(window);

    drawBuildPreview(window);
    drawTutorialHints(window);

    drawHud(window);
    drawShop(window);
    drawControls(window);
    drawAbilities(window);
    drawTowerPanel(window);
    drawMessage(window);
    if (m_paused) drawPauseOverlay(window);
    if (m_draftActive) drawDraft(window);
    if (m_tutorial && m_tutorialDirector) m_tutorialDirector->draw(window);
}

void PlayState::drawControls(sf::RenderWindow& window) {
    m_btnSpeed.draw(window);
    m_btnSaveGame.draw(window);
    m_btnMenu.draw(window);
}

void PlayState::drawAbilities(sf::RenderWindow& window) {
    if (!m_abilitiesEnabled) return;
    const sf::Font& font = m_game.getResources().getFont();

    sf::Text hdr("MOCE", font, 13);
    hdr.setFillColor(Theme::TextDim);
    hdr.setPosition(m_abilities.empty() ? 1210.f : m_abilities[0].pos.x - 22.f, 64.f);
    window.draw(hdr);

    const char* keys[2] = {"Q", "W"};
    for (int i = 0; i < static_cast<int>(m_abilities.size()); ++i) {
        const Ability& a = m_abilities[i];
        bool ready = a.cooldown <= 0.f;
        float pulse = ready ? 1.f + 0.06f * std::sin(m_uiTime * 4.f) : 1.f;

        if (ready) {
            float gr = (a.radius + 6.f) * pulse;
            sf::CircleShape glow(gr);
            glow.setOrigin(gr, gr); glow.setPosition(a.pos);
            glow.setFillColor(sf::Color(a.color.r, a.color.g, a.color.b, 55));
            window.draw(glow);
        }

        sf::Color body = ready ? a.color : sf::Color(a.color.r / 3, a.color.g / 3, a.color.b / 3);
        sf::CircleShape c(a.radius, 6);
        c.setOrigin(a.radius, a.radius); c.setPosition(a.pos);
        c.setRotation(ready ? m_uiTime * 18.f : 0.f);
        c.setFillColor(sf::Color(body.r, body.g, body.b, 60));
        c.setOutlineThickness(2.5f); c.setOutlineColor(body);
        window.draw(c);

        if (ready) {
            sf::CircleShape core(a.radius * 0.4f);
            core.setOrigin(a.radius * 0.4f, a.radius * 0.4f); core.setPosition(a.pos);
            core.setFillColor(body);
            window.draw(core);
        } else {
            sf::Text cd(std::to_string(static_cast<int>(std::ceil(a.cooldown))), font, 20);
            cd.setFillColor(Theme::TextMain);
            sf::FloatRect b = cd.getLocalBounds();
            cd.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            cd.setPosition(a.pos);
            window.draw(cd);
        }

        sf::Text name(a.name, font, 11);
        name.setFillColor(ready ? a.color : Theme::TextDim);
        sf::FloatRect nb = name.getLocalBounds();
        name.setOrigin(nb.left + nb.width / 2.f, 0.f);
        name.setPosition(a.pos.x, a.pos.y + a.radius + 2.f);
        window.draw(name);

        sf::Text key(keys[i % 2], font, 12);
        key.setFillColor(Theme::TextDim);
        key.setPosition(a.pos.x - a.radius - 4.f, a.pos.y - a.radius - 4.f);
        window.draw(key);
    }
}

void PlayState::drawBuildPreview(sf::RenderWindow& window) {
    if (m_buildSelection < 0 || !m_preview) return;
    if (m_tutorial && m_tutBuildLocked) return;
    sf::Vector2f p = m_mouse;
    if (p.y >= SHOP_Y) return;
    bool ok = canPlaceAt(p);

    sf::Color col = ok ? Theme::NeonGreen : Theme::Danger;
    float rr = m_preview->effectiveRange();
    sf::CircleShape range(rr);
    range.setOrigin(rr, rr); range.setPosition(p);
    range.setFillColor(sf::Color(col.r, col.g, col.b, 24));
    range.setOutlineThickness(2.f);
    range.setOutlineColor(sf::Color(col.r, col.g, col.b, 150));
    window.draw(range);

    sf::CircleShape foot(kTowerFootprint);
    foot.setOrigin(kTowerFootprint, kTowerFootprint); foot.setPosition(p);
    foot.setFillColor(sf::Color(col.r, col.g, col.b, 60));
    foot.setOutlineThickness(2.f);
    foot.setOutlineColor(col);
    window.draw(foot);

    sf::CircleShape body(13.f, 8);
    body.setOrigin(13.f, 13.f); body.setPosition(p);
    body.setRotation(22.5f);
    sf::Color tc = m_preview->getColor();
    body.setFillColor(sf::Color(tc.r, tc.g, tc.b, 150));
    window.draw(body);
}

void PlayState::drawPauseOverlay(sf::RenderWindow& window) {
    sf::RectangleShape dim({1280.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    const sf::Font& font = m_game.getResources().getFont();
    sf::Text t("PAUZA", font, 72);
    t.setStyle(sf::Text::Bold);
    t.setFillColor(Theme::NeonCyan);
    sf::FloatRect tb = t.getLocalBounds();
    t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    t.setPosition(640.f, 330.f);
    window.draw(t);

    const sf::Font& body = m_game.getResources().getFont();
    sf::Text hint("SPACE - wznow", body, 24);
    hint.setFillColor(Theme::TextDim);
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
    hint.setPosition(640.f, 400.f);
    window.draw(hint);
}