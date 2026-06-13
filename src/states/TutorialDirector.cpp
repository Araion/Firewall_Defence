#include "states/TutorialDirector.h"
#include "states/PlayState.h"
#include "managers/ResourceManager.h"
#include "entities/Enemy.h"
#include "entities/VirusEnemy.h"
#include "entities/TrojanEnemy.h"
#include "entities/WormEnemy.h"
#include "entities/GlitchDroneEnemy.h"
#include "entities/ProxyEnemy.h"
#include "entities/BossMalwareEnemy.h"
#include "util/Path.h"
#include "util/Theme.h"
#include "util/MathUtils.h"
#include <algorithm>

// Geometria panelu Info.
static constexpr float P_X = 230.f, P_Y = 150.f, P_W = 820.f, P_H = 380.f;

// Po zamknieciu panelu z opisem wroga odczekaj chwile, zanim wrog ruszy (lekki lead-in).
static constexpr float kSpawnLeadIn = 0.5f;
// "Sekunda przerwy" po postawieniu wiezy, zanim pojawi sie panel z opisem wroga.
static constexpr float kPostBuildPause = 1.0f;

// Indeksy typow wiez (TowerType): 0 Antivirus,1 Firewall,2 Laser,3 DataCleaner,4 Overclock,5 Corruption,6 EMP
TutorialDirector::TutorialDirector(PlayState& ps) : m_ps(ps) {
    const sf::Font& font = m_ps.resources().getFont("assets/fonts/body.ttf");
    m_btnNext.setup(font, "DALEJ >", {P_X + P_W / 2.f - 100.f, P_Y + P_H - 64.f}, {200.f, 48.f}, 22);
    m_btnNext.setColors(Theme::PanelSolid, Theme::NeonGreen, Theme::TextMain, Theme::NeonGreen);

    pickSpots();
    buildSteps();
    enterStep(0);
}

void TutorialDirector::pickSpots() {
    // Samouczek ma JEDNA sciezke przez srodek. Wyznaczamy 6 miejsc budowy TUZ
    // OBOK niej, skupionych w SRODKOWEJ czesci planszy, by gracz uczyl sie
    // stawiac wieze "plus minus na srodku". Kazde miejsce jest sprawdzane przez
    // PlayState::canPlaceAt (a gdy strona jest zajeta - probujemy druga).
    m_spots.clear();
    if (m_ps.paths().empty()) return;
    const Path* path = m_ps.paths().front();
    float total = path->totalLength();
    if (total <= 1.f) return;

    // 7 miejsc (po jednym na kazda z 7 wiez) rozlozonych wzdluz srodkowej sciezki.
    const float fracs[7] = {0.14f, 0.26f, 0.38f, 0.50f, 0.62f, 0.74f, 0.86f};
    const float off = 56.f;
    for (float f : fracs) {
        float d = total * f;
        sf::Vector2f on = path->positionAt(d);
        // Styczna -> normalna do sciezki w tym punkcie.
        sf::Vector2f ahead = path->positionAt(std::min(total, d + 6.f));
        sf::Vector2f tang = MathUtils::normalize(ahead - on);
        sf::Vector2f nrm(-tang.y, tang.x);
        sf::Vector2f a = on + nrm * off, b = on - nrm * off;
        // Preferuj strone blizsza srodkowi pionowemu
        if (std::fabs(a.y - 360.f) > std::fabs(b.y - 360.f)) std::swap(a, b);
        if (m_ps.canPlaceAt(a)) m_spots.push_back(a);
        else if (m_ps.canPlaceAt(b)) m_spots.push_back(b);
        else m_spots.push_back(a); // ostatecznosc: i tak pokaz miejsce
    }
}

