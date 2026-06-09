#include "managers/AudioManager.h"
#include "managers/ResourceManager.h"
#include "util/Paths.h"

namespace {
    // Dodatkowy mnoznik glosnosci uzywany tylko w tym pliku niezaleznie od ustawien
    constexpr float kSfxTrim = 0.2f;
}

// Tworzy pule glosow do jednoczesnego odtwarzania efektow
AudioManager::AudioManager(ResourceManager& res) : m_res(res) {
    m_voices.resize(16);
}

void AudioManager::play(const std::string& name, float volume) {
    if (!m_enabled)
        return;

    // Szuka efektu dzwiekowego w formacie .wav
    auto buffer = m_res.getSound("assets/sounds/" + name + ".wav");

    // Brak pliku oznacza cisze
    if (!buffer)
        return;

    // Szuka wolnego glosu do odtworzenia efektu
    sf::Sound* voice = nullptr;

    for (auto& sound : m_voices) {
        if (sound.getStatus() != sf::Sound::Playing) {
            voice = &sound;
            break;
        }
    }

    // Jesli wszystkie glosy sa zajete, uzywa pierwszego ponownie
    if (!voice)
        voice = &m_voices[0];

    voice->setBuffer(*buffer);
    voice->setVolume(volume * static_cast<float>(m_sfxVolume) / 100.f * kSfxTrim);
    voice->play();
}

void AudioManager::playMusic(const std::string& name) {
    if (!m_enabled) return;
    if (m_currentMusic == name && m_music.getStatus() == sf::Music::Playing) return;

    // Szuka efektu dzwiekowego w formacie .mp3
    bool ok = m_music.openFromFile(Paths::resolve("assets/sounds/" + name + ".mp3"));
    if (!ok) { m_currentMusic.clear(); return; } // brak pliku - cisza

    m_currentMusic = name;
    m_music.setLoop(true);
    m_music.setVolume(static_cast<float>(m_musicVolume) * kSfxTrim);
    m_music.play();
}

void AudioManager::stopMusic() {
    m_music.stop();
    m_currentMusic.clear();
}

void AudioManager::setSfxVolume(int v) {
    m_sfxVolume = v < 0 ? 0 : (v > 100 ? 100 : v);
}

void AudioManager::setMusicVolume(int v) {
    m_musicVolume = v < 0 ? 0 : (v > 100 ? 100 : v);
    // Zastosuj od razu do aktualnie grajacej muzyki
    m_music.setVolume(static_cast<float>(m_musicVolume) * kSfxTrim);
}
