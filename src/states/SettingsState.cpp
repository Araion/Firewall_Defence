#include "states/SettingsState.h"
#include "states/MainMenuState.h"
#include "core/Game.h"
#include "managers/ConfigManager.h"
#include "managers/AudioManager.h"
#include "util/TextUtils.h"
#include "util/Theme.h"
#include <algorithm>
#include <memory>

SettingsState::SettingsState(Game& game) : GameState(game) {
    auto& cfg = m_game.getConfig();

    m_startingCredits = cfg.getInt("startingCredits", 300);
    m_serverHealth = cfg.getInt("serverHealth", 20);
    m_totalWaves = cfg.getInt("totalWaves", 20);
    m_towerCostPercent = cfg.getInt("towerCostPercent", 100);
    m_enemyRewardPercent = cfg.getInt("enemyRewardPercent", 100);
    m_sfxVolume = cfg.getInt("sfxVolume", 100);
    m_musicVolume = cfg.getInt("musicVolume", 15);

    int diff = cfg.getInt("difficultyLevel", 1);
    m_difficulty = (diff == 0) ? 0 : (diff == 2 ? 2 : 1);

    auto& res = m_game.getResources();

    m_title.setFont(res.getFont());
    m_title.setString("USTAWIENIA");
    m_title.setCharacterSize(48);
    m_title.setStyle(sf::Text::Bold);
    m_title.setFillColor(Theme::NeonCyan);

    sf::FloatRect titleBounds = m_title.getLocalBounds();
    m_title.setOrigin(titleBounds.left + titleBounds.width / 2.f,
                      titleBounds.top + titleBounds.height / 2.f);
    m_title.setPosition(640.f, 70.f);

    const sf::Font& font = res.getFont();

    m_btnSave.setup(font, "ZAPISZ", {380.f, 560.f}, {200.f, 50.f}, 22);
    m_btnSave.setColors(Theme::PanelSolid, Theme::NeonGreen,
                        Theme::TextMain, Theme::NeonGreen);

    m_btnBack.setup(font, utf8("POWRÓT"), {710.f, 560.f}, {200.f, 50.f}, 22);
    m_btnBack.setColors(Theme::PanelSolid, Theme::Warn,
                        Theme::TextMain, Theme::Warn);

    m_game.getAudio().playMusic("music_menu");
}

sf::FloatRect SettingsState::minusRect(float y) {
    return sf::FloatRect(690.f, y, 40.f, 40.f);
}

sf::FloatRect SettingsState::plusRect(float y) {
    return sf::FloatRect(990.f, y, 40.f, 40.f);
}

sf::FloatRect SettingsState::sliderRect(float y) {
    return sf::FloatRect(690.f, y + 8.f, 340.f, 24.f);
}

static void adjust(int& v, int delta, int lo, int hi) {
    v = std::max(lo, std::min(hi, v + delta));
}

void SettingsState::applySlider(int which, float mouseX) {
    sf::FloatRect bar = sliderRect(which == 0 ? Y_SFXVOL : Y_MUSVOL);
    float t = (mouseX - bar.left) / bar.width;
    t = std::max(0.f, std::min(1.f, t));
    int v = static_cast<int>(t * 100.f + 0.5f);
    if (which == 0) { m_sfxVolume = v; m_game.getAudio().setSfxVolume(v); }
    else            { m_musicVolume = v; m_game.getAudio().setMusicVolume(v); }
}

void SettingsState::saveSettings() {
    auto& cfg = m_game.getConfig();

    cfg.setInt("startingCredits", m_startingCredits);
    cfg.setInt("serverHealth", m_serverHealth);
    cfg.setInt("totalWaves", m_totalWaves);
    cfg.setInt("towerCostPercent", m_towerCostPercent);
    cfg.setInt("enemyRewardPercent", m_enemyRewardPercent);
    cfg.setInt("sfxVolume", m_sfxVolume);
    cfg.setInt("musicVolume", m_musicVolume);
    cfg.setInt("difficultyLevel", m_difficulty);
    cfg.save();

    m_game.getAudio().setSfxVolume(m_sfxVolume);
    m_game.getAudio().setMusicVolume(m_musicVolume);

    m_savedTimer = 2.f;
}

