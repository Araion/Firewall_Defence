#pragma once
#include <SFML/Graphics/Color.hpp>

// =============================================================
//  Theme - wspolna paleta kolorow gry
// =============================================================
namespace Theme {
    // Tla i panele
    inline const sf::Color Background  {10, 14, 22};
    inline const sf::Color Panel       {18, 24, 38, 235};
    inline const sf::Color PanelSolid  {18, 24, 38};
    inline const sf::Color GridLine    {30, 44, 66};

    // Kolory neonowe i akcenty
    inline const sf::Color NeonCyan    {0, 229, 255};
    inline const sf::Color NeonGreen   {57, 255, 136};
    inline const sf::Color NeonMagenta {255, 46, 147};
    inline const sf::Color NeonBlue    {64, 156, 255};

    // Kolory statusow
    inline const sf::Color Warn        {255, 176, 32};
    inline const sf::Color Danger      {255, 64, 64};
    inline const sf::Color Ok          {57, 255, 136};

     // Kolory tekstu
    inline const sf::Color TextMain    {220, 235, 245};
    inline const sf::Color TextDim     {130, 150, 170};

     // Miesza dwa kolory wedlug wartosci t od 0 do 1
    inline sf::Color lerp(const sf::Color& a, const sf::Color& b, float t) {
        if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f;
        return sf::Color(
            static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
            static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
            static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
            static_cast<sf::Uint8>(a.a + (b.a - a.a) * t));
    }
}
