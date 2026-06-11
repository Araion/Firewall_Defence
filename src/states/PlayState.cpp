#include "states/PlayState.h"
#include "core/Game.h"
#include "states/MainMenuState.h"
#include "states/GameOverState.h"
#include "entities/ServerCore.h"
#include "entities/Enemy.h"
#include "entities/VirusEnemy.h"
#include "entities/TrojanEnemy.h"
#include "entities/WormEnemy.h"
#include "entities/ProxyEnemy.h"
#include "entities/GlitchDroneEnemy.h"
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
#include "managers/ResourceManager.h"
#include "managers/ConfigManager.h"
#include "managers/AudioManager.h"
#include "managers/SaveManager.h"
#include "util/MapFactory.h"
#include "util/MathUtils.h"
#include "util/Rng.h"
#include "util/Theme.h"
#include <memory>
#include <algorithm>
#include <string>
#include <cmath>

namespace {
// Mapowanie nazwy typu wiezy (getTypeName) na TowerType (do wczytania zapisu)
bool towerTypeFromName(const std::string& n, TowerType& out) {
    if (n == "AntivirusTower")        out = TowerType::Antivirus;
    else if (n == "FirewallTower")    out = TowerType::Firewall;
    else if (n == "LaserTower")       out = TowerType::Laser;
    else if (n == "DataCleanerTower") out = TowerType::DataCleaner;
    else if (n == "OverclockTower")   out = TowerType::Overclock;
    else if (n == "CorruptionTower")  out = TowerType::Corruption;
    else if (n == "EMPTower")         out = TowerType::EMP;
    else return false;
    return true;
}
}

PlayState::PlayState(Game& game) : GameState(game) {
    m_serverMaxHealth = m_game.getConfig().getInt("serverHealth", 20);
    m_serverHealth = m_serverMaxHealth;
    m_credits = m_game.getConfig().getInt("startCredits", 300);
    m_difficulty = m_game.getConfig().getInt("difficulty", 1);

    // Losowe ziarno mapy (zapamietane, by zapis/wczyt odtworzyl ten sam uklad)
    unsigned seed = static_cast<unsigned>(Rng::range(1, 1000000));
    buildBoard(seed, m_difficulty);

    // Przyciski panelu zaznaczonej wiezy
    const sf::Font& font = m_game.getResources().getFont();

    m_btnUpgrade.setup(font, "ULEPSZ", {16.f, kHudH + 200.f}, {184.f, 32.f}, 17);
    m_btnUpgrade.setColors(Theme::PanelSolid, Theme::NeonCyan, Theme::TextMain, Theme::NeonCyan);

    m_btnSell.setup(font, "SPRZEDAJ", {16.f, kHudH + 238.f}, {184.f, 32.f}, 17);
    m_btnSell.setColors(Theme::PanelSolid, Theme::Danger, Theme::TextMain, Theme::Danger);

    playMusic("music_game");
}

PlayState::~PlayState() = default;

void PlayState::playMusic(const std::string& name) {
    m_game.getAudio().playMusic(name);
}

void PlayState::buildBoard(unsigned seed, int difficulty) {
    // Czysci poprzedni uklad (istotne przy wczytywaniu gry)
    m_objects.clear();
    m_pendingSpawns.clear();
    m_paths.clear();
    m_frameEnemies.clear();
    m_frameTowers.clear();
    m_server = nullptr;
    m_selectedTower = nullptr;
    m_preview.reset();
    m_buildSelection = -1;

    m_mapSeed = seed;
    m_difficulty = difficulty;

    // Ładujemy mapę i jej ścieżki
    m_levelMap = MapFactory::generate(difficulty, seed);

    for (const auto& lane : m_levelMap.lanes) {
        m_paths.push_back(std::make_unique<Path>(lane));
    }

    // Tworzymy serwer na pozycji określonej przez mapę
    auto server = std::make_unique<ServerCore>(m_game.getResources(), m_levelMap.serverPos);
    server->setHealth(m_serverHealth, m_serverMaxHealth);
    m_server = server.get();
    m_objects.push_back(std::move(server));
}

std::unique_ptr<Enemy> PlayState::createEnemyByName(const std::string& name, const Path* path) {
    auto& res = m_game.getResources();
    if (name == "VirusEnemy")        return std::make_unique<VirusEnemy>(res, path);
    if (name == "TrojanEnemy")       return std::make_unique<TrojanEnemy>(res, path);
    if (name == "WormEnemy")         return std::make_unique<WormEnemy>(res, path);
    if (name == "ProxyEnemy")        return std::make_unique<ProxyEnemy>(res, path);
    if (name == "GlitchDroneEnemy")  return std::make_unique<GlitchDroneEnemy>(res, path);
    if (name == "BossMalwareEnemy")  return std::make_unique<BossMalwareEnemy>(res, path);
    return nullptr;
}

