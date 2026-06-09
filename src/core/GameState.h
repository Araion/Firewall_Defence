#pragma once
#include <SFML/Graphics.hpp>

class Game;

// =============================================================
//  GameState - bazowa klasa dla wszystkich stanow gry,
//  np menu ustawien, rozgrywki czy sama rozkrywka
//  kazdy stan musi obslugiwac zdarzenia, aktualizowac logike
//  i rysowac sie
// =============================================================
class GameState {
public:
    explicit GameState(Game& game) : m_game(game) {}
    virtual ~GameState() = default;

    virtual void handleEvent(const sf::Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
protected:
    Game& m_game;
};
