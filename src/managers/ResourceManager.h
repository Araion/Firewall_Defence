#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <string>

// =============================================================
// ResourceManager - laduje i przechowuje zasoby gry
// Zarzadza teksturami i dzwiekami
// Raz wczytany zasob jest zapamietywany i uzywany ponownie
// Jesli pliku brakuje, gra dziala dalej bez przerywania
// =============================================================
class ResourceManager {
public:
    ResourceManager();

    // Zwraca teksture, jesli pliku nie ma, zwraca nullptr
    std::shared_ptr<sf::Texture> getTexture(const std::string& relativePath);

    // Pobiera czcionke systemowa
    const sf::Font& getFont();

    // Zwraca dzwiek, jesli pliku nie ma, zwraca nullptr
    std::shared_ptr<sf::SoundBuffer> getSound(const std::string& relativePath);

private:
    std::map<std::string, std::shared_ptr<sf::Texture>> m_textures;
    std::map<std::string, std::shared_ptr<sf::SoundBuffer>> m_sounds;

    std::shared_ptr<sf::Font> m_systemFont;

    // Laduje czcionke systemowa, jesli nie zostala jeszcze zaladowana
    bool ensureSystemFont();
};
