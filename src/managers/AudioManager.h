#pragma once
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <memory>

class ResourceManager;

// =============================================================
// AudioManager - zarzadza dzwiekami i muzyka w grze
// Odtwarza efekty dzwiekowe oraz muzyke w tle
// Jesli plik dzwiekowy nie istnieje, gra dziala dalej bez dzwieku
// =============================================================
class AudioManager {
public:
    explicit AudioManager(ResourceManager& res);

    // Odtwarza efekt dzwiekowy o podanej nazwie
    void play(const std::string& name, float volume = 100.f);

    // Odtwarza muzyke w tle
    void playMusic(const std::string& name);

    // Zatrzymuje aktualnie grajaca muzyke
    void stopMusic();

    // Wlacza lub wylacza dzwiek
    void setEnabled(bool e) { m_enabled = e; }

    // Ustawia glosnosc efektow dzwiekowych 0-100
    void setSfxVolume(int v);

    // Ustawia glosnosc muzyki 0-100
    void setMusicVolume(int v);

    int sfxVolume() const { return m_sfxVolume; }
    int musicVolume() const { return m_musicVolume; }

private:
    ResourceManager& m_res;
    std::vector<sf::Sound> m_voices; // pula obiektow odtwarzajacych efekty dzwiekowe
    sf::Music m_music;               // muzyka w tle
    std::string m_currentMusic;      // nazwa aktualnie grajacej muzyki
    bool m_enabled = true;           // czy dzwiek jest wlaczony
    int m_sfxVolume = 100;           // glosnosc efektow dzwiekowych 0-100
    int m_musicVolume = 50;          // glosnosc muzyki 0-100
};
