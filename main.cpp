#include "Engine.h"

int main(int argc, char* argv[]) {
    // Engine sýnýfýndan bir nesne oluþtur
    Engine engine;

    // Oyun motorunu baþlat
    if (engine.initialize()) {
        // Oyun döngüsünü baþlat
        engine.run();
    }

    return 0;
}