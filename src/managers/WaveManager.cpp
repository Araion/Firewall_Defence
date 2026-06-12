#include "managers/WaveManager.h"
#include "states/PlayState.h"
#include "managers/ConfigManager.h"
#include "util/Path.h"
#include "util/Rng.h"
#include "util/Theme.h"
#include "entities/VirusEnemy.h"
#include "entities/TrojanEnemy.h"
#include "entities/WormEnemy.h"
#include "entities/GlitchDroneEnemy.h"
#include "entities/ProxyEnemy.h"
#include "entities/BossMalwareEnemy.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

WaveManager::WaveManager(PlayState& state, ConfigManager& cfg,
                         const std::vector<const Path*>& paths)
    : m_state(state), m_cfg(cfg), m_paths(paths) {
    m_totalWaves = cfg.getInt("totalWaves", 20);
    m_bossEvery = cfg.getInt("bossEveryNWaves", 5);
    m_maxActive = cfg.getInt("maxActiveEnemies", 40);
    m_timeBetweenWaves = cfg.getFloat("timeBetweenWaves", 8.f);
    m_hpGrowth = cfg.getFloat("waveHealthGrowth", 0.08f);
    m_speedGrowth = cfg.getFloat("waveSpeedGrowth", 0.02f);
    m_rewardGrowth = cfg.getFloat("waveRewardGrowth", 0.03f);
    m_difficulty = cfg.getFloat("difficultyMultiplier", 1.f);
    m_rewardPercent = cfg.getFloat("enemyRewardPercent", 100.f) / 100.f;
    m_encryptChance = cfg.getFloat("encryptedEnemyChance", 0.15f);
}

std::unique_ptr<Enemy> WaveManager::makeEnemy(int kind, const Path* path) {
    auto& res = m_state.resources();
    std::unique_ptr<Enemy> e;
    switch (kind) {
    case KTrojan: e = std::make_unique<TrojanEnemy>(res, path, m_hpScale, m_speedScale, m_rewardScale); break;
    case KWorm:   e = std::make_unique<WormEnemy>(res, path, 0, m_hpScale, m_speedScale, m_rewardScale); break;
    case KGlitch: e = std::make_unique<GlitchDroneEnemy>(res, path, m_state, m_hpScale, m_speedScale, m_rewardScale); break;
    case KProxy:  e = std::make_unique<ProxyEnemy>(res, path, m_state, m_hpScale, m_speedScale, m_rewardScale); break;
    case KBoss:   e = std::make_unique<BossMalwareEnemy>(res, path, m_state, m_hpScale, m_speedScale, m_rewardScale); break;
    case KVirus:
    default:      e = std::make_unique<VirusEnemy>(res, path, m_hpScale, m_speedScale, m_rewardScale); break;
    }
    e->setOwner(&m_state);
    return e;
}

void WaveManager::startWave(int wave) {
    m_wave = wave;
    m_queue.clear();
    m_queueIdx = 0;

    // Skalowanie statystyk z numerem fali.
    m_hpScale = std::pow(1.f + m_hpGrowth, static_cast<float>(wave - 1)) * m_difficulty;
    m_speedScale = std::pow(1.f + m_speedGrowth, static_cast<float>(wave - 1));
    m_rewardScale = std::pow(1.f + m_rewardGrowth, static_cast<float>(wave - 1)) * m_rewardPercent;

    // Losowy modyfikator fali.
    int mod = Rng::range(0, 3);
    std::string modMsg;
    switch (mod) {
    case 1: m_speedScale *= 1.20f; modMsg = " [+20% predkosci]"; break;
    case 2: m_hpScale *= 1.15f;    modMsg = " [+15% HP]"; break;
    case 3: m_rewardScale *= 2.0f; modMsg = " [x2 kredyty]"; break;
    default: break; // brak modyfikatora
    }

    // Pula typow wrogow odblokowana dla danej fali (z wagami).
    std::vector<int> pool;
    auto add = [&](int kind, int weight) { for (int i = 0; i < weight; ++i) pool.push_back(kind); };
    add(KVirus, 3);
    if (wave >= 2) add(KWorm, 2);
    if (wave >= 3) add(KTrojan, 2);
    if (wave >= 4) add(KGlitch, 1);
    if (wave >= 5) add(KProxy, 1);
    if (std::getenv("FD_PROXYTEST")) add(KProxy, 6); // dev: wymus Proxy do weryfikacji

    int npaths = std::max(1, static_cast<int>(m_paths.size()));
    // Odstep miedzy spawnami maleje z numerem fali (0.9s -> min 0.35s) - pozniejsze
    // fale sa gestsze. Liczba wrogow rosnie liniowo z fala.
    float interval = std::max(0.35f, 0.9f - wave * 0.02f);
    int count = 5 + wave;

    bool bossWave = (m_bossEvery > 0) && (wave % m_bossEvery == 0);
    if (bossWave) {
        m_queue.push_back({KBoss, Rng::range(0, npaths - 1), 1.5f});
        count = std::max(4, count / 2); // przy bossie mniej "trashu"
    }
    for (int i = 0; i < count; ++i) {
        int kind = pool[Rng::range(0, static_cast<int>(pool.size()) - 1)];
        m_queue.push_back({kind, Rng::range(0, npaths - 1), interval});
    }

    m_phase = Phase::Spawning;
    m_spawnTimer = 0.f;
    m_state.showMessage("FALA " + std::to_string(wave) + modMsg, Theme::NeonCyan);
    // Sygnal dzwiekowy startu fali (fala z bossem ma wlasny, zlowrogi).
    m_state.playSfx(bossWave ? "boss_warning" : "wave_start", bossWave ? 95.f : 80.f);
    // Muzyka: fala z bossem ma wlasny utwor; zwykla fala wraca do muzyki rozgrywki.
    // playMusic jest idempotentne, wiec zwykle fale nie restartuja grajacego utworu.
    m_state.playMusic(bossWave ? "music_boss" : "music_game");
}