void PlayState::spawnEnemy() {
    if (m_paths.empty()) return;
    const Path* path = m_paths[Rng::range(0, static_cast<int>(m_paths.size()) - 1)].get();

    static const char* types[] = {
        "VirusEnemy", "TrojanEnemy", "WormEnemy", "ProxyEnemy", "GlitchDroneEnemy", "BossMalwareEnemy"
    };
    auto enemy = createEnemyByName(types[Rng::range(0, 5)], path);
    if (!enemy) return;

    // Czesc wrogow jest zaszyfrowana - niewidoczna dla Antivirus/Laser do wykrycia
    if (Rng::chance(0.30f)) enemy->setEncrypted(true);

    m_objects.push_back(std::move(enemy));
}

// =============================================================
// API dla obiektow gry
// =============================================================

void PlayState::spawn(std::unique_ptr<GameObject> obj) {
    m_pendingSpawns.push_back(std::move(obj));
}

void PlayState::flushPending() {
    // Dodaje obiekty utworzone w trakcie aktualnej klatki
    for (auto& object : m_pendingSpawns)
        m_objects.push_back(std::move(object));

    m_pendingSpawns.clear();
}

ResourceManager& PlayState::resources() {
    return m_game.getResources();
}

void PlayState::spawnExplosion(sf::Vector2f pos, sf::Color color, float scale) {
    spawn(std::make_unique<ExplosionEffect>(pos, color, scale));
}

// =============================================================
// Zapis / wczytanie stanu gry
// =============================================================

std::vector<PlayState::TowerSave> PlayState::snapshotTowers() const {
    std::vector<TowerSave> out;

    for (const auto& object : m_objects) {
        if (Tower* tower = dynamic_cast<Tower*>(object.get())) {
            sf::Vector2f p = tower->getPosition();
            out.push_back({tower->getTypeName(), tower->getLevel(), p.x, p.y});
        }
    }

    return out;
}

int PlayState::pathIndexOf(const Path* p) const {
    for (int i = 0; i < static_cast<int>(m_paths.size()); ++i)
        if (m_paths[i].get() == p) return i;
    return 0;
}

std::vector<PlayState::EnemySave> PlayState::snapshotEnemies() const {
    std::vector<EnemySave> out;

    for (const auto& object : m_objects) {
        if (Enemy* enemy = dynamic_cast<Enemy*>(object.get())) {
            if (!enemy->isAlive() || enemy->reachedServer())
                continue;

            out.push_back({enemy->getTypeName(), enemy->getHp(),
                           pathIndexOf(enemy->getPath()), enemy->getDistance()});
        }
    }

    return out;
}

void PlayState::applyLoadedMeta(int credits, int serverHp, int score) {
    m_credits = credits;
    m_score = score;
    m_serverHealth = serverHp;

    if (m_server)
        m_server->setHealth(m_serverHealth, m_serverMaxHealth);
}

void PlayState::placeTowerFromSave(const std::string& type, int level, sf::Vector2f pos) {
    TowerType tt;

    if (!towerTypeFromName(type, tt))
        return;

    auto tower = createTower(tt);

    if (!tower)
        return;

    tower->setLevelDirect(level);
    tower->setPosition(pos);
    m_objects.push_back(std::move(tower));
}

void PlayState::addEnemyFromSave(const std::string& type, float hp, int pathIndex, float distance) {
    if (pathIndex < 0 || pathIndex >= static_cast<int>(m_paths.size()))
        return;

    auto enemy = createEnemyByName(type, m_paths[pathIndex].get());

    if (!enemy)
        return;

    enemy->setHp(hp);
    enemy->setDistance(distance);
    m_objects.push_back(std::move(enemy));
}

void PlayState::finishLoad() {
    rebuildCaches();
}

bool PlayState::saveGame() {
    SaveManager sm;
    return sm.save(*this);
}

bool PlayState::loadGame() {
    SaveManager sm;
    return sm.load(*this);
}

// =============================================================
// Budowanie i wybor wiez
// =============================================================

