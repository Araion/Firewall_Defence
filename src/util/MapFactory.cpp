#include "util/MapFactory.h"
#include "util/MathUtils.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace {
using V = sf::Vector2f;
using Lane = std::vector<sf::Vector2f>;

struct Lcg {
    unsigned s;
    explicit Lcg(unsigned seed) : s(seed ? seed : 0x9E3779B9u) {}
    unsigned next() { s = s * 1664525u + 1013904223u; return s; }
    int range(int lo, int hi) {
        if (hi <= lo) return lo;
        return lo + static_cast<int>(next() % static_cast<unsigned>(hi - lo + 1));
    }
    bool chance(int pct) { return static_cast<int>(next() % 100u) < pct; }
};

LevelMap mk(const char* name, std::vector<Lane> lanes, std::vector<Lane> breaches) {
    LevelMap m;
    m.name = name;
    m.serverPos = {1180.f, 360.f};
    m.lanes = std::move(lanes);
    m.breachLanes = std::move(breaches);
    return m;
}

void dedupe(Lane& l) {
    Lane out;
    for (const V& p : l) {
        if (out.empty() || MathUtils::distance(out.back(), p) > 1.f)
            out.push_back(p);
    }
    l = std::move(out);
}
} // namespace

namespace MapFactory {

LevelMap tutorialMap() {
    LevelMap m = mk("Samouczek",
                    {
                     {{-40,360},{240,360},{240,290},{560,290},{560,430},{880,430},{880,360},{1180,360}},
                     },
                    { {{1040,-30},{1040,360},{1180,360}} });
    return m;
}

LevelMap generate(int difficulty, unsigned seed) {
    Lcg rng(seed * 2654435761u + 0x85EBCA6Bu + static_cast<unsigned>(difficulty + 1) * 0x27D4EB2Fu);

    const float SRV_X = 1180.f, SRV_Y = 360.f;
    const float X_IN  = -40.f;
    const float X_END = 850.f;
    const float TOP   = 112.f;
    const float BOT   = 612.f;

    const int n = (difficulty <= 0) ? 2 : (difficulty >= 2 ? 4 : 3);

    const float bandTop = 175.f, bandBot = 545.f;
    std::vector<float> bandY(n);
    for (int i = 0; i < n; ++i)
        bandY[i] = (n == 1) ? SRV_Y : bandTop + (bandBot - bandTop) * i / (n - 1);

    const float gap = (n >= 2) ? (bandY[1] - bandY[0]) : 300.f;
    float amp = gap * 0.5f - 30.f;
    amp = std::min(amp, 95.f);
    if (amp < 22.f) amp = 22.f;

    const int jogs = rng.range(3, 4);
    std::vector<float> cols;
    for (int c = 0; c <= jogs; ++c)
        cols.push_back(150.f + (X_END - 150.f) * c / jogs);

    std::vector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return std::fabs(bandY[a] - SRV_Y) < std::fabs(bandY[b] - SRV_Y);
    });
    std::vector<float> turnX(n);
    const float convStart = 905.f, convStep = 42.f;
    for (int k = 0; k < n; ++k) turnX[order[k]] = convStart + convStep * k;

    std::vector<Lane> lanes;
    for (int i = 0; i < n; ++i) {
        const float y = bandY[i];
        float a = amp;
        a = std::min(a, y - TOP - 6.f);
        a = std::min(a, BOT - y - 6.f);
        if (a < 16.f) a = 16.f;
        const int dir = rng.chance(50) ? 1 : -1;

        Lane lane;
        lane.push_back({X_IN, y});
        float prev = y;
        for (size_t c = 0; c < cols.size(); ++c) {
            float level;
            if (c == 0 || c + 1 == cols.size()) level = y;
            else level = y + ((c % 2 == 1) ? a * dir : -a * dir);
            lane.push_back({cols[c], prev});
            if (std::fabs(level - prev) > 0.5f) {
                lane.push_back({cols[c], level});
                prev = level;
            }
        }
        lane.push_back({turnX[i], y});
        lane.push_back({turnX[i], SRV_Y});
        lane.push_back({SRV_X, SRV_Y});
        dedupe(lane);
        lanes.push_back(std::move(lane));
    }

    // Szablony breach - dwa rodzaje:
    //  KRAWEDZIOWE - pelne tunele tuz przy serwerze,
    //  SKROTY - zaczynaja sie w polowie planszy
    std::vector<Lane> breaches;
    {
        float bxT = static_cast<float>(rng.range(1040, 1100));
        breaches.push_back({{bxT, -30.f}, {bxT, SRV_Y}, {SRV_X, SRV_Y}});
        if (n >= 3) {
            float bxB = static_cast<float>(rng.range(1040, 1100));
            breaches.push_back({{bxB, 750.f}, {bxB, SRV_Y}, {SRV_X, SRV_Y}});
        }
        // Skrot gorny: usta tunelu w srodku planszy, potem prosto do serwera
        float sxU = static_cast<float>(rng.range(560, 680));
        float syU = static_cast<float>(rng.range(200, 290));
        breaches.push_back({{sxU, syU}, {sxU, SRV_Y}, {SRV_X, SRV_Y}});
        if (n >= 3) {
            float sxL = static_cast<float>(rng.range(560, 680));
            float syL = static_cast<float>(rng.range(430, 520));
            breaches.push_back({{sxL, syL}, {sxL, SRV_Y}, {SRV_X, SRV_Y}});
        }
    }

    static const char* easyNames[]   = {"Sektor Alfa", "Sektor Beta"};
    static const char* normalNames[] = {"Sektor Gamma", "Sektor Delta"};
    static const char* hardNames[]   = {"Sektor Omega", "Sektor Sigma"};
    const char* nm = (n == 2) ? easyNames[seed % 2]
                     : (n == 4) ? hardNames[seed % 2]
                                : normalNames[seed % 2];

    return mk(nm, std::move(lanes), std::move(breaches));
}

} // namespace MapFactory