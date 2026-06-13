#include "states/HighScoreState.h"
#include "states/MainMenuState.h"
#include "core/Game.h"
#include "managers/ScoreManager.h"
#include "managers/AudioManager.h"
#include "util/Theme.h"
#include "util/TextUtils.h"
#include <cstdio>

HighScoreState::HighScoreState(Game& game) : GameState(game) {
    const sf::Font& font = m_game.getResources().getFont();
    m_title.setFont(font);
    m_title.setString(utf8("TABLICA WYNIKÓW"));
    m_title.setCharacterSize(48);
    m_title.setStyle(sf::Text::Bold);
    m_title.setFillColor(Theme::NeonCyan);
    sf::FloatRect tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_title.setPosition(640.f, 80.f);

    m_btnBack.setup(font, utf8("POWRÓT"), {540.f, 640.f}, {200.f, 48.f}, 20);
    m_btnBack.setColors(Theme::PanelSolid, Theme::NeonCyan, Theme::TextMain, Theme::NeonCyan);

    // Odswiezamy liste z pliku (na wypadek nowych wynikow)
    m_game.getScores().load("data/highscores.txt");
    m_game.getAudio().playMusic("music_menu");
}

void HighScoreState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
        m_game.changeState(std::make_unique<MainMenuState>(m_game));
        return;
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f m(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y));
        if (m_btnBack.contains(m)) {
            m_game.getAudio().play("ui_back", 80.f);
            m_game.changeState(std::make_unique<MainMenuState>(m_game));
        }
    }
}

void HighScoreState::update(float dt) {
    sf::Vector2i mi = sf::Mouse::getPosition(m_game.getWindow());
    m_btnBack.update(dt, sf::Vector2f(static_cast<float>(mi.x), static_cast<float>(mi.y)));
}

void HighScoreState::draw(sf::RenderWindow& window) {
    window.clear(Theme::Background);
    window.draw(m_title);

    const sf::Font& font = m_game.getResources().getFont();
    const auto& entries = m_game.getScores().entries();

    auto drawRow = [&](float y, const sf::String& a, const sf::String& b, const sf::String& c,
                       const sf::String& d, const sf::String& e, sf::Color col, unsigned size) {
        sf::Text t1(a, font, size); t1.setFillColor(col); t1.setPosition(250.f, y); window.draw(t1);
        sf::Text t2(b, font, size); t2.setFillColor(col); t2.setPosition(560.f, y); window.draw(t2);
        sf::Text t3(c, font, size); t3.setFillColor(col); t3.setPosition(680.f, y); window.draw(t3);
        sf::Text t4(d, font, size); t4.setFillColor(col); t4.setPosition(770.f, y); window.draw(t4);
        sf::Text t5(e, font, size); t5.setFillColor(col); t5.setPosition(950.f, y); window.draw(t5);
    };

    drawRow(160.f, "#  NICK", "WYNIK", "FALA", utf8("TRUDNOŚĆ"), "DATA", Theme::TextDim, 20);

    if (entries.empty()) {
        sf::Text none(utf8("Brak zapisanych wyników."), font, 22);
        none.setFillColor(Theme::TextDim);
        none.setPosition(440.f, 220.f);
        window.draw(none);
    } else {
        float y = 200.f;
        int rank = 1;
        for (const auto& e : entries) {
            sf::Color col = rank == 1 ? Theme::NeonGreen : Theme::TextMain;
            char idx[8]; std::snprintf(idx, sizeof(idx), "%d. ", rank);
            drawRow(y, utf8(sf::String(idx) + e.name), std::to_string(e.score), std::to_string(e.wave),
                    utf8(difficultyName(e.difficulty)), e.date, col, 22);
            y += 38.f;
            ++rank;
        }
    }

    m_btnBack.draw(window);
}
