#include "core/Game.h"
#include <iostream>
#include <exception>

using namespace std;

int main()
{
    // na wypadek bledu wyswietli sie on w konsoli
    try {
        Game game;
        game.run();
    } catch (const exception& ex) {
        cout << "[FATAL] Wyjatek: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        cout << "[FATAL] Nieznany wyjatek.\n";
        return 1;
    }
    return 0;
}
