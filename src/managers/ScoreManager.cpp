#include "managers/ScoreManager.h"
#include "util/Paths.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>

bool ScoreManager::load(const std::string& relativePath) {
    m_path = Paths::resolve(relativePath);
    m_entries.clear();
    m_loaded = true;

    std::ifstream in(m_path);
    if (!in.is_open()) return false; // brak pliku oznacza pusta tabele wynikow

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        // Format wpisu: nick;wynik;fala;trudnosc;data
        std::stringstream ss(line);
        std::string name, scoreStr, waveStr, diffStr, date;
        if (!std::getline(ss, name, ';')) continue;
        if (!std::getline(ss, scoreStr, ';')) continue;
        if (!std::getline(ss, waveStr, ';')) continue;
        if (!std::getline(ss, diffStr, ';')) continue;
        std::getline(ss, date); // data moze byc pusta

        ScoreEntry e;
        e.name = name;
        try { e.score = std::stoi(scoreStr); } catch (...) { e.score = 0; }
        try { e.wave = std::stoi(waveStr); } catch (...) { e.wave = 0; }
        try { e.difficulty = std::stoi(diffStr); } catch (...) { e.difficulty = 1; }
        e.date = date;
        m_entries.push_back(e);
    }

    // Najlepsze wyniki maja byc na poczatku listy
    std::sort(m_entries.begin(), m_entries.end(),
              [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
    return true;
}

bool ScoreManager::save() const {
    std::ofstream out(m_path);
    if (!out.is_open()) return false;
    for (const auto& e : m_entries)
        out << e.name << ";" << e.score << ";" << e.wave << ";" << e.difficulty << ";" << e.date << "\n";
    return true;
}

void ScoreManager::addEntry(const std::string& name, int score, int wave, int difficulty) {
    if (!m_loaded) load(m_path);

    ScoreEntry e;
    e.name = sanitize(name.empty() ? "Gracz" : name);
    e.score = score;
    e.wave = wave;
    e.difficulty = difficulty;
    e.date = today();
    m_entries.push_back(e);

    // Po dodaniu wyniku sortujemy liste od najlepszego wyniku
    std::sort(m_entries.begin(), m_entries.end(),
              [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });

    // Zostawiamy tylko najlepsze wpisy
    if (m_entries.size() > kMaxEntries) m_entries.resize(kMaxEntries);
    save();
}

// Pobranie daty dzisiejszej w formacie YYYY-MM-DD
std::string ScoreManager::today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

std::string ScoreManager::sanitize(const std::string& s) {
    std::string out;

    // Czysci nick gracza przed zapisem do pliku wynikow
    // Usuwa znaki psujace format pliku, ustawia domyslna nazwe i ogranicza dlugosc
    for (char c : s)
        if (c != ';' && c != '\n' && c != '\r') out += c;
    if (out.empty()) out = "Gracz";
    if (out.size() > 16) out = out.substr(0, 16);
    return out;
}
