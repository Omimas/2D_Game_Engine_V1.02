#include "Engine.h"

int main(int argc, char* argv[]) {
    // Engine s�n�f�ndan bir nesne olu�tur
    Engine engine;

    // Oyun motorunu ba�lat
    if (engine.initialize()) {
        // Oyun d�ng�s�n� ba�lat
        engine.run();
    }

    return 0;
}