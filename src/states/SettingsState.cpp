#include "states/SettingsState.h"
#include "states/MainMenuState.h"
#include "core/Game.h"
#include "managers/ConfigManager.h"
#include "managers/AudioManager.h"
#include "util/Theme.h"
#include <algorithm>
#include <memory>

namespace {
int clampValue(int value, int minValue, int maxValue) {
    return std::max(minValue, std::min(maxValue, value));
}

int valueFromBar(float mouseX, const sf::FloatRect& bar) {
    float t = (mouseX - bar.left) / bar.width;
    t = std::max(0.f, std::min(1.f, t));

    return static_cast<int>(t * 100.f + 0.5f);
}
}

SettingsState::SettingsState(Game& game) : GameState(game) {
    auto& cfg = m_game.getConfig();

    m_serverHealth = cfg.getInt("serverHealth", 20);
    m_sfxVolume = cfg.getInt("sfxVolume", 100);
    m_musicVolume = cfg.getInt("musicVolume", 50);

    auto& res = m_game.getResources();

    m_title.setFont(res.getFont());
    m_title.setString("USTAWIENIA");
    m_title.setCharacterSize(48);
    m_title.setStyle(sf::Text::Bold);
    m_title.setFillColor(Theme::NeonCyan);

    sf::FloatRect titleBounds = m_title.getLocalBounds();
    m_title.setOrigin(titleBounds.left + titleBounds.width / 2.f,
                      titleBounds.top + titleBounds.height / 2.f);
    m_title.setPosition(640.f, 90.f);

    const sf::Font& font = res.getFont();

    m_btnSave.setup(font, "ZAPISZ", {410.f, 530.f}, {220.f, 50.f}, 22);
    m_btnSave.setColors(Theme::PanelSolid, Theme::NeonGreen,
                        Theme::TextMain, Theme::NeonGreen);

    m_btnBack.setup(font, "POWROT", {660.f, 530.f}, {220.f, 50.f}, 22);
    m_btnBack.setColors(Theme::PanelSolid, Theme::Warn,
                        Theme::TextMain, Theme::Warn);

    m_game.getAudio().playMusic("music_menu");
}

sf::FloatRect SettingsState::minusRect(float y) {
    return sf::FloatRect(600.f, y, 40.f, 40.f);
}

sf::FloatRect SettingsState::plusRect(float y) {
    return sf::FloatRect(790.f, y, 40.f, 40.f);
}

sf::FloatRect SettingsState::volumeBarRect(float y) {
    return sf::FloatRect(600.f, y + 10.f, 230.f, 20.f);
}

void SettingsState::saveSettings() {
    auto& cfg = m_game.getConfig();

    cfg.setInt("serverHealth", m_serverHealth);
    cfg.setInt("sfxVolume", m_sfxVolume);
    cfg.setInt("musicVolume", m_musicVolume);
    cfg.save();

    m_game.getAudio().setSfxVolume(m_sfxVolume);
    m_game.getAudio().setMusicVolume(m_musicVolume);

    m_savedTimer = 2.f;
}

void SettingsState::goBackToMenu() {
    m_game.getAudio().play("ui_back", 80.f);
    m_game.changeState(std::make_unique<MainMenuState>(m_game));
}

void SettingsState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
        goBackToMenu();
        return;
    }

    if (e.type != sf::Event::MouseButtonPressed || e.mouseButton.button != sf::Mouse::Left)
        return;

    sf::Vector2f mouse(static_cast<float>(e.mouseButton.x),
                       static_cast<float>(e.mouseButton.y));

    bool changedControl = false;

    // Zmiana zycia serwera przyciskami minus i plus
    if (minusRect(Y_HEALTH).contains(mouse)) {
        m_serverHealth = clampValue(m_serverHealth - 5, 5, 100);
        changedControl = true;
    }

    if (plusRect(Y_HEALTH).contains(mouse)) {
        m_serverHealth = clampValue(m_serverHealth + 5, 5, 100);
        changedControl = true;
    }

    // Zmiana glosnosci przez klikniecie w pasek
    if (volumeBarRect(Y_SFXVOL).contains(mouse)) {
        m_sfxVolume = valueFromBar(mouse.x, volumeBarRect(Y_SFXVOL));
        m_game.getAudio().setSfxVolume(m_sfxVolume);
        changedControl = true;
    }

    if (volumeBarRect(Y_MUSVOL).contains(mouse)) {
        m_musicVolume = valueFromBar(mouse.x, volumeBarRect(Y_MUSVOL));
        m_game.getAudio().setMusicVolume(m_musicVolume);
        changedControl = true;
    }

    if (changedControl)
        m_game.getAudio().play("ui_adjust", 55.f);

    if (m_btnSave.contains(mouse)) {
        m_game.getAudio().play("ui_click", 80.f);
        saveSettings();
    }

    if (m_btnBack.contains(mouse))
        goBackToMenu();
}