void TutorialDirector::buildSteps() {
    auto info = [&](std::string t, std::string txt) {
        Step s; s.type = Info; s.title = std::move(t); s.text = std::move(txt); m_steps.push_back(s);
    };
    auto build = [&](int slot, int spot, std::string t, std::string txt) {
        Step s; s.type = Build; s.slot = slot; s.spotIdx = spot; s.title = std::move(t); s.text = std::move(txt);
        m_steps.push_back(s);
    };
    auto combat = [&](int kind, int count, std::string t, std::string txt,
                      bool enc = false, bool breach = false) {
        Step s; s.type = Combat; s.kind = kind; s.count = count; s.title = std::move(t);
        s.text = std::move(txt); s.encrypted = enc; s.breach = breach; m_steps.push_back(s);
    };
    auto combatSeq = [&](std::vector<std::pair<int, bool>> seq, std::string t, std::string txt,
                         bool breach = false) {
        Step s; s.type = Combat; s.seq = std::move(seq); s.count = static_cast<int>(s.seq.size());
        s.title = std::move(t); s.text = std::move(txt); s.breach = breach; m_steps.push_back(s);
    };

    info("WITAJ W FIREWALL DEFENSE",
         "Bronisz rdzenia serwera przed malware.\n"
         "Przeprowadzę Cię krok po kroku. Czytasz opis w tym oknie,\n"
         "klikasz DALEJ, a potem wykonujesz pokazaną akcję.\n"
         "Kliknij DALEJ.");

    // Schemat: panel z opisem WIEZY -> DALEJ -> postaw gdziekolwiek -> przerwa ->
    //          panel z opisem WROGA -> DALEJ -> wrog rusza.
    // --- 1) ANTIVIRUS ---
    build(0, 0, "ANTIVIRUS (slot 1)",
          "ANTIVIRUS strzela pojedynczo w jeden cel. Po DALEJ wybierz go\n"
          "(slot 1) i postaw w DOWOLNYM miejscu przy ścieżce.");
    combat(0, 1, "WRÓG: VIRUS",
           "VIRUS - słaby i szybki, bez zdolności.");

    // --- 2) FIREWALL ---
    build(1, 1, "FIREWALL (slot 2)",
          "FIREWALL nie strzela - roztacza pole, które SPOWALNIA wrogów\n"
          "w zasięgu. Postaw go przy ścieżce.");
    combat(2, 2, "WRÓG: WORM",
           "WORM jest szybki i po śmierci DZIELI się na 2 mini-wormy,\n"
           "które dzielą się na kolejne 2 mniejsze.");

    // --- 3) LASER ---
    build(2, 2, "LASER (slot 3)",
          "LASER to ciągły promień o dużej szybkostrzelności - mocny\n"
          "na pojedyncze, wytrzymałe cele. Postaw go.");
    combat(1, 1, "WRÓG: TROJAN",
           "TROJAN jest wolny i wytrzymały.\n"
           "Po śmierci uwalnia 2 wirusy (Payload).");

    // --- 4) DATA CLEANER ---
    build(3, 3, "DATA CLEANER (slot 4)",
          "DATA CLEANER nie strzela i nie zadaje obrażeń -\n"
          "SKANUJE i wykrywa zaszyfrowanych wrogów.\n"
          "Postaw go blisko ścieżki, by obejmował ją skanem.");
    combatSeq({{0, false}, {0, true}}, "WRÓG: ZASZYFROWANY",
              "Pojawi się zwykły VIRUS, a za nim ZASZYFROWANY. Zaszyfrowany\n"
              "jest NIEWIDOCZNY dla Antivirus/Laser - dopiero gdy Data Cleaner\n"
              "go wykryje, wieże mogą go namierzyć i zniszczyć.");

    // --- 5) CORRUPTION ---
    build(5, 4, "CORRUPTION (slot 6)",
          "CORRUPTION zadaje obrażenia w CZASIE (DoT).\n"
          "Słabe trafienie, ale rani przez kilka sekund. Postaw go.");
    combat(1, 1, "DZIAŁANIE DoT",
           "Trojan jest wytrzymały - zobacz, jak jego\n"
           "HP maleje po trafieniu Corruption.");

    // --- 6) OVERCLOCK ---
    info("CPU i TEMPERATURA",
         "Każda wieża zużywa CPU (pasek u góry). Mocne przeciążenie\n"
         "podnosi TEMP - przy 100% następuje PRZEGRZANIE i przegrana.");
    build(4, 5, "OVERCLOCK (slot 5)",
          "OVERCLOCK daje sąsiednim wieżom +szybkostrzelności,\n"
          "ale najmocniej grzeje serwer. Postaw go OBOK innych wież.");
    combat(0, 2, "WZMOCNIENIE",
           "Zwróć uwagę na przyspieszenie OVERCLOCK'u.");

    // --- 7) EMP: wybuch obszarowy ---
    build(6, 6, "EMP (slot 7)",
          "EMP wystrzeliwuje impuls, który WYBUCHA w miejscu trafienia i\n"
          "razi WSZYSTKICH wrogów w pobliżu. Idealny na grupy. Postaw go.");
    combatSeq({{0, false}, {0, false}, {0, false}, {2, false}}, "WRÓG: GRUPA",
              "EMP uderza w kilku wrogów naraz jednym wybuchem obszarowym -\n"
              "najlepszy na zbite grupy przeciwników.");

    // --- Wrogowie specjalni ---
    combat(3, 2, "WRÓG: GLITCH DRONE",
           "GLITCH DRONE jest bardzo szybki i co chwilę WYŁĄCZA wieże\n"
           "(atak DDoS) - trafiona wieża na chwilę przestaje strzelać.");
    combatSeq({{0, false}, {0, false}, {2, false}, {5, false}, {0, false}, {0, false}},
              "WRÓG: PROXY",
              "PROXY (przekaźnik botnetu) co chwilę PRZERZUCA\n"
              "do przodu sąsiednich wrogów.");

    // --- PORT BREACH ---
    info("PORT BREACH",
         "Czasem otwiera się CZERWONY tunel - nowy wektor ataku.\n"
         "Wrogowie mogą nim ominąć część obrony. Bądź czujny!");
    combat(2, 3, "ATAK PRZEZ TUNEL",
           "Wrogowie wchodzą nowym, czerwonym tunelem.", false, true);

    // --- BOSS ---
    combat(4, 1, "BOSS MALWARE",
           "BOSS ma PANCERZ (mniej obrażeń), PRZYZYWA wrogów, a poniżej\n"
           "50% HP jest ODPORNY na spowolnienie. Możesz teraz stawiać\n"
           "wieże GDZIEKOLWIEK - pokonaj go!");
    m_steps.back().freeBuild = true; // w walce z bossem gracz buduje swobodnie

    info("SAMOUCZEK UKOŃCZONY!",
         "Brawo! W normalnej grze masz też aktywne MOCE (Coolant/Surge),\n"
         "co kilka fal wybierasz ulepszenia (System Update), a trudność\n"
         "rośnie z falą. Kliknij DALEJ, aby wrócić do menu. Powodzenia!");
}