int WaveManager::takeCompletedWave() {
    int w = m_completedSignal;
    m_completedSignal = 0;
    return w;
}

void WaveManager::resumeAt(int wave, bool enemiesPresent) {
    m_wave = wave;
    m_victory = false;
    m_queue.clear();
    m_queueIdx = 0;
    if (enemiesPresent) {
        // Wczytani wrogowie sa na planszy - czekamy az zostana pokonani.
        m_phase = Phase::Active;
        m_activeGrace = 1.0f;
    } else {
        // Brak wrogow - odliczanie do nastepnej fali.
        m_phase = Phase::Cooldown;
        m_cooldownTimer = m_timeBetweenWaves;
    }
}

void WaveManager::spawnNext() {
    if (m_queueIdx >= m_queue.size()) return;
    const SpawnDef& s = m_queue[m_queueIdx];

    int idx = std::max(0, std::min(s.pathIndex, static_cast<int>(m_paths.size()) - 1));
    const Path* path = m_paths[idx];
    // PORT BREACH w trybie "tylko Worm": inni wrogowie nie korzystaja z tunelu.
    if (m_state.breachWormOnly() && m_state.isBreachPath(path) && s.kind != KWorm && !m_paths.empty())
        path = m_paths[0];

    auto enemy = makeEnemy(s.kind, path);
    // Modyfikator "Encrypted" - losowo nakladany na czesc wrogow.
    if (m_encryptChance > 0.f && Rng::chance(m_encryptChance) && s.kind != KBoss)
        enemy->setEncrypted(true);
    m_state.spawn(std::move(enemy));

    m_spawnTimer = s.interval;
    ++m_queueIdx;
}

void WaveManager::update(float dt) {
    if (m_victory) return;

    switch (m_phase) {
    case Phase::Cooldown:
        m_cooldownTimer -= dt;
        if (m_cooldownTimer <= 0.f) startWave(m_wave + 1);
        break;

    case Phase::Spawning:
        m_spawnTimer -= dt;
        // Nie przekraczaj limitu jednoczesnych wrogow.
        if (m_spawnTimer <= 0.f && static_cast<int>(m_state.enemies().size()) < m_maxActive) {
            if (m_queueIdx < m_queue.size()) spawnNext();
        }
        if (m_queueIdx >= m_queue.size()) {
            m_phase = Phase::Active;
            m_activeGrace = 0.5f; // odczekaj, az ostatni spawn trafi do kontenera
        }
        break;

    case Phase::Active:
        if (m_activeGrace > 0.f) { m_activeGrace -= dt; break; }
        // Fala zaliczona, gdy nie ma juz zywych wrogow.
        if (m_state.enemies().empty()) {
            int bonus = 50 + m_wave * 10;
            m_state.addScore(bonus);
            m_completedSignal = m_wave; // sygnal dla PlayState (draft ulepszen)
            m_state.showMessage("Fala " + std::to_string(m_wave) +
                                    " ukonczona! +" + std::to_string(bonus), Theme::NeonGreen);
            // Po pokonaniu fali z minibossem konczymy muzyke bossa i wracamy
            // do zwyklej muzyki rozgrywki (poza ostatnia fala - tam i tak konczy gre).
            bool wasBoss = (m_bossEvery > 0) && (m_wave % m_bossEvery == 0);
            if (wasBoss && m_wave < m_totalWaves) m_state.playMusic("music_game");
            if (m_wave >= m_totalWaves) {
                m_victory = true;
            } else {
                m_phase = Phase::Cooldown;
                m_cooldownTimer = m_timeBetweenWaves;
            }
        }
        break;
    }
}
