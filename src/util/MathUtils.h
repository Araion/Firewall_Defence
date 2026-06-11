#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

// =============================================================
//  MathUtils - drobne funkcje matematyczne na wektorach 2D:
//  dlugosc, normalizacja, dystans, kat. Uzywane przez ruch
//  wrogow/pociskow.
// =============================================================
namespace MathUtils {
constexpr float PI = 3.14159265358979323846f;

inline float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    return length(b - a);
}

// Zwraca wektor jednostkowy; dla zerowego zwraca (0,0).
inline sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len < 1e-6f) return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(v.x / len, v.y / len);
}

// Kat wektora w stopniach (0 = w prawo, rosnie zgodnie z ruchem wskazowek - uklad SFML).
inline float angleDeg(const sf::Vector2f& v) {
    return std::atan2(v.y, v.x) * 180.f / PI;
}

// Najkrotsza roznica katow (z zakresu -180..180) - do plynnego obrotu lufy.
// Sztuczka +540 (=360+180) i -180 sprowadza dowolna roznice do przedzialu
// [-180,180], wiec lufa obraca sie zawsze "krotsza strona".
inline float shortestAngleDiff(float from, float to) {
    float diff = std::fmod(to - from + 540.f, 360.f) - 180.f;
    return diff;
}

// Obraca 'current' w strone 'target' z maksymalna predkoscia 'maxStep' (stopnie).
inline float rotateTowards(float current, float target, float maxStep) {
    float diff = shortestAngleDiff(current, target);
    if (std::fabs(diff) <= maxStep) return target;
    return current + (diff > 0 ? maxStep : -maxStep);
}

inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

// Najmniejsza odleglosc punktu 'p' od odcinka a-b (sprawdzanie, czy miejsce
// budowy wiezy nie lezy na sciezce wrogow)
inline float pointSegmentDistance(const sf::Vector2f& p,
                                  const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f ab = b - a;
    float len2 = ab.x * ab.x + ab.y * ab.y;
    if (len2 < 1e-6f) return distance(p, a);
    float t = dot(p - a, ab) / len2;
    if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f;
    sf::Vector2f proj = a + ab * t;
    return distance(p, proj);
}
}

