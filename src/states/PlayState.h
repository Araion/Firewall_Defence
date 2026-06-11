#pragma once
#include "core/GameState.h"
#include "core/GameObject.h"
#include "util/Path.h"
#include "util/LevelMap.h"
#include "util/Button.h"
#include "managers/CollisionManager.h"
#include "entities/TowerType.h"
#include <vector>
#include <memory>

class ServerCore;
class Enemy;
class Tower;

// =========================================+====================
// PlayState - stan rozgrywki
// Odpowiada za aktualizacje i rysowanie obiektow gry
// Obsluguje tez powrot do menu po nacisnieciu ESC
// =============================================================
class PlayState : public GameState {
public:
    explicit PlayState(Game& game);
    ~PlayState() override;

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Dodaje obiekt do gry na koniec aktualnej klatki
    void spawn(std::unique_ptr<GameObject> obj);

    // Tworzy efekt eksplozji w podanej pozycji
    void spawnExplosion(sf::Vector2f pos, sf::Color color, float scale = 1.f);

    const std::vector<Enemy*>& enemies() const { return m_frameEnemies; }
    class ResourceManager& resources();
    void playMusic(const std::string& name);

    // --- Zapis / wczytanie stanu (SaveManager) ---
    struct TowerSave { std::string type; int level; float x, y; };
    struct EnemySave { std::string type; float hp; int pathIndex; float distance; };
    std::vector<TowerSave> snapshotTowers() const;
    std::vector<EnemySave> snapshotEnemies() const;
    int credits() const { return m_credits; }
    int score() const { return m_score; }
    int serverHealth() const { return m_serverHealth; }
    unsigned mapSeed() const { return m_mapSeed; }
    int difficultyLevel() const { return m_difficulty; }
    void buildBoard(unsigned seed, int difficulty);     // mapa + sciezki + serwer
    void applyLoadedMeta(int credits, int serverHp, int score);
    void placeTowerFromSave(const std::string& type, int level, sf::Vector2f pos);
    void addEnemyFromSave(const std::string& type, float hp, int pathIndex, float distance);
    void finishLoad();
    bool saveGame();   // szybki zapis (F5)
    bool loadGame();   // wczytanie (F9 / menu)

private:
    // Tworzenie przeciwnikow
    void spawnEnemy();
    std::unique_ptr<Enemy> createEnemyByName(const std::string& name, const Path* path);
    int  pathIndexOf(const Path* p) const;

    // Aktualizacja obiektow gry
    void rebuildCaches();
    void handleDeaths();
    void removeDead();
    void flushPending();

    // Budowanie i wybor wiez
    std::unique_ptr<Tower> createTower(TowerType type);
    bool canPlaceAt(sf::Vector2f pos) const;
    bool tryBuildAt(TowerType type, sf::Vector2f pos);
    Tower* towerAt(sf::Vector2f pos) const;
    void setBuildSelection(int idx);
    void selectTower(Tower* t);
    void deselectAll();
    void sellSelected();
    void upgradeSelected();
    void onLeftClick(sf::Vector2f mouse);
    sf::FloatRect shopSlotRect(int i) const;

    // Rysowanie planszy i interfejsu
    void drawBackground(sf::RenderWindow& window);
    void drawPaths(sf::RenderWindow& window);
    static void drawThickLine(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b,
                              float thickness, sf::Color color);
    void drawBuildPreview(sf::RenderWindow& window);
    void drawShop(sf::RenderWindow& window);
    void drawHud(sf::RenderWindow& window);
    void drawTowerPanel(sf::RenderWindow& window);

    static constexpr float kTowerFootprint = 22.f;
    static constexpr float kHudH = 50.f;
    static constexpr float kShopY = 648.f;

    // Plansza i obiekty gry
    std::vector<std::unique_ptr<Path>> m_paths;
    LevelMap m_levelMap;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    std::vector<std::unique_ptr<GameObject>> m_pendingSpawns;

    ServerCore* m_server = nullptr;

    // Listy pomocnicze odswiezane co klatke
    std::vector<Enemy*> m_frameEnemies;
    std::vector<Tower*> m_frameTowers;

    CollisionManager m_collision;

    // Stan rozgrywki
    int m_serverHealth = 20;
    int m_serverMaxHealth = 20;
    int m_credits = 300;
    int m_score = 0;
    unsigned m_mapSeed = 0;
    int   m_difficulty = 1;

    float m_spawnTimer = 0.f;
    float m_nextSpawnDelay = 1.0f;
    float m_time = 0.f;
    float m_uiTime = 0.f;

    sf::Vector2f m_mouse;

    // Wybrany typ wiezy do budowy, -1 oznacza brak wyboru
    int m_buildSelection = -1;

    // Podglad wiezy przed postawieniem
    std::unique_ptr<Tower> m_preview;

    // Aktualnie zaznaczona wieza z otwartym panelem
    Tower* m_selectedTower = nullptr;

    Button m_btnUpgrade;
    Button m_btnSell;
};