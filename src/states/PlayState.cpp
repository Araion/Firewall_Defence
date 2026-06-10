#include "states/PlayState.h"
#include "core/Game.h"
#include "states/MainMenuState.h"
#include "entities/ServerCore.h"
#include "entities/Enemy.h"
#include "entities/VirusEnemy.h"
#include "entities/TrojanEnemy.h"
#include "entities/WormEnemy.h"
#include "entities/ProxyEnemy.h"
#include "entities/GlitchDroneEnemy.h"
#include "entities/BossMalwareEnemy.h"
#include "util/Theme.h"
#include "util/MapFactory.h"
#include "util/Rng.h"
#include <algorithm>
#include <iostream>
#include <cmath>

PlayState::PlayState(Game& game) : GameState(game) {
    m_serverMaxHealth = m_game.getConfig().getInt("serverHealth", 20);
    m_serverHealth = m_serverMaxHealth;

    // Ładujemy mapę i jej ścieżki
    m_levelMap = MapFactory::generate(1, 12345); // Poziom normalny, stałe seed do testów

    for (const auto& lane : m_levelMap.lanes) {
        m_paths.push_back(std::make_unique<Path>(lane));
    }

    // Tworzymy serwer na pozycji określonej przez mapę
    auto server = std::make_unique<ServerCore>(m_game.getResources(), m_levelMap.serverPos);
    server->setHealth(m_serverHealth, m_serverMaxHealth);
    m_server = server.get();
    m_objects.push_back(std::move(server));
}

PlayState::~PlayState() = default;

void PlayState::handleEvent(const sf::Event& e) {
    // ESC wraca do menu glownego
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
        m_game.changeState(std::make_unique<MainMenuState>(m_game));
    }
}

void PlayState::update(float dt) {
    m_time += dt;
    // prosta petla do sprawdzania przeciwnikow (tymczasowa)
    m_spawnTimer += dt;
    if (m_spawnTimer >= m_nextSpawnDelay && !m_paths.empty()) {
        m_spawnTimer = 0.f;
        m_nextSpawnDelay = Rng::rangef(0.5f, 2.0f);

        int pathIdx = Rng::range(0, static_cast<int>(m_paths.size()) - 1);

        int typeRoll = Rng::range(0, 5);
        std::unique_ptr<Enemy> enemy;

        if (typeRoll == 0) {
            enemy = std::make_unique<VirusEnemy>(m_game.getResources(), m_paths[pathIdx].get());
        } else if (typeRoll == 1) {
            enemy = std::make_unique<TrojanEnemy>(m_game.getResources(), m_paths[pathIdx].get());
        } else if (typeRoll == 2) {
            enemy = std::make_unique<WormEnemy>(m_game.getResources(), m_paths[pathIdx].get());
        } else if (typeRoll == 3) {
            enemy = std::make_unique<ProxyEnemy>(m_game.getResources(), m_paths[pathIdx].get());
        } else if (typeRoll == 4) {
            enemy = std::make_unique<GlitchDroneEnemy>(m_game.getResources(), m_paths[pathIdx].get());
        } else {
            enemy = std::make_unique<BossMalwareEnemy>(m_game.getResources(), m_paths[pathIdx].get());}

        m_objects.push_back(std::move(enemy));
    }

    for (auto& o : m_objects) {
        o->update(dt);
    }

    // Sprawdza, czy wrogowie doszli do serwera (i zadaje mu obrażenia)
    for (auto& o : m_objects) {
        if (o->isAlive()) continue;

        if (Enemy* e = dynamic_cast<Enemy*>(o.get())) {
            if (e->reachedServer()) {
                if (m_server && !m_server->isDestroyed()) {
                    m_server->takeDamage(e->getServerDamage());
                    m_serverHealth = m_server->getHealth();
                    m_game.getAudio().play("server_hit", 90.f);
                }
            }
        }
    }

    // Usuwa "martwe" obiekty (w tym wrogów, którzy weszli w serwer lub stracili całe HP)
    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
                       [](const std::unique_ptr<GameObject>& o) { return !o->isAlive(); }),
        m_objects.end());

    // Prosty warunek przegranej
    if (m_serverHealth <= 0) {
        std::cout << "GAME OVER! Serwer został zniszczony.\n";
        m_game.changeState(std::make_unique<MainMenuState>(m_game)); // Zastępcze wyrzucenie do menu
    }
}

void PlayState::draw(sf::RenderWindow& window) {
    drawBackground(window);
    drawPaths(window);

    // Rysuje wszystkie obiekty gry (serwer, wrogów)
    for (auto& o : m_objects) o->draw(window);
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