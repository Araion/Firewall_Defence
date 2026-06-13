#include "managers/ResourceManager.h"
#include "util/Paths.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

ResourceManager::ResourceManager() = default;

std::shared_ptr<sf::Texture> ResourceManager::getTexture(const std::string& relativePath) {
    auto it = m_textures.find(relativePath);
    if (it != m_textures.end()) return it->second;

    auto tex = std::make_shared<sf::Texture>();
    std::string full = Paths::resolve(relativePath);
    if (tex->loadFromFile(full)) {
        tex->setSmooth(true);
        m_textures[relativePath] = tex;
        return tex;
    }

    std::cerr << "[ResourceManager] Brak tekstury: " << full
              << " (uzywam ksztaltu zastepczego)\n";
    m_textures[relativePath] = nullptr;
    return nullptr;
}

bool ResourceManager::ensureSystemFont() {
    if (m_systemFont) return true;

    // Lista typowych czcionek
    const char* candidates[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/SFNS.ttf",
        "/Library/Fonts/Arial.ttf"
    };

    for (const char* c : candidates) {
        std::error_code ec;
        if (!fs::exists(c, ec)) continue;

        auto f = std::make_shared<sf::Font>();
        if (f->loadFromFile(c)) {
            m_systemFont = f;
            std::cout << "[ResourceManager] Czcionka systemowa: " << c << "\n";
            return true;
        }
    }

    std::cerr << "[ResourceManager] UWAGA: brak jakiejkolwiek czcionki - tekst bedzie niewidoczny.\n";
    return false;
}

const sf::Font& ResourceManager::getFont() {
    ensureSystemFont();
    if (m_systemFont) return *m_systemFont;

    static sf::Font empty;
    return empty;
}

std::shared_ptr<sf::SoundBuffer> ResourceManager::getSound(const std::string& relativePath) {
    auto it = m_sounds.find(relativePath);
    if (it != m_sounds.end()) return it->second;

    auto buf = std::make_shared<sf::SoundBuffer>();
    std::string full = Paths::resolve(relativePath);
    if (buf->loadFromFile(full)) {
        m_sounds[relativePath] = buf;
        return buf;
    }

    m_sounds[relativePath] = nullptr;
    return nullptr;
}