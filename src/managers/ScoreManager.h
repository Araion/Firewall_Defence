#pragma once
#include <string>
#include <vector>

// =============================================================
// ScoreEntry - pojedynczy wpis w tabeli wynikow
// Przechowuje nick gracza, wynik, fale, poziom trudnosci i date
// =============================================================
struct ScoreEntry {
    std::string name;
    int score = 0;
    int wave = 0;
    int difficulty = 1;   // 0 latwy / 1 normalny / 2 trudny
    std::string date;
};

// Zwraca nazwe poziomu trudnosci do wyswietlenia
inline const char* difficultyName(int d) {
    return d == 0 ? "Latwy" : (d == 2 ? "Trudny" : "Normalny");
}

// =============================================================
// ScoreManager - zarzadza tabela najlepszych wynikow
// Wczytuje wyniki z pliku, dodaje nowe wpisy i zapisuje je z powrotem
// Po dodaniu wyniku sortuje liste od najlepszego wyniku
// =============================================================
class ScoreManager {
public:
    // Wczytuje wyniki z pliku
    bool load(const std::string& relativePath = "data/highscores.txt");
    bool save() const;

    // Dodaje nowy wynik, sortuje liste i zapisuje plik
    void addEntry(const std::string& name, int score, int wave, int difficulty);

    // Zwraca liste wynikow
    const std::vector<ScoreEntry>& entries() const { return m_entries; }

private:
    static constexpr size_t kMaxEntries = 10; // maksymalna liczba wynikow w tabeli

    std::string m_path = "data/highscores.txt";
    std::vector<ScoreEntry> m_entries;
    bool m_loaded = false;

    static std::string today(); // zwraca biezaca date w formacie YYYY-MM-DD
    static std::string sanitize(const std::string& s);  // czysci tekst z niedozwolonych znakow
};
