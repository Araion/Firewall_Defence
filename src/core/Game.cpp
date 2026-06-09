#include "core/Game.h"
#include "core/GameState.h"
#include "states/MainMenuState.h"
#include "util/Paths.h"
#include "util/Rng.h"

Game::Game() : m_window(sf::VideoMode(1280, 720), "Firewall Defense", sf::Style::Titlebar | sf::Style::Close) {
    m_window.setFramerateLimit(120);
    m_window.setKeyRepeatEnabled(false);

    // Przygotowanie podstawowych systemow gry
    Paths::init();
    Rng::seedRandom();
    m_config.load("data/config.txt");

    // Stan poczatkowy gry to menu glowne
    m_state = std::make_unique<MainMenuState>(*this);
}

Game::~Game() = default;

void Game::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        // Czas jednej klatki w sekundach
        // Ograniczamy zbyt duze wartosci, zeby ruch obiektow nie przeskakiwal
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        processEvents();

        if (m_state)
            m_state->update(dt);

        applyPendingStateChange();
        render();

        if (!m_state)
            m_window.close();
    }
}

void Game::processEvents() {
    sf::Event e;
    while (m_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            m_window.close();
            return;
        }

        // Zdarzenia przekazujemy do aktualnego stanu gry
        if (m_state)
            m_state->handleEvent(e);
    }
}

void Game::render() {
    m_window.clear(sf::Color(10, 14, 22));

    // Rysujemy tylko aktualny stan gry
    if (m_state)
        m_state->draw(m_window);

    m_window.display();
}

// Zleca zamiane aktualnego stanu na nowy
void Game::changeState(std::unique_ptr<GameState> state) {
    m_pending.type = PendingType::Change;
    m_pending.state = std::move(state);
}
// Zleca zakonczenie gry
void Game::quit() {
    m_pending.type = PendingType::Quit;
}

void Game::applyPendingStateChange() {
    // Wykonujemy zaplanowane zmiany stanow w bezpiecznym momencie
    switch (m_pending.type) {
    case PendingType::Change:
        m_state = std::move(m_pending.state);
        break;

    case PendingType::Quit:
        m_state.reset();
        m_window.close();
        break;

    case PendingType::None:
        break;
    }

    m_pending.type = PendingType::None;
}