std::unique_ptr<Tower> PlayState::createTower(TowerType type) {
    auto& res = m_game.getResources();
    auto& cfg = m_game.getConfig();
    switch (type) {
    case TowerType::Antivirus:   return std::make_unique<AntivirusTower>(*this, res, cfg);
    case TowerType::Firewall:    return std::make_unique<FirewallTower>(*this, res, cfg);
    case TowerType::Laser:       return std::make_unique<LaserTower>(*this, res, cfg);
    case TowerType::DataCleaner: return std::make_unique<DataCleanerTower>(*this, res, cfg);
    case TowerType::Overclock:   return std::make_unique<OverclockTower>(*this, res, cfg);
    case TowerType::Corruption:  return std::make_unique<CorruptionTower>(*this, res, cfg);
    case TowerType::EMP:         return std::make_unique<EMPTower>(*this, res, cfg);
    default:                     return nullptr;
    }
}

bool PlayState::canPlaceAt(sf::Vector2f pos) const {
    const float radius = kTowerFootprint;

    // Sprawdza, czy pozycja miesci sie w obszarze budowania
    if (pos.x < 24.f + radius || pos.x > 1180.f ||
        pos.y < kHudH + 18.f || pos.y > kShopY - 18.f) {
        return false;
    }

    // Blokuje budowanie zbyt blisko rdzenia serwera
    if (m_server && MathUtils::distance(pos, m_server->getPosition()) < 70.f + radius)
        return false;

    // Blokuje budowanie na sciezkach przeciwnikow
    const float pathClearance = 15.f + radius + 2.f;

    for (auto& path : m_paths) {
        const auto& points = path->points();

        for (size_t i = 1; i < points.size(); ++i) {
            if (MathUtils::pointSegmentDistance(pos, points[i - 1], points[i]) < pathClearance)
                return false;
        }
    }

    // Blokuje nakladanie wiez na siebie
    for (Tower* tower : m_frameTowers) {
        if (MathUtils::distance(pos, tower->getPosition()) < 2.f * radius)
            return false;
    }

    return true;
}

bool PlayState::tryBuildAt(TowerType type, sf::Vector2f pos) {
    if (!canPlaceAt(pos))
        return false;

    auto tower = createTower(type);

    if (!tower)
        return false;

    if (m_credits < tower->getCost())
        return false;

    m_credits -= tower->getCost();

    tower->setPosition(pos);
    m_objects.push_back(std::move(tower));

    return true;
}

Tower* PlayState::towerAt(sf::Vector2f pos) const {
    for (Tower* tower : m_frameTowers) {
        if (tower->getBounds().contains(pos))
            return tower;
    }

    return nullptr;
}

void PlayState::setBuildSelection(int idx) {
    deselectAll();

    m_buildSelection = idx;

    // Tworzy podglad wybranej wiezy
    if (idx >= 0 && idx < static_cast<int>(TowerType::Count))
        m_preview = createTower(static_cast<TowerType>(idx));
    else
        m_preview.reset();
}

void PlayState::selectTower(Tower* tower) {
    deselectAll();

    m_selectedTower = tower;

    if (tower)
        tower->setSelected(true);
}

void PlayState::deselectAll() {
    if (m_selectedTower)
        m_selectedTower->setSelected(false);

    m_selectedTower = nullptr;
}

void PlayState::sellSelected() {
    if (!m_selectedTower)
        return;

    m_credits += m_selectedTower->getSellValue();

    m_selectedTower->kill();
    m_selectedTower = nullptr;
}

void PlayState::upgradeSelected() {
    if (!m_selectedTower || !m_selectedTower->canUpgrade())
        return;

    int cost = m_selectedTower->getUpgradeCost();

    if (m_credits < cost)
        return;

    m_credits -= cost;
    m_selectedTower->upgrade();
}

void PlayState::onLeftClick(sf::Vector2f mouse) {
    // Obsluga przyciskow panelu zaznaczonej wiezy
    if (m_selectedTower) {
        if (m_btnUpgrade.contains(mouse)) {
            upgradeSelected();
            return;
        }

        if (m_btnSell.contains(mouse)) {
            sellSelected();
            return;
        }
    }

    // Obsluga slotow sklepu
    for (int i = 0; i < static_cast<int>(TowerType::Count); ++i) {
        if (shopSlotRect(i).contains(mouse)) {
            setBuildSelection(m_buildSelection == i ? -1 : i);
            return;
        }
    }

    // Budowanie aktualnie wybranej wiezy
    if (m_buildSelection >= 0) {
        tryBuildAt(static_cast<TowerType>(m_buildSelection), mouse);
        return;
    }

    // Wybor postawionej wiezy albo zamkniecie panelu
    if (Tower* tower = towerAt(mouse))
        selectTower(tower);
    else
        deselectAll();
}

