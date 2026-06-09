#include "managers/ConfigManager.h"
#include "util/Paths.h"
#include <fstream>
#include <sstream>

ConfigManager::ConfigManager() = default;

// Usuwa spacje i inne biale znaki z poczatku oraz konca tekstu
std::string ConfigManager::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");

    if (start == std::string::npos)
        return "";

    size_t end = s.find_last_not_of(" \t\r\n");

    return s.substr(start, end - start + 1);
}

bool ConfigManager::load(const std::string& relativePath) {
    m_path = Paths::resolve(relativePath);
    m_lines.clear();
    m_values.clear();

    std::ifstream in(m_path);

    // Brak pliku nie zatrzymuje gry, uzyte zostana wartosci domyslne
    if (!in.is_open())
        return false;

    std::string line;
    while (std::getline(in, line)) {
        m_lines.push_back(line);

        std::string trimmedLine = trim(line);

        // Pomijamy puste linie i komentarze
        if (trimmedLine.empty() || trimmedLine[0] == '#')
            continue;

        size_t separator = trimmedLine.find('=');

        if (separator == std::string::npos)
            continue;

        std::string key = trim(trimmedLine.substr(0, separator));
        std::string value = trim(trimmedLine.substr(separator + 1));

        if (!key.empty())
            m_values[key] = value;
    }

    return true;
}

bool ConfigManager::save() const {
    if (m_path.empty()) return false;

    std::ofstream out(m_path);
    if (!out.is_open()) return false;

    for (const auto& line : m_lines)
        out << line << "\n";

    return true;
}

// Pobiera float
float ConfigManager::getFloat(const std::string& key, float def) const {
    auto it = m_values.find(key);
    if (it == m_values.end()) return def;
    try { return std::stof(it->second); } catch (...) { return def; }
}

// Pobiera int
int ConfigManager::getInt(const std::string& key, int def) const {
    auto it = m_values.find(key);
    if (it == m_values.end()) return def;
    try { return static_cast<int>(std::stof(it->second)); } catch (...) { return def; }
}

// Pobiera bool
bool ConfigManager::getBool(const std::string& key, bool def) const {
    auto it = m_values.find(key);
    if (it == m_values.end()) return def;
    const std::string& v = it->second;
    if (v == "1" || v == "true" || v == "TRUE" || v == "yes") return true;
    if (v == "0" || v == "false" || v == "FALSE" || v == "no") return false;
    return def;
}

// Pobiera stringa
std::string ConfigManager::getString(const std::string& key, const std::string& def) const {
    auto it = m_values.find(key);
    return it == m_values.end() ? def : it->second;
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    m_values[key] = value;

    // Aktualizujemy istniejaca linie albo dopisujemy nowa
    bool found = false;
    for (auto& line : m_lines) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;

        size_t separator  = t.find('=');
        if (separator == std::string::npos) continue;

        std::string currentKey = trim(t.substr(0, separator));

        if (currentKey == key) {
            line = key + " = " + value;
            found = true;
            break;
        }
    }
    if (!found) m_lines.push_back(key + " = " + value);
}

void ConfigManager::setFloat(const std::string& key, float value) {
    std::ostringstream ss;
    ss << value;
    set(key, ss.str());
}

void ConfigManager::setInt(const std::string& key, int value) {
    set(key, std::to_string(value));
}