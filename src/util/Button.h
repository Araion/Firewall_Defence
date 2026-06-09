#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// =============================================================
// Button - prosty przycisk interfejsu gry
// Obsluguje wyglad, tekst, pozycje, animacje najechania i wykrywanie klikniecia
// =============================================================
class Button {
public:
    Button() = default;

    // Ustawia tekst, czcionke, pozycje i rozmiar przycisku
    // Czcionka musi istniec tak dlugo jak przycisk
    void setup(const sf::Font& font, const std::string& label,
               sf::Vector2f pos, sf::Vector2f size, unsigned charSize = 24);

    void setColors(sf::Color base, sf::Color hover, sf::Color text, sf::Color outline);
    void setLabel(const std::string& label);
    void setPosition(sf::Vector2f pos);
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Aktualizacja animacji hover na podstawie pozycji myszy
    void update(float dt, sf::Vector2f mouse);
    void draw(sf::RenderWindow& window);

    // Sprawdza, czy podany punkt znajduje sie na przycisku
    bool contains(sf::Vector2f point) const;
    bool isHovered() const { return m_hovered; }

private:
    // Ustawia tekst na srodku przycisku
    void relayout();

    const sf::Font* m_font = nullptr;
    sf::RectangleShape m_box;
    sf::Text m_text;

    sf::Vector2f m_pos{0.f, 0.f};
    sf::Vector2f m_size{200.f, 50.f};

    sf::Color m_base{18, 24, 38};
    sf::Color m_hover{0, 229, 255};
    sf::Color m_textColor{220, 235, 245};
    sf::Color m_outline{0, 229, 255};

    float m_scale = 1.f;     // aktualna skala animacji
    bool  m_hovered = false;
    bool  m_enabled = true;
};
