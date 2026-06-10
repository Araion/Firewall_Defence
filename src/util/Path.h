#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

// =============================================================
//  Path - sciezka, po ktorej poruszaja sie wrogowie. To lista
//  punktow (waypointow). Klasa potrafi zwrocic pozycje i kat ruchu
//  po przebytym dystansie (w pikselach), co pozwala na ruch
//  niezalezny od FPS (dystans = predkosc[px/s] * dt).
// =============================================================
class Path {
public:
    Path() = default;
    explicit Path(std::vector<sf::Vector2f> points);

    // Ustawia punkty i przelicza dlugosci segmentow.
    void setPoints(std::vector<sf::Vector2f> points);

    // Pozycja na sciezce po przebytym dystansie (clamp do konca).
    sf::Vector2f positionAt(float distance) const;
    // Kat ruchu (stopnie) w danym dystansie - kierunek biezacego segmentu.
    float angleAt(float distance) const;

    float totalLength() const { return m_totalLength; }
    bool isFinished(float distance) const { return distance >= m_totalLength; }

    const std::vector<sf::Vector2f>& points() const { return m_points; }
    bool empty() const { return m_points.empty(); }

    // Punkt startowy i koncowy (np. wejscie i serwer).
    sf::Vector2f start() const { return m_points.empty() ? sf::Vector2f() : m_points.front(); }
    sf::Vector2f end() const { return m_points.empty() ? sf::Vector2f() : m_points.back(); }

private:
    std::vector<sf::Vector2f> m_points;
    std::vector<float> m_cumulative; // skumulowana dlugosc do i-tego punktu
    float m_totalLength = 0.f;

    void recalc();
};
