#include "util/Path.h"
#include "util/MathUtils.h"

Path::Path(std::vector<sf::Vector2f> points) {
    setPoints(std::move(points));
}

void Path::setPoints(std::vector<sf::Vector2f> points) {
    m_points = std::move(points);
    recalc();
}

void Path::recalc() {
    // Buduje tablice dlugosci skumulowanych: m_cumulative[i] = dystans od poczatku do
    // wierzcholka i. Dzieki temu positionAt(distance) szybko znajduje wlasciwy segment.
    m_cumulative.clear();
    m_totalLength = 0.f;
    if (m_points.empty()) return;

    m_cumulative.reserve(m_points.size());
    m_cumulative.push_back(0.f);
    for (size_t i = 1; i < m_points.size(); ++i) {
        m_totalLength += MathUtils::distance(m_points[i - 1], m_points[i]);
        m_cumulative.push_back(m_totalLength);
    }
}

// Pozycja na sciezce po przebyciu 'distance' pikseli od poczatku (interpolacja liniowa
// wewnatrz segmentu). 'distance' nalicza wrog jako predkosc[px/s] * dt - stad ruch
// niezalezny od FPS.
sf::Vector2f Path::positionAt(float distance) const {
    if (m_points.empty()) return {};
    if (distance <= 0.f) return m_points.front();
    if (distance >= m_totalLength) return m_points.back();

    for (size_t i = 1; i < m_points.size(); ++i) {
        if (distance <= m_cumulative[i]) {
            float segStart = m_cumulative[i - 1];
            float segLen = m_cumulative[i] - segStart;
            float t = segLen > 1e-6f ? (distance - segStart) / segLen : 0.f;
            return m_points[i - 1] + (m_points[i] - m_points[i - 1]) * t;
        }
    }
    return m_points.back();
}

float Path::angleAt(float distance) const {
    if (m_points.size() < 2) return 0.f;
    if (distance >= m_totalLength) {
        sf::Vector2f dir = m_points.back() - m_points[m_points.size() - 2];
        return MathUtils::angleDeg(dir);
    }
    for (size_t i = 1; i < m_points.size(); ++i) {
        if (distance <= m_cumulative[i]) {
            return MathUtils::angleDeg(m_points[i] - m_points[i - 1]);
        }
    }
    return 0.f;
}
