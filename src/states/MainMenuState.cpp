#include "states/MainMenuState.h"
#include "states/PlayState.h"
#include "states/SettingsState.h"
#include "states/HighScoreState.h"
#include "core/Game.h"
#include "managers/AudioManager.h"
#include "util/Theme.h"
#include "util/ArtScale.h"
#include <cmath>

// Indeksy przyciskow menu
enum MenuButton { BtnNewGame = 0, BtnTutorial, BtnLoad, BtnScores, BtnSettings, BtnExit, BtnCount };

MainMenuState::MainMenuState(Game& game) : GameState(game) {
    auto& res = m_game.getResources();

    // Laduje logo, jesli plik istnieje
    m_logoTexture = res.getTexture("assets/textures/logo.png");

    if (m_logoTexture) {
        m_hasLogo = true;
        m_logoSprite.setTexture(*m_logoTexture);

        auto logoSize = m_logoTexture->getSize();
        m_logoSprite.setOrigin(logoSize.x / 2.f, logoSize.y / 2.f);
        m_logoSprite.setPosition(640.f, 120.f);

        m_logoBaseScale = (logoSize.x > 0) ? Art::kLogoWidth / static_cast<float>(logoSize.x) : 1.f;
    }


    // Przygotowuje podtytul menu
    m_subtitle.setFont(res.getFont());
    std::string subtitle = "Obroń serwer przed złośliwym oprogramowaniem";
    m_subtitle.setString(sf::String::fromUtf8(subtitle.begin(), subtitle.end()));
    m_subtitle.setCharacterSize(20);
    m_subtitle.setFillColor(Theme::NeonGreen);

    sf::FloatRect subtitleBounds = m_subtitle.getLocalBounds();
    m_subtitle.setOrigin(subtitleBounds.left + subtitleBounds.width / 2.f,
                         subtitleBounds.top + subtitleBounds.height / 2.f);
    m_subtitle.setPosition(640.f, 200.f);

    buildButtons();

    // Uruchamia muzyke menu
    m_game.getAudio().playMusic("music_menu");
}

void MainMenuState::buildButtons() {
    auto& res = m_game.getResources();
    const sf::Font& font = res.getFont();

    const char* labels[BtnCount] = { "NOWA GRA", "SAMOUCZEK", "WCZYTAJ GRE", "TABLICA WYNIKOW", "USTAWIENIA", "WYJSCIE" };

    const float w = 340.f, h = 50.f, gap = 13.f;
    const float startY = 272.f;
    const float x = 640.f - w / 2.f;

    m_buttons.clear();
    m_buttons.resize(BtnCount);
    for (int i = 0; i < BtnCount; ++i) {
        m_buttons[i].setup(font, labels[i], {x, startY + i * (h + gap)}, {w, h}, 24);
        if (i == BtnExit)
            m_buttons[i].setColors(Theme::PanelSolid, Theme::Danger, Theme::TextMain, Theme::Danger);
        else
            m_buttons[i].setColors(Theme::PanelSolid, Theme::NeonCyan, Theme::TextMain, sf::Color(40, 60, 86));
    }
}

void MainMenuState::onButtonClicked(int index) {
    switch (index) {
        case BtnNewGame:
            m_game.changeState(std::make_unique<PlayState>(m_game));
            break;
        case BtnTutorial:
            m_game.changeState(std::make_unique<PlayState>(m_game, true));
            break;
        case BtnLoad: {
            // Tworzymy rozgrywke i probujemy wczytac zapis - gdy sie nie uda zostajemy w menu
            auto play = std::make_unique<PlayState>(m_game);
            if (play->loadGame())
                m_game.changeState(std::move(play));
            break;
        }
        case BtnScores:
            m_game.changeState(std::make_unique<HighScoreState>(m_game));
            break;
        case BtnSettings:
            m_game.changeState(std::make_unique<SettingsState>(m_game));
            break;
        case BtnExit:
            m_game.quit();
            break;
        default: break;
    }
}

void MainMenuState::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse(static_cast<float>(e.mouseButton.x),
                           static_cast<float>(e.mouseButton.y));
        for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
            if (m_buttons[i].contains(mouse)) {
                m_game.getAudio().play("ui_click", 80.f);
                onButtonClicked(i);
                break;
            }
        }
    }
}

void MainMenuState::update(float dt) {
    m_time += dt;
    sf::Vector2i mousePos = sf::Mouse::getPosition(m_game.getWindow());
    sf::Vector2f mouse(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    for (auto& b : m_buttons) b.update(dt, mouse);
}

void MainMenuState::draw(sf::RenderWindow& window) {
    window.clear(Theme::Background);

    // Rysuje animowana siatke w tle
    sf::VertexArray grid(sf::Lines);
    const float spacing = 48.f;
    float offset = std::fmod(m_time * 18.f, spacing);
    sf::Color line = Theme::GridLine;

    for (float x = -spacing + offset; x < 1280.f; x += spacing) {
        grid.append(sf::Vertex({x, 0.f}, line));
        grid.append(sf::Vertex({x, 720.f}, line));
    }

    for (float y = -spacing + offset; y < 720.f; y += spacing) {
        grid.append(sf::Vertex({0.f, y}, line));
        grid.append(sf::Vertex({1280.f, y}, line));
    }
    window.draw(grid);

    // --- Logo i podtytul ---
    if (m_hasLogo) {
        float pulse = m_logoBaseScale * (1.f + 0.03f * std::sin(m_time * 2.f));
        m_logoSprite.setScale(pulse, pulse);
        window.draw(m_logoSprite);
    }
    window.draw(m_subtitle);

    // --- Przyciski ---
    for (auto& b : m_buttons) b.draw(window);
}
