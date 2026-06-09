#pragma once
#include "core/GameState.h"
#include "util/Button.h"
#include <vector>
#include <memory>

// =============================================================
// MainMenuState - ekran menu glownego
// Pokazuje logo, podtytul i przyciski startu oraz wyjscia z gry
// =============================================================
class MainMenuState : public GameState {
public:
    explicit MainMenuState(Game& game);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void buildButtons();
    void onButtonClicked(int index);

    std::shared_ptr<sf::Texture> m_logoTexture;
    sf::Sprite m_logoSprite;
    bool m_hasLogo = false;
    float m_logoBaseScale = 1.f; // podstawowa skala logo

    sf::Text m_subtitle;

    std::vector<Button> m_buttons;
    float m_time = 0.f;  // czas uzywany do animacji tla i logo
};