void TutorialDirector::enterStep(int i) {
    if (i >= static_cast<int>(m_steps.size())) {
        m_finished = true;
        m_panelOpen = false;
        m_ps.setTutorialGate(false, -1);
        return;
    }
    m_index = i;
    m_waiting = false;
    m_hasSpot = false;
    m_spawnSeq.clear();
    m_spawnIdx = 0;
    m_sawEnemies = false;
    m_stepDelay = 0.f;
    // kazdy krok zaczyna sie od wielkiego, zatryzmujacego gre panelu z opisem (wieza lub wrog).
    // Wlasciwa akcja (budowa / wyslanie wrogow) rusza dopiero po kliknieciu DALEJ.
    m_panelOpen = true;
    m_ps.setTutorialGate(true, -1); // zamroz gre, dopoki panel otwarty
    m_btnNext.setLabel("DALEJ >");
}

void TutorialDirector::onNext() {
    const Step& s = m_steps[m_index];
    if (s.type == Info) { enterStep(m_index + 1); return; } // sam opis -> nastepny krok

    // Build/Combat: panel byl wstepem; po DALEJ uruchamiamy wlasciwa akcje (gra rusza).
    m_panelOpen = false;
    m_waiting = true;
    if (s.type == Build) {
        if (s.slot >= 0) m_ps.unlockTower(s.slot);
        m_hasSpot = (s.spotIdx >= 0 && s.spotIdx < static_cast<int>(m_spots.size()));
        m_curSpot = m_hasSpot ? m_spots[s.spotIdx] : sf::Vector2f();
        m_towersAtStart = static_cast<int>(m_ps.towers().size());
        // Ograniczamy tylko TYP wiezy; miejsce DOWOLNE.
        m_ps.setTutorialGate(false, s.slot, m_curSpot, m_hasSpot, kSpotRadius);
    } else { // Combat
        if (s.breach) m_ps.scriptOpenBreach();
        if (!s.seq.empty()) m_spawnSeq = s.seq;
        else for (int k = 0; k < s.count; ++k) m_spawnSeq.push_back({s.kind, s.encrypted});
        m_spawnIdx = 0;
        m_sawEnemies = false;
        m_spawnTimer = kSpawnLeadIn;
        // Walka z bossem (freeBuild): pozwol stawiac wieze gdziekolwiek
        m_ps.setTutorialGate(s.freeBuild ? false : true, -1);
    }
}

std::unique_ptr<Enemy> TutorialDirector::makeEnemy(int kind, const Path* path, bool enc) {
    auto& res = m_ps.resources();
    std::unique_ptr<Enemy> e;
    switch (kind) {
    case 1: e = std::make_unique<TrojanEnemy>(res, path); break;
    case 2: e = std::make_unique<WormEnemy>(res, path, 0); break;
    case 3: e = std::make_unique<GlitchDroneEnemy>(res, path, m_ps); break;
    case 4: e = std::make_unique<BossMalwareEnemy>(res, path, m_ps); break;
    case 5: e = std::make_unique<ProxyEnemy>(res, path, m_ps); break;
    default: e = std::make_unique<VirusEnemy>(res, path); break;
    }
    e->setOwner(&m_ps);
    if (enc) e->setEncrypted(true);
    return e;
}

