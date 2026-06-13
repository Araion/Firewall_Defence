#include "util/Button.h"
#include "util/Theme.h"
#include <algorithm>

void Button::setup(const sf::Font& font, const sf::String& label,
                   sf::Vector2f pos, sf::Vector2f size, unsigned charSize) {
    m_font = &font;
    m_pos = pos;
    m_size = size;

    m_text.setFont(font);
    m_text.setString(label);
    m_text.setCharacterSize(charSize);
    m_text.setFillColor(m_textColor);

    m_box.setSize(m_size);
    m_box.setOutlineThickness(2.f);
    relayout();
}

void Button::setColors(sf::Color base, sf::Color hover, sf::Color text, sf::Color outline) {
    m_base = base; m_hover = hover; m_textColor = text; m_outline = outline;
    m_text.setFillColor(m_textColor);
}

void Button::setLabel(const sf::String& label) {
    m_text.setString(label);
    relayout();
}

void Button::setPosition(sf::Vector2f pos) {
    m_pos = pos;
    relayout();
}

void Button::relayout() {
    // Ustawia rozmiar i pozycje przycisku z uwzglednieniem animacji skali
    sf::Vector2f scaledSize = m_size * m_scale;
    sf::Vector2f center = m_pos + m_size * 0.5f;

    m_box.setSize(scaledSize);
    m_box.setOrigin(scaledSize * 0.5f);
    m_box.setPosition(center);

    // Wysrodkowanie napisu w przycisku
    sf::FloatRect tb = m_text.getLocalBounds();
    m_text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_text.setPosition(center);
}

bool Button::contains(sf::Vector2f point) const {
    // Sprawdza klikniecie wzgledem podstawowego rozmiaru przycisku
    sf::FloatRect r(m_pos.x, m_pos.y, m_size.x, m_size.y);
    return r.contains(point);
}

void Button::update(float dt, sf::Vector2f mouse) {
    m_hovered = m_enabled && contains(mouse);

    // Plynnie zmienia skale przycisku przy najechaniu mysza
    float target = m_hovered ? 1.06f : 1.0f;
    m_scale += (target - m_scale) * std::min(1.f, dt * 12.f);
    relayout();
}

void Button::draw(sf::RenderWindow& window) {
    if (!m_enabled) {
        m_box.setFillColor(sf::Color(30, 34, 44));
        m_box.setOutlineColor(sf::Color(60, 70, 84));
        m_text.setFillColor(Theme::TextDim);
    } else if (m_hovered) {
        // Podswietla przycisk po najechaniu mysza
        m_box.setFillColor(sf::Color(m_hover.r, m_hover.g, m_hover.b, 45));
        m_box.setOutlineColor(m_hover);
        m_text.setFillColor(m_hover);
    } else {
        m_box.setFillColor(m_base);
        m_box.setOutlineColor(m_outline);
        m_text.setFillColor(m_textColor);
    }
    window.draw(m_box);
    window.draw(m_text);
}
