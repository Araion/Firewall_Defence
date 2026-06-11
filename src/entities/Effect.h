#pragma once
#include "core/GameObject.h"

// =============================================================
// Effect - abstrakcyjna baza efektow wizualnych
// Efekty nie biora udzialu w kolizjach i znikaja po uplywie czasu zycia
// =============================================================
class Effect : public GameObject {
public:
    Effect(sf::Vector2f pos, float life) { m_position = pos; m_maxLife = life; }

    sf::FloatRect getBounds() const override { return sf::FloatRect(m_position.x, m_position.y, 0.f, 0.f); }
    std::string getTypeName() const override { return "Effect"; }

protected:
    float m_life = 0.f;
    float m_maxLife = 0.6f;

    // Zwraca postep zycia efektu od 0 do 1
    // Uzywane do wygaszania, zmiany skali albo przezroczystosci
    float progress() const { return m_maxLife > 0.f ? m_life / m_maxLife : 1.f; }

    // Aktualizuje czas zycia efektu i usuwa go po zakonczeniu animacji
    void advance(float dt) { m_life += dt; if (m_life >= m_maxLife) kill(); }
};