void TutorialDirector::spawnOne() {
    const auto& paths = m_ps.paths();
    if (paths.empty() || m_spawnIdx >= static_cast<int>(m_spawnSeq.size())) return;
    int lane = m_laneToggle % static_cast<int>(paths.size());
    ++m_laneToggle;
    int kind = m_spawnSeq[m_spawnIdx].first;
    bool enc = m_spawnSeq[m_spawnIdx].second;
    m_ps.spawn(makeEnemy(kind, paths[lane], enc));
    ++m_spawnIdx;
}

void TutorialDirector::update(float dt) {
    m_anim += dt;
    m_btnNext.update(dt, m_ps.mousePos());
    if (m_panelOpen) return;

    // Krotka przerwa miedzy krokami (np. "sekunda" po postawieniu wiezy, zanim wyskoczy
    // panel z opisem wroga). Gra w tym czasie dziala, gracz widzi swoja swiezo postawiona wieze.
    if (m_stepDelay > 0.f) {
        m_stepDelay -= dt;
        if (m_stepDelay <= 0.f) enterStep(m_index + 1);
        return;
    }

    const Step& s = m_steps[m_index];
    if (s.type == Build) {
        // Zaliczamy krok, gdy gracz postawi wymagana wieze.
        if (static_cast<int>(m_ps.towers().size()) > m_towersAtStart) {
            m_ps.setTutorialGate(true, -1);   // chwilowo blokuj do nastepnego kroku
            m_stepDelay = kPostBuildPause;    // przerwa, potem panel z opisem wroga
        }
    } else if (s.type == Combat) {
        int total = static_cast<int>(m_spawnSeq.size());
        if (m_spawnIdx < total) {
            m_spawnTimer -= dt;
            if (m_spawnTimer <= 0.f) { spawnOne(); m_spawnTimer = 0.7f; }
        }
        if (!m_ps.enemies().empty()) m_sawEnemies = true; // wrog faktycznie pojawil sie na planszy
        // Krok konczy sie, gdy WSZYSCY wyslani, jacys naprawde sie pojawili i ZGINELI.
        if (m_spawnIdx >= total && m_sawEnemies && m_ps.enemies().empty())
            enterStep(m_index + 1);
    }
}

void TutorialDirector::handleEvent(const sf::Event& e) {
    if (!m_panelOpen) return;
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f m(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y));
        if (m_btnNext.contains(m)) onNext();
    }
}

void TutorialDirector::devNext() {
    if (m_panelOpen) onNext();
}

void TutorialDirector::draw(sf::RenderWindow& window) {
    // Poza panelem samouczek nic nie rysuje - cala tresc jest w
    // wielkim, zatrzymujacym gre panelu ponizej.
    if (!m_panelOpen) return;

    const sf::Font& titleF = m_ps.resources().getFont("assets/fonts/title.ttf");
    const sf::Font& bodyF = m_ps.resources().getFont("assets/fonts/body.ttf");

    // Pelny, blokujacy panel z opisem (wieza lub wrog).
    {
        sf::RectangleShape dim({1280.f, 720.f});
        dim.setFillColor(sf::Color(0, 0, 0, 170));
        window.draw(dim);

        sf::RectangleShape panel({P_W, P_H});
        panel.setPosition(P_X, P_Y);
        panel.setFillColor(Theme::Panel);
        panel.setOutlineThickness(3.f);
        panel.setOutlineColor(Theme::NeonCyan);
        window.draw(panel);

        const Step& s = m_steps[m_index];
        // Teksty samouczka zawieraja polskie znaki zapisane jako bajty UTF-8
        sf::Text title(sf::String::fromUtf8(s.title.begin(), s.title.end()), titleF, 32);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(Theme::NeonCyan);
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, 0.f);
        title.setPosition(P_X + P_W / 2.f, P_Y + 26.f);
        window.draw(title);

        sf::Text body(sf::String::fromUtf8(s.text.begin(), s.text.end()), bodyF, 21);
        body.setFillColor(Theme::TextMain);
        body.setLineSpacing(1.25f);
        body.setPosition(P_X + 40.f, P_Y + 104.f);
        window.draw(body);

        std::string prog = "Etap " + std::to_string(m_index + 1) + " / " + std::to_string(m_steps.size());
        sf::Text pr(prog, bodyF, 16);
        pr.setFillColor(Theme::TextDim);
        pr.setPosition(P_X + 40.f, P_Y + P_H - 54.f);
        window.draw(pr);

        m_btnNext.draw(window);
    }
}
