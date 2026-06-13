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

    static sf::FloatRect minusRect(float y);
    static sf::FloatRect plusRect(float y);
    static sf::FloatRect sliderRect(float y);   // pasek glosnosci (przeciagany)
    void drawRow(sf::RenderWindow& window, float y, const sf::String& label, const std::string& value);
    void drawSlider(sf::RenderWindow& window, float y, const sf::String& label, int value);
    void applySlider(int which, float mouseX);   // 0 = SFX, 1 = muzyka

    int m_startingCredits;
    int m_serverHealth;
    int m_totalWaves;
    int m_difficulty;
    int m_towerCostPercent;
    int m_enemyRewardPercent;
    int m_sfxVolume;
    int m_musicVolume;

    int m_dragSlider = -1; // ktory suwak jest aktualnie przeciagany (-1 = zaden)

    sf::Text m_title;
    Button m_btnSave;
    Button m_btnBack;
    float m_savedTimer = 0.f; // komunikat "zapisano"

    static constexpr float Y_CREDITS = 132.f;
    static constexpr float Y_HEALTH  = 184.f;
    static constexpr float Y_WAVES   = 236.f;
    static constexpr float Y_DIFF    = 288.f;
    static constexpr float Y_TCOST   = 340.f;
    static constexpr float Y_REWARD  = 392.f;
    static constexpr float Y_SFXVOL  = 444.f;
    static constexpr float Y_MUSVOL  = 496.f;
};