void PlayState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Escape) {
            if (m_buildSelection >= 0) {
                setBuildSelection(-1);
                return;
            }

            m_game.changeState(std::make_unique<MainMenuState>(m_game));
            return;
        }

        if (e.key.code == sf::Keyboard::F5) { saveGame(); return; } // szybki zapis
        if (e.key.code == sf::Keyboard::F9) { loadGame(); return; } // wczytanie

        // Klawisze 1-7 wybieraja typ wiezy do budowy
        if (e.key.code >= sf::Keyboard::Num1 && e.key.code <= sf::Keyboard::Num4) {
            int idx = e.key.code - sf::Keyboard::Num1;

            setBuildSelection(m_buildSelection == idx ? -1 : idx);
            return;
        }
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        onLeftClick(sf::Vector2f(
            static_cast<float>(e.mouseButton.x),
            static_cast<float>(e.mouseButton.y)
            ));
    }
}

// =============================================================
// Petla rozgrywki
// =============================================================

void PlayState::rebuildCaches() {
    m_frameEnemies.clear();
    m_frameTowers.clear();

    // Odswieza listy pomocnicze uzywane przez wieze i kolizje
    for (const auto& object : m_objects) {
        if (!object->isAlive())
            continue;

        if (Enemy* enemy = dynamic_cast<Enemy*>(object.get()))
            m_frameEnemies.push_back(enemy);
        else if (Tower* tower = dynamic_cast<Tower*>(object.get())) {
            tower->resetFireRateBoost(); // boost aury Overclocka naliczany co klatke
            m_frameTowers.push_back(tower);
        }
    }
}

void PlayState::handleDeaths() {
    for (auto& o : m_objects) {
        if (o->isAlive())
            continue;

        if (Enemy* e = dynamic_cast<Enemy*>(o.get())) {
            if (e->reachedServer()) {
                if (m_server && !m_server->isDestroyed()) {
                    m_server->takeDamage(e->getServerDamage());
                    m_serverHealth = m_server->getHealth();
                    m_game.getAudio().play("server_hit", 90.f);
                }
            } else {
                // Przyznaje nagrode za pokonanego przeciwnika
                m_credits += e->getReward();
                m_score += e->getPoints();

                spawnExplosion(e->getPosition(), e->getBodyColor());
            }
        }
    }
}

void PlayState::removeDead() {
    // Usuwa "martwe" obiekty (w tym wrogów, którzy weszli w serwer lub stracili całe HP)
    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
                       [](const std::unique_ptr<GameObject>& o) { return !o->isAlive(); }),
        m_objects.end());
}

void PlayState::update(float dt) {
    m_time += dt;
    m_uiTime += dt;

    sf::Vector2i mousePos = sf::Mouse::getPosition(m_game.getWindow());
    m_mouse = sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (m_selectedTower) {
        m_btnUpgrade.update(dt, m_mouse);
        m_btnSell.update(dt, m_mouse);
    }

    // prosta petla do sprawdzania przeciwnikow (tymczasowa)
    m_spawnTimer += dt;

    if (m_spawnTimer >= m_nextSpawnDelay && !m_paths.empty()) {
        m_spawnTimer = 0.f;
        m_nextSpawnDelay = Rng::rangef(0.5f, 2.0f);

        spawnEnemy();
    }

    rebuildCaches();
    // Aura Overclocka przed aktualizacja wiez
    for (Tower* t : m_frameTowers) t->applyAura(m_frameTowers);

    for (auto& o : m_objects) {
        o->update(dt);
    }

    m_collision.resolve(*this, m_objects);

    handleDeaths();
    removeDead();
    flushPending();

    // Serwer zniszczony - ekran konca gry (z wynikiem do tablicy)
    if (m_serverHealth <= 0)
        m_game.changeState(std::make_unique<GameOverState>(m_game, m_score, 0, m_difficulty));
}

// =============================================================
// Rysowanie
// =============================================================

void PlayState::draw(sf::RenderWindow& window) {
    drawBackground(window);
    drawPaths(window);

    // Rysuje najpierw podstawy wiez, a potem glowki
    for (auto& object : m_objects) {
        if (Tower* tower = dynamic_cast<Tower*>(object.get()))
            tower->drawBaseLayer(window);
        else
            object->draw(window);
    }

    for (auto& object : m_objects) {
        if (Tower* tower = dynamic_cast<Tower*>(object.get()))
            tower->drawHeadLayer(window);
    }

    drawBuildPreview(window);
    drawShop(window);
    drawHud(window);
    drawTowerPanel(window);
}

