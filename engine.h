#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <random>
#include <fstream>

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameMode { MODE_NONE, MODE_1, MODE_2, MODE_3 };

struct ScoreEntry {
    std::string playerName;
    int food;
    int time;
};

struct Confetti {
    float x, y;
    float velocityX, velocityY;
    SDL_Color color;
};

class Engine {
public:
    Engine();
    ~Engine();

    bool initialize();
    void run();
    void handleEvents();
    void update();
    void render();
    void cleanup();

    void handleKeyPress(SDL_Keycode key);
    void handleKeyRelease(SDL_Keycode key);
    void handleMouseMotion(int x, int y);
    void handleMouseWheel(int y);

    void toggleFullscreen();
    void toggleGravityMode(const std::string& onText, const std::string& offText, float speed, float acceleration);
    void startSnakeGame(GameMode mode, int customTime = 120, int customFoodGoal = 20);
    void resetSnakeGame();
    void spawnFood();
    void moveSnake();
    void checkCollision();
    void saveScore();
    void loadScores();
    void setBackgroundColor(const std::string& colorName);

    void updateConfetti();
    void renderConfetti();
    void renderSnakeGame();
    void drawTextBox();
    void renderScoreboard();
    void updateSnakeGame();
    void showScoreboard();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    bool isRunning;
    int windowWidth, windowHeight;

    // Rectangle properties
    int rectX, rectY;
    int rectWidth, rectHeight;
    SDL_Color rectColor;

    // Gravity mechanics
    bool gravityMode;
    std::string gravityText;
    float gravitySpeed;
    float gravityAcceleration;
    float velocityY;
    float bounceFactor;
    float groundFriction;
    bool isOnGround;

    // Snake game
    bool snakeGameActive;
    Direction snakeDirection;
    int snakeSpeed;
    int snakeBoostedSpeed;
    bool gameOver;
    int score;
    int timer;
    Uint32 startTime;
    bool isPaused;
    std::vector<SDL_Point> snakeBody;
    SDL_Point foodPosition;

    // FPS tracking
    int frameCount;
    Uint32 lastFPSUpdateTime;
    int fps;

    // Scoreboard
    std::vector<ScoreEntry> mode1Scores;
    std::vector<ScoreEntry> mode2Scores;
    bool askingForName;
    bool showingScoreboard;

    // Confetti effects
    bool showConfetti;
    Uint32 confettiStartTime;
    std::vector<Confetti> confettiParticles;

    // UI
    bool showTextBox;
    std::string inputText;
    bool showHelp;
    SDL_Color backgroundColor;

    // Game logic
    Uint32 lastUpdateTime;
    float deltaTime;
    Uint32 lastSnakeMoveTime;
    GameMode currentMode;
    int foodGoal;
};

#endif // ENGINE_H