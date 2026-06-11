#pragma once
#include "core/GameState.h"
#include "util/Button.h"

// =============================================================
// HighScoreState - ekran tablicy wynikow. Wyswietla posortowana
// malejaco liste najlepszych wynikow (nick, wynik, fala, trudnosc,
// data) wczytana przez ScoreManager z data/highscores.txt
// =============================================================
class HighScoreState : public GameState {
public:
    explicit HighScoreState(Game& game);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::Text m_title;
    Button m_btnBack;
};