void PlayState::drawBackground(sf::RenderWindow& window) {
    window.clear(Theme::Background);

    // Rysuje siatke w tle planszy
    sf::VertexArray grid(sf::Lines);
    const int spacing = 48.f;
    sf::Color line = Theme::GridLine;

    for (int x = 0.f; x <= 1280.f; x += spacing) {
        grid.append(sf::Vertex({static_cast<float>(x), 0.f}, line));
        grid.append(sf::Vertex({static_cast<float>(x), 720.f}, line));
    }

    for (int y = 0.f; y <= 720.f; y += spacing) {
        grid.append(sf::Vertex({0.f, static_cast<float>(y)}, line));
        grid.append(sf::Vertex({1280.f, static_cast<float>(y)}, line));
    }

    window.draw(grid);
}

void PlayState::drawPaths(sf::RenderWindow& window) {
    const sf::Color darkBase(22, 34, 52);
    const sf::Color lineBase(0, 120, 150);

    auto drawBacking = [&](const std::vector<sf::Vector2f>& pts, sf::Color dark) {
        for (size_t i = 1; i < pts.size(); ++i) {
            sf::Vector2f d = pts[i] - pts[i - 1];
            float len = std::sqrt(d.x * d.x + d.y * d.y);
            if (len < 1e-3f) continue;

            sf::RectangleShape rect({len, 30.f});
            rect.setOrigin(0.f, 15.f);
            rect.setPosition(pts[i - 1]);
            rect.setRotation(std::atan2(d.y, d.x) * 180.f / 3.14159f);
            rect.setFillColor(dark);
            window.draw(rect);
        }
        for (const auto& p : pts) {
            sf::CircleShape joint(15.f);
            joint.setOrigin(15.f, 15.f);
            joint.setPosition(p);
            joint.setFillColor(dark);
            window.draw(joint);
        }
    };

    auto drawCenter = [&](const std::vector<sf::Vector2f>& pts, sf::Color line) {
        for (size_t i = 1; i < pts.size(); ++i) {
            sf::Vector2f d = pts[i] - pts[i - 1];
            float len = std::sqrt(d.x * d.x + d.y * d.y);
            if (len < 1e-3f) continue;

            sf::RectangleShape rect({len, 3.f});
            rect.setOrigin(0.f, 1.5f);
            rect.setPosition(pts[i - 1]);
            rect.setRotation(std::atan2(d.y, d.x) * 180.f / 3.14159f);
            rect.setFillColor(line);
            window.draw(rect);
        }
    };

    // Rysowanie podkładów torów i osi
    for (auto& path : m_paths) drawBacking(path->points(), darkBase);
    for (auto& path : m_paths) drawCenter(path->points(), lineBase);
}

void PlayState::drawBuildPreview(sf::RenderWindow& window) {
    if (m_buildSelection < 0 || !m_preview)
        return;

    bool canBuild = canPlaceAt(m_mouse);
    sf::Color color = canBuild ? Theme::Ok : Theme::Danger;

    // Rysuje zasieg wiezy przed jej postawieniem
    float rangeRadius = m_preview->getRange();

    sf::CircleShape range(rangeRadius);
    range.setOrigin(rangeRadius, rangeRadius);
    range.setPosition(m_mouse);
    range.setFillColor(sf::Color(color.r, color.g, color.b, 26));
    range.setOutlineThickness(2.f);
    range.setOutlineColor(sf::Color(color.r, color.g, color.b, 150));

    window.draw(range);

    // Rysuje miejsce postawienia wiezy
    sf::CircleShape footprint(kTowerFootprint);
    footprint.setOrigin(kTowerFootprint, kTowerFootprint);
    footprint.setPosition(m_mouse);
    footprint.setFillColor(sf::Color(color.r, color.g, color.b, 90));

    window.draw(footprint);
}

sf::FloatRect PlayState::shopSlotRect(int i) const {
    const int count = static_cast<int>(TowerType::Count);
    const float margin = 12.f;
    const float gap = 8.f;
    const float height = 60.f;
    const float width = (1280.f - 2.f * margin - gap * (count - 1)) / count;

    return sf::FloatRect(
        margin + i * (width + gap),
        kShopY + 6.f,
        width,
        height
        );
}

