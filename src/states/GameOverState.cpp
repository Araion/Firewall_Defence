#include "states/GameOverState.h"
#include "states/MainMenuState.h"
#include "states/PlayState.h"
#include "core/Game.h"
#include "managers/AudioManager.h"
#include "managers/ScoreManager.h"
#include "util/Theme.h"

GameOverState::GameOverState(Game& game, int score, int wave, int difficulty)
    : GameState(game), m_score(score), m_wave(wave), m_difficulty(difficulty) {
    const sf::Font& font = m_game.getResources().getFont();

    // Ekran konca gry
    m_title.setFont(font);
    m_title.setString("KONIEC GRY!");
    m_title.setCharacterSize(64);
    m_title.setStyle(sf::Text::Bold);
    m_title.setFillColor(Theme::Danger);
    sf::FloatRect tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_title.setPosition(640.f, 150.f);

    m_info.setFont(font);
    m_info.setString("Wynik: " + std::to_string(score) +
                     "     Trudnosc: " + difficultyName(difficulty));
    m_info.setCharacterSize(26);
    m_info.setFillColor(Theme::TextMain);
    sf::FloatRect ib = m_info.getLocalBounds();
    m_info.setOrigin(ib.left + ib.width / 2.f, ib.top + ib.height / 2.f);
    m_info.setPosition(640.f, 230.f);

    m_prompt.setFont(font);
    m_prompt.setString("Wpisz nick i zapisz wynik:");
    m_prompt.setCharacterSize(20);
    m_prompt.setFillColor(Theme::TextDim);
    m_prompt.setPosition(440.f, 290.f);

    m_nickText.setFont(font);
    m_nickText.setCharacterSize(28);
    m_nickText.setFillColor(Theme::NeonCyan);
    m_nickText.setPosition(452.f, 322.f);

    m_savedText.setFont(font);
    m_savedText.setString("Wynik zapisany!");
    m_savedText.setCharacterSize(20);
    m_savedText.setFillColor(Theme::NeonGreen);
    m_savedText.setPosition(452.f, 370.f);

    m_btnSave.setup(font, "ZAPISZ", {440.f, 400.f}, {400.f, 48.f}, 22);
    m_btnSave.setColors(Theme::PanelSolid, Theme::NeonGreen, Theme::TextMain, Theme::NeonGreen);
    m_btnReplay.setup(font, "ZAGRAJ PONOWNIE", {440.f, 462.f}, {400.f, 48.f}, 22);
    m_btnReplay.setColors(Theme::PanelSolid, Theme::NeonCyan, Theme::TextMain, Theme::NeonCyan);
    m_btnMenu.setup(font, "MENU GLOWNE", {440.f, 524.f}, {400.f, 48.f}, 22);
    m_btnMenu.setColors(Theme::PanelSolid, Theme::Warn, Theme::TextMain, Theme::Warn);

    m_game.getAudio().play("game_over");
}

void GameOverState::saveScore() {
    if (m_saved) return;

    if (m_nick.empty())
        m_nick = "Gracz";

    m_game.getScores().addEntry(m_nick, m_score, m_wave, m_difficulty);
    m_saved = true;
}

void GameOverState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::TextEntered && !m_saved) {
        sf::Uint32 u = e.text.unicode;
        if (u == 8) {                              // backspace
            if (!m_nick.empty()) m_nick.pop_back();
        } else if (u >= 32 && u < 127) {           // drukowalne ASCII
            if (m_nick.size() < 16) m_nick += static_cast<char>(u);
        }
        return;
    }
    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Return) {
            m_game.getAudio().play("ui_click", 80.f);
            saveScore();
            return;
        }
        if (e.key.code == sf::Keyboard::Escape) {
            m_game.changeState(std::make_unique<MainMenuState>(m_game));
            return;
        }
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f m(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y));
        if (!m_saved && m_btnSave.contains(m)) { m_game.getAudio().play("ui_click", 80.f); saveScore(); return; }
        if (m_btnReplay.contains(m)) { m_game.changeState(std::make_unique<PlayState>(m_game)); return; }
        if (m_btnMenu.contains(m)) { m_game.changeState(std::make_unique<MainMenuState>(m_game)); return; }
    }
}

void GameOverState::update(float dt) {
    m_caretTimer += dt;
    if (m_caretTimer >= 0.5f) { m_caretTimer = 0.f; m_caretVisible = !m_caretVisible; }

    sf::Vector2i mi = sf::Mouse::getPosition(m_game.getWindow());
    sf::Vector2f mouse(static_cast<float>(mi.x), static_cast<float>(mi.y));
    m_btnSave.setEnabled(!m_saved);
    m_btnSave.update(dt, mouse);
    m_btnReplay.update(dt, mouse);
    m_btnMenu.update(dt, mouse);
}

void GameOverState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(8, 10, 18));

    window.draw(m_title);
    window.draw(m_info);
    window.draw(m_prompt);

    sf::RectangleShape box({360.f, 44.f});
    box.setPosition(448.f, 318.f);
    box.setFillColor(Theme::PanelSolid);
    box.setOutlineThickness(2.f);
    box.setOutlineColor(Theme::NeonCyan);
    window.draw(box);

    std::string shown = m_nick;
    if (!m_saved && m_caretVisible) shown += "_";
    m_nickText.setString(shown);
    window.draw(m_nickText);

    if (m_saved) window.draw(m_savedText);

    m_btnSave.draw(window);
    m_btnReplay.draw(window);
    m_btnMenu.draw(window);
}
