#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// =============================================================
//  GameObject - bazowa klasa dla wszystkich obiektow gry
//  zyjacych na planszy (wieze, wrogowie, pociski, efekty, serwer),
//  daje wspolny interfejs, dzieki ktoremu petla gry moze
//  operowac polimorficznie na wskaznikach do bazy
// =============================================================
class GameObject {
public:
    virtual ~GameObject() = default;

    // Logika jednej klatki (liczone z dt w sekundach)
    virtual void update(float dt) = 0;
    // Rysowanie obiektu
    virtual void draw(sf::RenderWindow& window) = 0;
    // Prostokat ograniczajacy - do kolizji i klikniec
    virtual sf::FloatRect getBounds() const = 0;
    // Nazwa typu - do zapisu stanu i debugowania
    virtual std::string getTypeName() const = 0;

    bool isAlive() const { return m_alive; }
    void kill() { m_alive = false; }   // oznacz do usuniecia
    sf::Vector2f getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& p) { m_position = p; }

protected:
    sf::Vector2f m_position{0.f, 0.f}; // pozycja w pikselach
    bool m_alive = true;               // gdy false - obiekt zostanie usuniety z kontenera
};