void SettingsState::update(float dt) {
    if (m_savedTimer > 0.f)
        m_savedTimer -= dt;

    sf::Vector2i mousePos = sf::Mouse::getPosition(m_game.getWindow());
    sf::Vector2f mouse(static_cast<float>(mousePos.x),
                       static_cast<float>(mousePos.y));

    m_btnSave.update(dt, mouse);
    m_btnBack.update(dt, mouse);
}

void SettingsState::drawHealthRow(sf::RenderWindow& window, float y) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::Text label("Zycie serwera", font, 22);
    label.setFillColor(Theme::TextMain);
    label.setPosition(200.f, y + 6.f);
    window.draw(label);

    auto drawBox = [&](const sf::FloatRect& rect, const std::string& text) {
        sf::RectangleShape box({rect.width, rect.height});
        box.setPosition(rect.left, rect.top);
        box.setFillColor(Theme::PanelSolid);
        box.setOutlineThickness(2.f);
        box.setOutlineColor(Theme::NeonCyan);
        window.draw(box);

        sf::Text boxText(text, font, 24);
        boxText.setFillColor(Theme::NeonCyan);

        sf::FloatRect bounds = boxText.getLocalBounds();
        boxText.setPosition(rect.left + rect.width / 2.f - bounds.width / 2.f - bounds.left,
                            rect.top + rect.height / 2.f - bounds.height / 2.f - bounds.top);
        window.draw(boxText);
    };

    drawBox(minusRect(y), "-");
    drawBox(plusRect(y), "+");

    sf::Text value(std::to_string(m_serverHealth), font, 24);
    value.setFillColor(Theme::TextMain);

    sf::FloatRect valueBounds = value.getLocalBounds();
    value.setPosition(715.f - valueBounds.width / 2.f, y + 4.f);
    window.draw(value);
}

void SettingsState::drawVolumeRow(sf::RenderWindow& window, float y,
                                  const std::string& labelText, int value) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::Text label(labelText, font, 22);
    label.setFillColor(Theme::TextMain);
    label.setPosition(200.f, y + 6.f);
    window.draw(label);

    sf::FloatRect rect = volumeBarRect(y);

    // Tlo paska glosnosci
    sf::RectangleShape background({rect.width, rect.height});
    background.setPosition(rect.left, rect.top);
    background.setFillColor(Theme::PanelSolid);
    background.setOutlineThickness(2.f);
    background.setOutlineColor(sf::Color(60, 80, 100));
    window.draw(background);

    // Wypelnienie paska glosnosci
    float fillWidth = rect.width * static_cast<float>(value) / 100.f;

    sf::RectangleShape fill({fillWidth, rect.height});
    fill.setPosition(rect.left, rect.top);
    fill.setFillColor(Theme::NeonCyan);
    window.draw(fill);

    sf::Text valueText(std::to_string(value) + "%", font, 22);
    valueText.setFillColor(Theme::TextMain);
    valueText.setPosition(850.f, y + 4.f);
    window.draw(valueText);
}

void SettingsState::draw(sf::RenderWindow& window) {
    window.clear(Theme::Background);

    window.draw(m_title);

    drawHealthRow(window, Y_HEALTH);
    drawVolumeRow(window, Y_SFXVOL, "Glosnosc dzwiekow", m_sfxVolume);
    drawVolumeRow(window, Y_MUSVOL, "Glosnosc muzyki", m_musicVolume);

    m_btnSave.draw(window);
    m_btnBack.draw(window);

    if (m_savedTimer > 0.f) {
        const sf::Font& font = m_game.getResources().getFont();

        sf::Text saved("Zapisano ustawienia", font, 20);
        saved.setFillColor(Theme::NeonGreen);
        saved.setPosition(410.f, 600.f);
        window.draw(saved);
    }
}