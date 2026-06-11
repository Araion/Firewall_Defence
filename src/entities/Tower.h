#pragma once
#include "core/GameObject.h"
#include "util/ArtScale.h"
#include <memory>
#include <string>

class ResourceManager;
class PlayState;
class Enemy;

// =============================================================
// Tower - abstrakcyjna baza wszystkich wiez obronnych
// Obsluguje celowanie, przeladowanie, ulepszanie, sprzedaz i rysowanie wiezy
// Strzal albo efekt ataku definiuje klasa pochodna
// =============================================================
class Tower : public GameObject {
public:
    Tower(PlayState& state, ResourceManager& res);
    virtual ~Tower() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Rysuje podstawe wiezy
    virtual void drawBaseLayer(sf::RenderWindow& window);

    // Rysuje glowke wiezy oraz elementy widoczne ponad podstawa
    virtual void drawHeadLayer(sf::RenderWindow& window);

    sf::FloatRect getBounds() const override;

    // Wykonuje atak zdefiniowany przez konkretna wieze
    virtual void attack() = 0;

    // Ulepsza wieze do maksymalnego poziomu
    void upgrade();

    bool canUpgrade() const { return m_level < kMaxLevel; }
    void setLevelDirect(int level);  // ustawia poziom wprost (do wczytania zapisu)
    int getUpgradeCost() const;
    int getSellValue() const;

    // Dane uzywane przez panel informacji i logike gry
    int getCost() const { return m_cost; }
    int getLevel() const { return m_level; }
    float getDamage() const { return m_damage; }
    float getRange() const { return m_range; }
    bool canShoot() const { return m_canShoot; }
    virtual std::string statLine() const { return ""; }
    const std::string& displayName() const { return m_name; }
    sf::Color getColor() const { return m_color; }
    float effectiveRange() const { return m_range; }

    void setSelected(bool s) { m_selected = s; }

    // Mnoznik szybkostrzelnosci z aury OverclockTower (1 = brak)
    void setFireRateBoost(float b) { m_fireRateBoost = b; }
    void resetFireRateBoost() { m_fireRateBoost = 1.f; }
    float fireRateBoost() const { return m_fireRateBoost; }

    // Aura wsparcia (OverclockTower)
    virtual void applyAura(const std::vector<Tower*>& towers) { (void)towers; }

protected:
    static constexpr int kMaxLevel = 3;

    // Sposob animacji glowki wiezy
    enum class HeadMotion {
        Aim,
        Bob
    };

    Enemy* acquireTarget() const;
    void rotateBarrelTowards(Enemy* target, float dt);
    sf::Vector2f barrelTip(float length) const;
    sf::Vector2f headPoint(float along, float perp) const;

    // Przelicza statystyki po zmianie poziomu wiezy
    virtual void applyLevelStats();

    // Konczy konfiguracje wiezy po ustawieniu danych w konstruktorze klasy pochodnej
    void finalizeSetup();

    // Laduje tekstury podstawy i glowki wiezy
    void loadTowerArt(const std::string& key);

    PlayState& m_state;
    ResourceManager& m_res;

    // Ustawienia ulozenia grafiki wiezy
    sf::Vector2f m_headPivot{0.5f, 0.5f};
    sf::Vector2f m_headOffset{0.f, 0.f};
    float m_headScaleMult = 1.f;
    float m_baseSizePx = Art::kTowerBaseWidth;

    std::shared_ptr<sf::Texture> m_baseTex;
    std::shared_ptr<sf::Texture> m_headTex;
    sf::Sprite m_baseSprite;
    sf::Sprite m_headSprite;

    bool m_hasArt = false;
    float m_artScale = 1.f;
    float m_headScale = 1.f;

    HeadMotion m_headMotion = HeadMotion::Aim;
    bool m_canShoot = true;
    float m_bobAmp = 4.f;

    std::string m_name = "Tower";
    sf::Color m_color{200, 200, 200};

    // Statystyki bazowe ustawiane przez konkretna wieze
    int m_cost = 100;
    float m_baseDamage = 20.f;
    float m_baseRange = 150.f;
    float m_fireCooldown = 0.75f;
    float m_rotationSpeed = 240.f;

    // Statystyki aktualne po uwzglednieniu poziomu
    float m_damage = 20.f;
    float m_range = 150.f;

    int m_level = 1;
    int m_invested = 0;
    float m_cooldownTimer = 0.f;
    float m_barrelAngle = 0.f;
    float m_fireRateBoost = 1.f;
    float m_animTime = 0.f;
    bool m_selected = false;

    // Aktualny cel wiezy
    Enemy* m_target = nullptr;

    void drawRangeCircle(sf::RenderWindow& window) const;
};