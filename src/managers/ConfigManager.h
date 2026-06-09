#pragma once
#include <string>
#include <map>
#include <vector>

// =============================================================
// ConfigManager - zarzadza ustawieniami gry
// Wczytuje wartosci z pliku data/config.txt i pozwala je zapisac
// Format pliku to tekstowe pary klucz = wartosc
// =============================================================
class ConfigManager {
public:
    ConfigManager();

    // Wczytuje ustawienia z pliku konfiguracyjnego
    bool load(const std::string& relativePath = "data/config.txt");

    // Zapisuje aktualne ustawienia do pliku
    bool save() const;

    // Odczytuje wartosci ustawien
    // Jesli klucz nie istnieje, zwraca wartosc domyslna
    float getFloat(const std::string& key, float def = 0.f) const;
    int   getInt(const std::string& key, int def = 0) const;
    bool  getBool(const std::string& key, bool def = false) const;
    std::string getString(const std::string& key, const std::string& def = "") const;

    // Zmienia wartosc ustawienia
    void set(const std::string& key, const std::string& value);
    void setFloat(const std::string& key, float value);
    void setInt(const std::string& key, int value);

private:
    std::string m_path;                            // sciezka pliku (do zapisu)
    std::vector<std::string> m_lines;              // oryginalne linie (dla zachowania komentarzy)
    std::map<std::string, std::string> m_values;   // sparsowane pary klucz->wartosc

    static std::string trim(const std::string& s); // usuwa biale znaki z poczatku i konca tekstu
};
