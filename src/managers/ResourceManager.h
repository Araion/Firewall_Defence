#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <string>

// =============================================================
//  ResourceManager - laduje i przechowuje zasoby (tekstury, czcionki,
//  dzwieki) jako shared_ptr i cache'uje je po sciezce.
//  Klucz odpornosci: jesli pliku brak - NIE wywalamy gry.
//   * tekstura: zwracamy nullptr,
//   * czcionka: czcionka systemowa,
//   * dzwiek: zwracamy nullptr (cisza).
// =============================================================
class ResourceManager {
public:
    ResourceManager();

    // Zwraca teksture (cache). Moze byc nullptr, gdy pliku nie ma.
    std::shared_ptr<sf::Texture> getTexture(const std::string& relativePath);

    // Zwraca czcionke systemowa (ladowana raz z zasobow systemu operacyjnego).
    // Parametr domyslny = "" pozwala na wywolania zarowno z M3 jak i z M4!
    const sf::Font& getFont(const std::string& relativePath = "");

    // Zwraca bufor dzwieku lub nullptr, gdy pliku nie ma.
    std::shared_ptr<sf::SoundBuffer> getSound(const std::string& relativePath);

private:
    std::map<std::string, std::shared_ptr<sf::Texture>> m_textures;
    std::map<std::string, std::shared_ptr<sf::SoundBuffer>> m_sounds;

    std::shared_ptr<sf::Font> m_systemFont; // jedyna czcionka - z systemu operacyjnego

    // Probuje zaladowac czcionke systemowa (raz). Zwraca true, gdy sie uda.
    bool ensureSystemFont();
};