void SettingsState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
        m_game.getAudio().play("ui_back", 80.f);
        m_game.changeState(std::make_unique<MainMenuState>(m_game));
        return;
    }

    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
        m_dragSlider = -1;
        return;
    }

    if (e.type != sf::Event::MouseButtonPressed || e.mouseButton.button != sf::Mouse::Left) return;
    sf::Vector2f m(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y));

    // Suwaki glosnosci - rozpoczecie przeciagania
    if (sliderRect(Y_SFXVOL).contains(m)) { m_dragSlider = 0; applySlider(0, m.x); m_game.getAudio().play("ui_adjust", 55.f); return; }
    if (sliderRect(Y_MUSVOL).contains(m)) { m_dragSlider = 1; applySlider(1, m.x); m_game.getAudio().play("ui_adjust", 55.f); return; }

    // Minusy i plusy
    if (minusRect(Y_CREDITS).contains(m)) adjust(m_startingCredits, -50, 50, 2000);
    if (plusRect(Y_CREDITS).contains(m))  adjust(m_startingCredits, +50, 50, 2000);
    if (minusRect(Y_HEALTH).contains(m))  adjust(m_serverHealth, -5, 5, 100);
    if (plusRect(Y_HEALTH).contains(m))   adjust(m_serverHealth, +5, 5, 100);
    if (minusRect(Y_WAVES).contains(m))   adjust(m_totalWaves, -1, 5, 50);
    if (plusRect(Y_WAVES).contains(m))    adjust(m_totalWaves, +1, 5, 50);
    if (minusRect(Y_TCOST).contains(m))   adjust(m_towerCostPercent, -10, 50, 200);
    if (plusRect(Y_TCOST).contains(m))    adjust(m_towerCostPercent, +10, 50, 200);
    if (minusRect(Y_REWARD).contains(m))  adjust(m_enemyRewardPercent, -10, 50, 200);
    if (plusRect(Y_REWARD).contains(m))   adjust(m_enemyRewardPercent, +10, 50, 200);

    // Przyciski trudnosci
    if (sf::FloatRect(690.f, Y_DIFF, 100.f, 40.f).contains(m)) m_difficulty = 0;
    if (sf::FloatRect(800.f, Y_DIFF, 120.f, 40.f).contains(m)) m_difficulty = 1;
    if (sf::FloatRect(930.f, Y_DIFF, 100.f, 40.f).contains(m)) m_difficulty = 2;

    bool onCtl = sf::FloatRect(600.f, Y_DIFF, 450.f, 40.f).contains(m);
    for (float y : {Y_CREDITS, Y_HEALTH, Y_WAVES, Y_TCOST, Y_REWARD})
        onCtl = onCtl || minusRect(y).contains(m) || plusRect(y).contains(m);
    if (onCtl) m_game.getAudio().play("ui_adjust", 55.f);

    // Zapis / powrot
    if (m_btnSave.contains(m)) { m_game.getAudio().play("ui_click", 80.f); saveSettings(); }
    if (m_btnBack.contains(m)) { m_game.getAudio().play("ui_back", 80.f); m_game.changeState(std::make_unique<MainMenuState>(m_game)); }
}

void SettingsState::update(float dt) {
    if (m_savedTimer > 0.f)
        m_savedTimer -= dt;

    sf::Vector2i mousePos = sf::Mouse::getPosition(m_game.getWindow());
    sf::Vector2f mouse(static_cast<float>(mousePos.x),
                       static_cast<float>(mousePos.y));

    // Plynne przeciaganie suwaka
    if (m_dragSlider >= 0) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) applySlider(m_dragSlider, mouse.x);
        else m_dragSlider = -1;
    }

    m_btnSave.update(dt, mouse);
    m_btnBack.update(dt, mouse);
}

void SettingsState::drawRow(sf::RenderWindow& window, float y, const sf::String& lbl, const std::string& val) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::Text label(lbl, font, 22);
    label.setFillColor(Theme::TextMain);
    label.setPosition(250.f, y + 6.f);
    window.draw(label);

    auto box = [&](sf::FloatRect r, const std::string& s) {
        sf::RectangleShape b({r.width, r.height});
        b.setPosition(r.left, r.top);
        b.setFillColor(Theme::PanelSolid);
        b.setOutlineThickness(2.f);
        b.setOutlineColor(Theme::NeonCyan);
        window.draw(b);
        sf::Text t(s, font, 24);
        t.setFillColor(Theme::NeonCyan);
        sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(r.left + r.width / 2.f - tb.width / 2.f - tb.left,
                      r.top + r.height / 2.f - tb.height / 2.f - tb.top);
        window.draw(t);
    };
    box(minusRect(y), "-");
    box(plusRect(y), "+");

    sf::Text value(val, font, 24);
    value.setFillColor(Theme::TextMain);
    sf::FloatRect vb = value.getLocalBounds();
    value.setPosition(860.f - vb.width / 2.f, y + 4.f);
    window.draw(value);
}

