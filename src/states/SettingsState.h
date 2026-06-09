#pragma once
#include "core/GameState.h"
#include "util/Button.h"

// =============================================================
// SettingsState - ekran ustawien gry
// =============================================================
class SettingsState : public GameState {
public:
    explicit SettingsState(Game& game);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void saveSettings();
    void goBackToMenu();

    static sf::FloatRect minusRect(float y);
    static sf::FloatRect plusRect(float y);
    static sf::FloatRect volumeBarRect(float y);

    void drawHealthRow(sf::RenderWindow& window, float y);
    void drawVolumeRow(sf::RenderWindow& window, float y,
                       const std::string& label, int value);

    int m_serverHealth;
    int m_sfxVolume;
    int m_musicVolume;

    sf::Text m_title;
    Button m_btnSave;
    Button m_btnBack;

    float m_savedTimer = 0.f;

    static constexpr float Y_HEALTH = 190.f;
    static constexpr float Y_SFXVOL = 270.f;
    static constexpr float Y_MUSVOL = 340.f;
};
