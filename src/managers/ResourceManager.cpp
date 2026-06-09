#include "managers/ResourceManager.h"
#include "util/Paths.h"
#include <filesystem>

namespace fs = std::filesystem;

ResourceManager::ResourceManager() = default;

std::shared_ptr<sf::Texture> ResourceManager::getTexture(const std::string& relativePath) {
    auto it = m_textures.find(relativePath);
    if (it != m_textures.end()) return it->second;

    auto texture = std::make_shared<sf::Texture>();
    std::string fullPath = Paths::resolve(relativePath);

    if (texture->loadFromFile(fullPath)) {
        texture->setSmooth(true);
        m_textures[relativePath] = texture;
        return texture;
    }

    // Zapamietujemy brak tekstury, zeby nie probowac ladowac jej wiele razy
    m_textures[relativePath] = nullptr;
    return nullptr;
}

const sf::Font& ResourceManager::getFont() {
    ensureSystemFont();

    if (m_systemFont)
        return *m_systemFont;

    // Pusta czcionka awaryjna, zeby gra sie nie wysypala
    static sf::Font emptyFont;
    return emptyFont;
}

std::shared_ptr<sf::SoundBuffer> ResourceManager::getSound(const std::string& relativePath) {
    auto it = m_sounds.find(relativePath);
    if (it != m_sounds.end())
        return it->second;

    auto buffer = std::make_shared<sf::SoundBuffer>();
    std::string fullPath = Paths::resolve(relativePath);

    if (buffer->loadFromFile(fullPath)) {
        m_sounds[relativePath] = buffer;
        return buffer;
    }

    // Zapamietujemy brak dzwieku, zeby nie probowac ladowac go wiele razy
    m_sounds[relativePath] = nullptr;
    return nullptr;
}

bool ResourceManager::ensureSystemFont() {
    if (m_systemFont)
        return true;

    // Szukamy czcionek systemowych
    const char* candidates[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    };

    for (const char* path : candidates) {
        std::error_code error;

        if (!fs::exists(path, error))
            continue;

        auto font = std::make_shared<sf::Font>();

        if (font->loadFromFile(path)) {
            m_systemFont = font;
            return true;
        }
    }

    return false;
}