void SettingsState::drawSlider(sf::RenderWindow& window, float y, const sf::String& label, int value) {
    const sf::Font& font = m_game.getResources().getFont();

    sf::Text lbl(label, font, 22);
    lbl.setFillColor(Theme::TextMain);
    lbl.setPosition(250.f, y + 6.f);
    window.draw(lbl);

    sf::FloatRect bar = sliderRect(y);

    // Tlo paska
    sf::RectangleShape bg({bar.width, bar.height});
    bg.setPosition(bar.left, bar.top);
    bg.setFillColor(Theme::PanelSolid);
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(sf::Color(60, 80, 100));
    window.draw(bg);

    // Wypelnienie do aktualnej wartosci
    float t = static_cast<float>(value) / 100.f;
    sf::RectangleShape fill({bar.width * t, bar.height});
    fill.setPosition(bar.left, bar.top);
    fill.setFillColor(Theme::NeonCyan);
    window.draw(fill);

    // Uchwyt (do przeciagania)
    sf::RectangleShape handle({10.f, bar.height + 8.f});
    handle.setOrigin(5.f, 4.f);
    handle.setPosition(bar.left + bar.width * t, bar.top);
    handle.setFillColor(Theme::TextMain);
    handle.setOutlineThickness(2.f);
    handle.setOutlineColor(Theme::NeonCyan);
    window.draw(handle);

    // Wartosc %
    sf::Text val(std::to_string(value) + "%", font, 22);
    val.setFillColor(Theme::TextMain);
    val.setPosition(bar.left + bar.width + 16.f, y + 4.f);
    window.draw(val);
}

void SettingsState::draw(sf::RenderWindow& window) {
    window.clear(Theme::Background);
    window.draw(m_title);

    drawRow(window, Y_CREDITS, utf8("Początkowe kredyty"), std::to_string(m_startingCredits));
    drawRow(window, Y_HEALTH, utf8("Życie serwera"), std::to_string(m_serverHealth));
    drawRow(window, Y_WAVES, "Liczba fal", std::to_string(m_totalWaves));
    drawRow(window, Y_TCOST, utf8("Koszt wież (%)"), std::to_string(m_towerCostPercent));
    drawRow(window, Y_REWARD, utf8("Nagroda za wrógow (%)"), std::to_string(m_enemyRewardPercent));
    drawSlider(window, Y_SFXVOL, utf8("Głosność dzwięków"), m_sfxVolume);
    drawSlider(window, Y_MUSVOL, utf8("Głosność muzyki"), m_musicVolume);

    // Wiersz trudnosci
    const sf::Font& font = m_game.getResources().getFont();
    sf::Text lbl(utf8("Poziom trudności"), font, 22);
    lbl.setFillColor(Theme::TextMain);
    lbl.setPosition(250.f, Y_DIFF + 6.f);
    window.draw(lbl);

    const std::string names[3] = {"ŁATWY", "NORMALNY", "TRUDNY"};
    sf::FloatRect rects[3] = {
        sf::FloatRect(690.f, Y_DIFF, 100.f, 40.f),
        sf::FloatRect(800.f, Y_DIFF, 120.f, 40.f),
        sf::FloatRect(930.f, Y_DIFF, 100.f, 40.f)
    };
    for (int i = 0; i < 3; ++i) {
        bool sel = (m_difficulty == i);
        sf::RectangleShape b({rects[i].width, rects[i].height});
        b.setPosition(rects[i].left, rects[i].top);
        b.setFillColor(sel ? sf::Color(0, 229, 255, 60) : Theme::PanelSolid);
        b.setOutlineThickness(2.f);
        b.setOutlineColor(sel ? Theme::NeonCyan : sf::Color(60, 80, 100));
        window.draw(b);
        sf::Text t(utf8(names[i]), font, 16);
        t.setFillColor(sel ? Theme::NeonCyan : Theme::TextDim);
        sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(rects[i].left + rects[i].width / 2.f - tb.width / 2.f - tb.left,
                      rects[i].top + rects[i].height / 2.f - tb.height / 2.f - tb.top);
        window.draw(t);
    }

    m_btnSave.draw(window);
    m_btnBack.draw(window);

    if (m_savedTimer > 0.f) {
        sf::Text saved("Zapisano ustawienia!", font, 20);
        saved.setFillColor(Theme::NeonGreen);
        saved.setPosition(370.f, 624.f);
        window.draw(saved);
    }
}