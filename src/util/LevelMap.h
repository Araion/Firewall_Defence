#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <string>

struct LevelMap {
    std::string name = "Mapa";
    sf::Vector2f serverPos{1180.f, 360.f};
    std::vector<std::vector<sf::Vector2f>> lanes; // aktywne sciezki wrogow
    std::vector<std::vector<sf::Vector2f>> breachLanes; // szablony breach
};