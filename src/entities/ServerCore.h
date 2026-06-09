#pragma once
#include "core/GameObject.h"
#include <memory>

class ResourceManager;

// =============================================================
// ServerCore - rdzen serwera broniony przez gracza
// Wyswietla animowany sprite oraz pasek zdrowia
// Wyglad rdzenia zalezy od aktualnego poziomu zdrowia
// =============================================================
class ServerCore : public GameObject {
public:
    ServerCore(ResourceManager& res, sf::Vector2f position);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    std::string getTypeName() const override { return "ServerCore"; }

    void setHealth(int current, int max) { m_health = current; m_maxHealth = max; }

private:
    static constexpr int kVariants = 3; // warianty zdrowia: dobry, sredni, niski

    std::shared_ptr<sf::Texture> m_sheets[kVariants];
    sf::Sprite m_sprites[kVariants];
    int   m_frameW[kVariants] = {0, 0, 0};  // szerokosc jednej klatki danego wariantu
    int   m_frameH[kVariants] = {0, 0, 0};
    float m_baseScale[kVariants] = {1.f, 1.f, 1.f}; // skala klatki do Art::kServerWidth
    bool m_hasTexture = false;

    int   m_frame = 0;        // biezaca klatka (0..kServerFrames-1) - wspolna dla wariantow
    float m_frameTimer = 0.f; // czas do przejscia na nastepna klatke

    sf::Vector2f m_size{96.f, 140.f};
    int m_health = 20;
    int m_maxHealth = 20;
};
