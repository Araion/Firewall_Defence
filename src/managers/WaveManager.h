#pragma once
#include <vector>
#include <memory>
#include <string>

class PlayState;
class ConfigManager;
class Path;
class Enemy;

// =============================================================
//  WaveManager - definiuje i wypuszcza fale wrogow. Z kazda fala
//  rosna statystyki (HP/predkosc/nagroda wg wspolczynnikow z configu).
//  Losuje typy wrogow z puli odblokowanej dla danego numeru fali,
//  losowa sciezke oraz modyfikator fali. Bossowie pojawiaja sie co
//  bossEveryNWaves. Nastepna fala startuje po odstepie lub po
//  pokonaniu wszystkich wrogow.
// =============================================================
class WaveManager {
public:
    WaveManager(PlayState& state, ConfigManager& cfg, const std::vector<const Path*>& paths);

    void update(float dt);

    // Wznowienie po wczytaniu zapisu: ustawia numer fali i faze.
    void resumeAt(int wave, bool enemiesPresent);

    // Zwraca numer fali ukonczonej w tej klatce (0, gdy zadna). Konsumuje sygnal.
    int   takeCompletedWave();

    int   currentWave() const { return m_wave; }
    int   totalWaves() const { return m_totalWaves; }
    bool  victory() const { return m_victory; }
    bool  inCooldown() const { return m_phase == Phase::Cooldown; }
    float cooldownRemaining() const { return m_cooldownTimer; }

private:
    // Faza fali: odliczanie do startu -> wypuszczanie wrogow -> czekanie na ich pokonanie.
    enum class Phase { Cooldown, Spawning, Active };
    // Typy wrogow w puli losowania (kolejnosc wlasna menedzera fal; KBoss = miniboss co N fal).
    enum Kind { KVirus = 0, KTrojan, KWorm, KGlitch, KProxy, KBoss };

    // Jeden wpis kolejki spawnu: typ wroga, indeks sciezki, odstep do nastepnego (s).
    struct SpawnDef { int kind; int pathIndex; float interval; };

    void startWave(int wave);
    void spawnNext();
    std::unique_ptr<Enemy> makeEnemy(int kind, const Path* path);

    PlayState& m_state;
    ConfigManager& m_cfg;
    const std::vector<const Path*>& m_paths;

    Phase m_phase = Phase::Cooldown;
    int   m_wave = 0;
    int   m_totalWaves = 20;
    int   m_bossEvery = 5;
    int   m_maxActive = 40;
    float m_timeBetweenWaves = 8.f;

    float m_cooldownTimer = 3.f;  // krotka pauza przed pierwsza fala
    float m_spawnTimer = 0.f;
    float m_activeGrace = 0.f;    // zabezpieczenie 1 klatki po zakonczeniu spawnu

    std::vector<SpawnDef> m_queue;
    size_t m_queueIdx = 0;

    // Skala biezacej fali (po uwzglednieniu wzrostu i modyfikatora).
    float m_hpScale = 1.f;
    float m_speedScale = 1.f;
    float m_rewardScale = 1.f;

    // Wspolczynniki wzrostu z configu.
    float m_hpGrowth = 0.08f;
    float m_speedGrowth = 0.02f;
    float m_rewardGrowth = 0.03f;
    float m_difficulty = 1.f;
    float m_rewardPercent = 1.f; // modyfikator nagrod z ustawien
    float m_encryptChance = 0.f; // szansa na zaszyfrowanego wroga

    bool m_victory = false;
    int  m_completedSignal = 0; // numer fali ukonczonej w biezacej klatce (0 = brak)
};