void PlayState::drawShop(sf::RenderWindow& window) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::RectangleShape bar({1280.f, 720.f - kShopY});
    bar.setPosition(0.f, kShopY);
    bar.setFillColor(Theme::PanelSolid);

    window.draw(bar);

    // Rysuje sloty sklepu z dostepnymi wiezami
    for (int i = 0; i < static_cast<int>(TowerType::Count); ++i) {
        const TowerMeta& meta = towerMeta(static_cast<TowerType>(i));
        sf::FloatRect rect = shopSlotRect(i);

        sf::RectangleShape slot({rect.width, rect.height});
        slot.setPosition(rect.left, rect.top);
        slot.setFillColor(sf::Color(18, 24, 38));
        slot.setOutlineThickness(2.f);
        slot.setOutlineColor(i == m_buildSelection ? Theme::NeonCyan : sf::Color(40, 60, 86));

        window.draw(slot);

        sf::Text key(std::to_string(i + 1), font, 12);
        key.setFillColor(Theme::TextDim);
        key.setPosition(rect.left + 5.f, rect.top + 3.f);

        window.draw(key);

        sf::Text name(meta.shortName, font, 14);
        name.setFillColor(meta.color);

        sf::FloatRect nameBounds = name.getLocalBounds();
        name.setPosition(rect.left + rect.width / 2.f - nameBounds.width / 2.f, rect.top + 12.f);

        window.draw(name);

        int cost = m_game.getConfig().getInt(meta.configCostKey, meta.defaultCost);

        sf::Text price("$" + std::to_string(cost), font, 13);
        price.setFillColor(m_credits >= cost ? Theme::NeonGreen : Theme::Danger);

        sf::FloatRect priceBounds = price.getLocalBounds();
        price.setPosition(rect.left + rect.width / 2.f - priceBounds.width / 2.f, rect.top + 36.f);

        window.draw(price);
    }
}

void PlayState::drawHud(sf::RenderWindow& window) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::RectangleShape bar({1280.f, kHudH});
    bar.setFillColor(Theme::PanelSolid);

    window.draw(bar);

    auto label = [&](const std::string& text, float x, sf::Color color) {
        sf::Text labelText(text, font, 20);
        labelText.setFillColor(color);
        labelText.setPosition(x, 13.f);

        window.draw(labelText);
    };

    label("KREDYTY  " + std::to_string(m_credits), 24.f, Theme::NeonGreen);
    label("WYNIK  " + std::to_string(m_score), 330.f, Theme::TextMain);
    label("SERWER  " + std::to_string(m_serverHealth) + "/" + std::to_string(m_serverMaxHealth),
          560.f, Theme::NeonCyan);
    label("1-7: wieza   F5/F9: zapis/wczyt   ESC: menu", 900.f, Theme::TextDim);
}

void PlayState::drawTowerPanel(sf::RenderWindow& window) {
    if (!m_selectedTower)
        return;

    Tower* tower = m_selectedTower;
    const sf::Font& font = m_game.getResources().getFont();

    sf::RectangleShape panel({200.f, 264.f});
    panel.setPosition(8.f, kHudH + 8.f);
    panel.setFillColor(Theme::Panel);
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(tower->getColor());

    window.draw(panel);

    float y = kHudH + 16.f;

    auto line = [&](const std::string& text, sf::Color color, unsigned size) {
        sf::Text label(text, font, size);
        label.setFillColor(color);
        label.setPosition(16.f, y);

        window.draw(label);

        y += size + 9.f;
    };

    line(tower->displayName(), tower->getColor(), 20);
    line("Poziom: " + std::to_string(tower->getLevel()) + "/3", Theme::TextMain, 15);

    if (tower->canShoot())
        line("Obrazenia: " + std::to_string(static_cast<int>(tower->getDamage())), Theme::TextMain, 15);

    line("Zasieg: " + std::to_string(static_cast<int>(tower->getRange())), Theme::TextMain, 15);

    std::string stats = tower->statLine();

    if (!stats.empty())
        line(stats, Theme::Warn, 14);

    if (tower->canUpgrade())
        m_btnUpgrade.setLabel("ULEPSZ $" + std::to_string(tower->getUpgradeCost()));
    else
        m_btnUpgrade.setLabel("MAKS. POZIOM");

    m_btnSell.setLabel("SPRZEDAJ $" + std::to_string(tower->getSellValue()));

    m_btnUpgrade.draw(window);
    m_btnSell.draw(window);
}