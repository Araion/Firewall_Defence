#include "states/PlayState.h"
#include "core/Game.h"
#include "states/MainMenuState.h"
#include "entities/ServerCore.h"
#include "util/Theme.h"
#include <memory>
#include <algorithm>

PlayState::PlayState(Game& game) : GameState(game) {
    m_serverMaxHealth = m_game.getConfig().getInt("serverHealth", 20);
    m_serverHealth = m_serverMaxHealth;

    // Tworzy rdzen serwera i dodaje go do listy obiektow gry
    auto server = std::make_unique<ServerCore>(m_game.getResources(), sf::Vector2f{640.f, 380.f});
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

    // Klikniecie w rdzen serwera zabiera 1 punkt zdrowia - testowo
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse(
            static_cast<float>(e.mouseButton.x),
            static_cast<float>(e.mouseButton.y)
            );

        if (m_server && m_server->getBounds().contains(mouse)) {
            m_serverHealth = std::max(0, m_serverHealth - 1);
            m_server->setHealth(m_serverHealth, m_serverMaxHealth);
            m_game.getAudio().play("ui_click", 60.f);
        }
    }
}

void PlayState::update(float dt) {
    m_time += dt;
    // Aktualizuje wszystkie obiekty gry
    for (auto& o : m_objects) o->update(dt);
}

void PlayState::draw(sf::RenderWindow& window) {
    drawBackground(window);

    // Rysuje wszystkie obiekty gry
    for (auto& o : m_objects) o->draw(window);
}

void PlayState::drawBackground(sf::RenderWindow& window) {
    window.clear(Theme::Background);

    // Rysuje siatke w tle planszy
    sf::VertexArray grid(sf::Lines);
    const float spacing = 48.f;
    sf::Color line = Theme::GridLine;

    for (float x = 0.f; x <= 1280.f; x += spacing) {
        grid.append(sf::Vertex({x, 0.f}, line));
        grid.append(sf::Vertex({x, 720.f}, line));
    }

    for (float y = 0.f; y <= 720.f; y += spacing) {
        grid.append(sf::Vertex({0.f, y}, line));
        grid.append(sf::Vertex({1280.f, y}, line));
    }

    window.draw(grid);
}
