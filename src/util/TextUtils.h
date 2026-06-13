#pragma once

#include <SFML/Graphics.hpp>
#include <string>

inline sf::String utf8(const std::string& text)
{
    return sf::String::fromUtf8(text.begin(), text.end());
}