#pragma once
#include "core/GameState.h"
#include "util/Button.h"
#include <string>

// =============================================================
// GameOverState - ekran konca gry. Pokazuje osiagniety wynik,
// pozwala wpisac nick i zapisac wynik do tablicy (ScoreManager),
// zagrac ponownie lub wrocic do menu.
// =============================================================
class GameOverState : public GameState {
public:
    GameOverState(Game& game, int score, int wave, int difficulty = 1);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void saveScore();

    int  m_score;
    int  m_wave;
    int  m_difficulty;
    std::string m_nick;
    bool m_saved = false;
    float m_caretTimer = 0.f;
    bool  m_caretVisible = true;

    sf::Text m_title, m_info, m_prompt, m_nickText, m_savedText;
    Button m_btnSave, m_btnReplay, m_btnMenu;
};
