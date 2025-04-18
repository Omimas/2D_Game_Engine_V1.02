﻿#include "Engine.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <random>
#include <fstream>
#include <functional>

Engine::Engine()
    : isRunning(false),
    window(nullptr),
    renderer(nullptr),
    font(nullptr),
    rectX(100),
    rectY(100),
    rectWidth(200),
    rectHeight(200),
    gravityMode(false),
    gravityText("Gravity mode off"),
    gravitySpeed(0.5f),
    gravityAcceleration(0.05f),
    windowWidth(0),
    windowHeight(0),
    rectColor({ 255, 0, 0, 255 }),
    showTextBox(false),
    inputText(""),
    snakeGameActive(false),
    snakeDirection(RIGHT),
    snakeSpeed(2),
    snakeBoostedSpeed(4),
    gameOver(false),
    score(0),
    timer(120),
    startTime(0),
    isPaused(false),
    frameCount(0),
    lastFPSUpdateTime(0),
    fps(0),
    foodPosition({ 0, 0 }),
    showHelp(false),
    backgroundColor({ 0, 0, 0, 255 }),
    velocityY(0.0f),
    bounceFactor(0.7f),
    groundFriction(0.1f),
    isOnGround(false),
    lastUpdateTime(SDL_GetTicks()),
    deltaTime(0.0f),
    lastSnakeMoveTime(SDL_GetTicks()),
    currentMode(MODE_NONE),
    foodGoal(20),
    askingForName(false),
    showingScoreboard(false),
    showConfetti(false),
    confettiStartTime(0) {
    std::cout << "Engine object created." << std::endl;
    loadScores();
}

Engine::~Engine() {
    cleanup();
    std::cout << "Engine object destroyed." << std::endl;
}

bool Engine::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("2D Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 800, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font) {
        std::cerr << "Font could not be loaded: " << TTF_GetError() << std::endl;
        std::cerr << "Font path: fonts/arial.ttf" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    isRunning = true;
    std::cout << "Graphics library initialized." << std::endl;
    return true;
}

void Engine::cleanup() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

void Engine::run() {
    while (isRunning) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

void Engine::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        if (event.type == SDL_KEYDOWN) {
            handleKeyPress(event.key.keysym.sym);
        }
        if (event.type == SDL_KEYUP) {
            handleKeyRelease(event.key.keysym.sym);
        }
        if (event.type == SDL_MOUSEMOTION && !gravityMode) {
            handleMouseMotion(event.motion.x, event.motion.y);
        }
        if (event.type == SDL_MOUSEWHEEL) {
            handleMouseWheel(event.wheel.y);
        }
    }
}

void Engine::handleKeyPress(SDL_Keycode key) {
    if (showTextBox) {
        switch (key) {
        case SDLK_BACKSPACE:
            if (!inputText.empty()) {
                inputText.pop_back();
            }
            break;
        case SDLK_RETURN:
            if (askingForName) {
                if (!inputText.empty()) {
                    saveScore();
                    askingForName = false;
                    showTextBox = false;
                    showingScoreboard = true;
                    if (currentMode == MODE_1 && !mode1Scores.empty() && score == 20 && (120 - timer) < mode1Scores[0].time) {
                        showConfetti = true;
                        confettiStartTime = SDL_GetTicks();
                        confettiParticles.clear();
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> disX(0, windowWidth);
                        std::uniform_int_distribution<> disY(0, windowHeight);
                        std::uniform_int_distribution<> disColor(0, 255);
                        for (int i = 0; i < 50; ++i) {
                            Confetti particle;
                            particle.x = static_cast<float>(disX(gen));
                            particle.y = static_cast<float>(disY(gen));
                            particle.color = { static_cast<Uint8>(disColor(gen)), static_cast<Uint8>(disColor(gen)), static_cast<Uint8>(disColor(gen)), 255 };
                            particle.velocityX = (static_cast<float>(disX(gen) - windowWidth / 2) / 100.0f) * 2.0f;
                            particle.velocityY = -5.0f + (static_cast<float>(disY(gen)) / 100.0f) * 2.0f;
                            confettiParticles.push_back(particle);
                        }
                    }
                    else if (currentMode == MODE_2 && (mode2Scores.empty() || score > mode2Scores[0].food)) {
                        showConfetti = true;
                        confettiStartTime = SDL_GetTicks();
                        confettiParticles.clear();
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> disX(0, windowWidth);
                        std::uniform_int_distribution<> disY(0, windowHeight);
                        std::uniform_int_distribution<> disColor(0, 255);
                        for (int i = 0; i < 50; ++i) {
                            Confetti particle;
                            particle.x = static_cast<float>(disX(gen));
                            particle.y = static_cast<float>(disY(gen));
                            particle.color = { static_cast<Uint8>(disColor(gen)), static_cast<Uint8>(disColor(gen)), static_cast<Uint8>(disColor(gen)), 255 };
                            particle.velocityX = (static_cast<float>(disX(gen) - windowWidth / 2) / 100.0f) * 2.0f;
                            particle.velocityY = -5.0f + (static_cast<float>(disY(gen)) / 100.0f) * 2.0f;
                            confettiParticles.push_back(particle);
                        }
                    }
                }
            }
            else if (showingScoreboard) {
                showingScoreboard = false;
                showConfetti = false;
                confettiParticles.clear();
                resetSnakeGame();
            }
            else if (inputText.find("background is ") == 0) {
                std::string colorName = inputText.substr(14);
                setBackgroundColor(colorName);
                showTextBox = false;
                inputText = "";
            }
            else if (inputText == "play mode1") {
                startSnakeGame(MODE_1);
                showTextBox = false;
                inputText = "";
            }
            else if (inputText == "play mode2") {
                startSnakeGame(MODE_2);
                showTextBox = false;
                inputText = "";
            }
            else if (inputText.find("play mode3") == 0) {
                int customTime = 120;
                int customFoodGoal = 20;
                std::string args = inputText.substr(10);
                std::istringstream iss(args);
                std::string token;

                while (iss >> token) {
                    if (token == "time" && iss >> customTime) {}
                    else if (token == "food" && iss >> customFoodGoal) {}
                }
                startSnakeGame(MODE_3, customTime, customFoodGoal);
                showTextBox = false;
                inputText = "";
            }
            else if (inputText == "help") {
                showHelp = true;
                inputText = "";
            }
            else if (inputText == "restart") {
                resetSnakeGame();
                startSnakeGame(currentMode, timer, foodGoal);
                showTextBox = false;
                inputText = "";
            }
            break;
        case SDLK_x:
            if (showingScoreboard) {
                showingScoreboard = false;
                showConfetti = false;
                confettiParticles.clear();
                resetSnakeGame();
            }
            else {
                resetSnakeGame();
                showTextBox = false;
                showHelp = false;
                askingForName = false;
                inputText = "";
            }
            break;
        case SDLK_f:
            toggleFullscreen();
            showTextBox = false;
            inputText = "";
            break;
        default:
            if (key >= 32 && key <= 126) {
                if (inputText.length() < 20) {
                    inputText += static_cast<char>(key);
                }
            }
            break;
        }
    }
    else {
        switch (key) {
        case SDLK_UP:
            if (snakeDirection != DOWN) {
                snakeDirection = UP;
                snakeSpeed = snakeBoostedSpeed;
            }
            break;
        case SDLK_DOWN:
            if (snakeDirection != UP) {
                snakeDirection = DOWN;
                snakeSpeed = snakeBoostedSpeed;
            }
            break;
        case SDLK_LEFT:
            if (snakeDirection != RIGHT) {
                snakeDirection = LEFT;
                snakeSpeed = snakeBoostedSpeed;
            }
            break;
        case SDLK_RIGHT:
            if (snakeDirection != LEFT) {
                snakeDirection = RIGHT;
                snakeSpeed = snakeBoostedSpeed;
            }
            break;
        case SDLK_e:
            if (!snakeGameActive) {
                toggleGravityMode("Earth Gravity is on", "Earth Gravity is off", 0.1f, 0.8f);
            }
            break;
        case SDLK_m:
            if (!snakeGameActive) {
                toggleGravityMode("Moon Gravity is on", "Moon Gravity is off", 0.1f, 0.13f);
            }
            break;
        case SDLK_r:
            if (!snakeGameActive) {
                rectColor = { 255, 0, 0, 255 };
            }
            break;
        case SDLK_g:
            if (!snakeGameActive) {
                rectColor = { 0, 255, 0, 255 };
            }
            break;
        case SDLK_b:
            if (!snakeGameActive) {
                rectColor = { 0, 0, 255, 255 };
            }
            break;
        case SDLK_c:
            showTextBox = true;
            showHelp = false;
            break;
        case SDLK_s:
            if (snakeGameActive) {
                isPaused = !isPaused;
            }
            break;
        case SDLK_x:
            if (snakeGameActive || showTextBox || showHelp) {
                resetSnakeGame();
                showTextBox = false;
                showHelp = false;
                inputText = "";
            }
            break;
        }
    }
}

void Engine::handleKeyRelease(SDL_Keycode key) {
    switch (key) {
    case SDLK_UP:
    case SDLK_DOWN:
    case SDLK_LEFT:
    case SDLK_RIGHT:
        snakeSpeed = 2;
        break;
    }
}

void Engine::handleMouseMotion(int mouseX, int mouseY) {
    rectX = mouseX - rectWidth / 2;
    rectY = mouseY - rectHeight / 2;

    rectX = std::max(0, std::min(windowWidth - rectWidth, rectX));
    rectY = std::max(0, std::min(windowHeight - rectHeight, rectY));
}

void Engine::handleMouseWheel(int y) {
    if (y > 0) {
        rectWidth += 10;
        rectHeight += 10;
    }
    else if (y < 0) {
        rectWidth -= 10;
        rectHeight -= 10;
    }

    rectWidth = std::max(50, std::min(400, rectWidth));
    rectHeight = std::max(50, std::min(400, rectHeight));
}

void Engine::toggleFullscreen() {
    Uint32 fullscreenFlag = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (fullscreenFlag) {
        SDL_SetWindowFullscreen(window, 0);
    }
    else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
}

void Engine::toggleGravityMode(const std::string& onText, const std::string& offText, float speed, float acceleration) {
    gravityMode = !gravityMode;
    gravityText = gravityMode ? onText : offText;
    gravitySpeed = speed;
    gravityAcceleration = acceleration;
    velocityY = 0.0f;
    isOnGround = false;
    rectY = 100;
}

void Engine::startSnakeGame(GameMode mode, int customTime, int customFoodGoal) {
    snakeGameActive = true;
    gameOver = false;
    isPaused = false;
    askingForName = false;
    showingScoreboard = false;
    showConfetti = false;
    confettiParticles.clear();
    snakeBody.clear();
    snakeBody.push_back({ windowWidth / 2, windowHeight / 2 });
    snakeDirection = RIGHT;
    snakeSpeed = 2;
    snakeBoostedSpeed = 4;
    foodPosition = { rand() % (windowWidth - 10), rand() % (windowHeight - 10) };
    score = 0;
    currentMode = mode;

    if (mode == MODE_1) {
        timer = 120;
        foodGoal = 20;
    }
    else if (mode == MODE_2) {
        timer = -1;
        foodGoal = -1;
    }
    else if (mode == MODE_3) {
        timer = customTime;
        foodGoal = customFoodGoal;
    }

    startTime = SDL_GetTicks() / 1000;
    lastSnakeMoveTime = SDL_GetTicks();
}

void Engine::updateSnakeGame() {
    if (!snakeGameActive || gameOver || isPaused) return;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 moveInterval = (snakeSpeed == snakeBoostedSpeed) ? 20 : 40;
    if (currentTime - lastSnakeMoveTime < moveInterval) return;
    lastSnakeMoveTime = currentTime;

    int newX = snakeBody[0].x;
    int newY = snakeBody[0].y;

    int stepSize = 4;
    switch (snakeDirection) {
    case UP: newY -= stepSize; break;
    case DOWN: newY += stepSize; break;
    case LEFT: newX -= stepSize; break;
    case RIGHT: newX += stepSize; break;
    }

    snakeBody.insert(snakeBody.begin(), { newX, newY });

    if (abs(newX - foodPosition.x) < 10 && abs(newY - foodPosition.y) < 10) {
        foodPosition = { rand() % (windowWidth - 10), rand() % (windowHeight - 10) };
        score++;
    }
    else {
        snakeBody.pop_back();
    }

    if (newX < 0 || newX >= windowWidth || newY < 0 || newY >= windowHeight) {
        gameOver = true;
        if (currentMode != MODE_3) {
            askingForName = true;
            showTextBox = true;
            inputText = "";
        }
        return;
    }

    for (size_t i = 1; i < snakeBody.size(); i++) {
        if (newX == snakeBody[i].x && newY == snakeBody[i].y) {
            gameOver = true;
            if (currentMode != MODE_3) {
                askingForName = true;
                showTextBox = true;
                inputText = "";
            }
            return;
        }
    }

    if (currentMode != MODE_2) {
        int currentTimeSec = SDL_GetTicks() / 1000;
        timer = (currentMode == MODE_1 || currentMode == MODE_3) ? (timer - (currentTimeSec - startTime)) : timer;
        startTime = currentTimeSec;

        if (score >= foodGoal) {
            gameOver = true;
            if (currentMode != MODE_3) {
                askingForName = true;
                showTextBox = true;
                inputText = "";
            }
            return;
        }
        else if (timer <= 0) {
            gameOver = true;
            if (currentMode != MODE_3) {
                askingForName = true;
                showTextBox = true;
                inputText = "";
            }
            return;
        }
    }
}

void Engine::renderSnakeGame() {
    if (!snakeGameActive) return;

    for (size_t i = 0; i < snakeBody.size(); i++) {
        Uint8 greenValue = static_cast<Uint8>(255 - (i * 100 / static_cast<int>(std::max<size_t>(1, snakeBody.size()))));
        SDL_SetRenderDrawColor(renderer, 0, greenValue, 0, 255);
        SDL_Rect rect;
        if (i == 0) {
            rect = { snakeBody[i].x - 2, snakeBody[i].y - 2, 16, 16 };
        }
        else if (i == static_cast<size_t>(snakeBody.size() - 1)) {
            rect = { snakeBody[i].x + 2, snakeBody[i].y + 2, 8, 8 };
        }
        else {
            rect = { snakeBody[i].x, snakeBody[i].y, 12, 12 };
        }
        SDL_RenderFillRect(renderer, &rect);

        if (i < snakeBody.size() - 1) {
            int x1 = snakeBody[i].x;
            int y1 = snakeBody[i].y;
            int x2 = snakeBody[i + 1].x;
            int y2 = snakeBody[i + 1].y;

            if (x1 == x2) {
                SDL_Rect connector = { x1, std::min(y1, y2), 12, static_cast<int>(abs(y1 - y2)) + 12 };
                SDL_RenderFillRect(renderer, &connector);
            }
            else if (y1 == y2) {
                SDL_Rect connector = { std::min(x1, x2), y1, static_cast<int>(abs(x1 - x2)) + 12, 12 };
                SDL_RenderFillRect(renderer, &connector);
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect foodRect = { foodPosition.x, foodPosition.y, 10, 10 };
    SDL_RenderFillRect(renderer, &foodRect);

    SDL_Color textColor = { 255, 255, 255, 255 };
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    if (scoreSurface) {
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        if (scoreTexture) {
            SDL_Rect scoreRect = { 10, 10, scoreSurface->w, scoreSurface->h };
            SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
        }
        SDL_FreeSurface(scoreSurface);
    }

    std::string timerText = "Time: " + (currentMode == MODE_2 ? "inf" : std::to_string(timer));
    SDL_Surface* timerSurface = TTF_RenderText_Solid(font, timerText.c_str(), textColor);
    if (timerSurface) {
        SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
        if (timerTexture) {
            SDL_Rect timerRect = { windowWidth - 150, 10, timerSurface->w, timerSurface->h };
            SDL_RenderCopy(renderer, timerTexture, nullptr, &timerRect);
            SDL_DestroyTexture(timerTexture);
        }
        SDL_FreeSurface(timerSurface);
    }

    if (snakeSpeed == snakeBoostedSpeed) {
        std::string boostText = "Boost Mode";
        SDL_Surface* boostSurface = TTF_RenderText_Solid(font, boostText.c_str(), textColor);
        if (boostSurface) {
            SDL_Texture* boostTexture = SDL_CreateTextureFromSurface(renderer, boostSurface);
            if (boostTexture) {
                SDL_Rect boostRect = { windowWidth - 150, 50, boostSurface->w, boostSurface->h };
                SDL_RenderCopy(renderer, boostTexture, nullptr, &boostRect);
                SDL_DestroyTexture(boostTexture);
            }
            SDL_FreeSurface(boostSurface);
        }
    }

    if (askingForName) {
        std::string prompt = "Enter your name:";
        SDL_Surface* promptSurface = TTF_RenderText_Solid(font, prompt.c_str(), textColor);
        if (promptSurface) {
            SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface);
            if (promptTexture) {
                SDL_Rect promptRect = { windowWidth / 2 - 100, windowHeight / 2 + 25, promptSurface->w, promptSurface->h };
                SDL_RenderCopy(renderer, promptTexture, nullptr, &promptRect);
                SDL_DestroyTexture(promptTexture);
            }
            SDL_FreeSurface(promptSurface);
        }

        SDL_Surface* nameSurface = TTF_RenderText_Solid(font, inputText.c_str(), textColor);
        if (nameSurface) {
            SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
            if (nameTexture) {
                SDL_Rect nameRect = { windowWidth / 2 - 100, windowHeight / 2 + 50, nameSurface->w, nameSurface->h };
                SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
                SDL_DestroyTexture(nameTexture);
            }
            SDL_FreeSurface(nameSurface);
        }
    }

    if (isPaused) {
        SDL_Color pauseColor = { 255, 255, 0, 255 };
        SDL_Surface* pauseSurface = TTF_RenderText_Solid(font, "Paused", pauseColor);
        if (pauseSurface) {
            SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface);
            if (pauseTexture) {
                SDL_Rect pauseRect = { windowWidth / 2 - 50, windowHeight / 2 - 50, pauseSurface->w, pauseSurface->h };
                SDL_RenderCopy(renderer, pauseTexture, nullptr, &pauseRect);
                SDL_DestroyTexture(pauseTexture);
            }
            SDL_FreeSurface(pauseSurface);
        }
    }
}

void Engine::resetSnakeGame() {
    snakeGameActive = false;
    gameOver = false;
    isPaused = false;
    askingForName = false;
    showingScoreboard = false;
    showConfetti = false;
    confettiParticles.clear();
    snakeBody.clear();
    currentMode = MODE_NONE;
}

void Engine::drawTextBox() {
    SDL_Rect textBoxRect = { windowWidth / 2 - 400, windowHeight / 2 - 150, 800, 300 };
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &textBoxRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &textBoxRect);

    if (showHelp) {
        std::vector<std::string> helpText = {
            "Shortcut Keys:", "C: Open/Close Chat Box", "X: Return to Main Menu", "E: Toggle Earth Gravity",
            "M: Toggle Moon Gravity", "R: Change Rectangle Color to Red", "G: Change Rectangle Color to Green",
            "B: Change Rectangle Color to Blue", "S: Pause/Resume Game", "Arrow Keys: Move Snake",
            "Backspace: Delete Last Character in Chat", "Type 'play mode1' for Mode 1 (120s, 20 food)",
            "Type 'play mode2' for Mode 2 (unlimited)", "Type 'play mode3 time x food y' for Mode 3",
            "Leaderboard:", "Mode 1 (Fastest Time, Most Food):"
        };

        for (size_t i = 0; i < std::min(mode1Scores.size(), static_cast<size_t>(5)); i++) {
            helpText.push_back(std::to_string(i + 1) + " | " + mode1Scores[i].playerName + " | " + std::to_string(mode1Scores[i].food) + " | " + std::to_string(mode1Scores[i].time) + "s");
        }
        helpText.push_back("Mode 2 (Most Food):");
        for (size_t i = 0; i < std::min(mode2Scores.size(), static_cast<size_t>(5)); i++) {
            helpText.push_back(std::to_string(i + 1) + " | " + mode2Scores[i].playerName + " | " + std::to_string(mode2Scores[i].food) + " | --");
        }

        SDL_Color textColor = { 255, 255, 255, 255 };
        int lineHeight = 20;
        int startY = windowHeight / 2 - 130;

        for (size_t i = 0; i < helpText.size(); i++) {
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, helpText[i].c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { windowWidth / 2 - 390, startY + static_cast<int>(i) * lineHeight, textSurface->w, textSurface->h };
                    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
    }
    else {
        std::string displayText = inputText;
        int maxCharsPerLine = 96;
        int lineHeight = 20;

        for (size_t i = 0; i < displayText.length(); i += maxCharsPerLine) {
            std::string line = displayText.substr(i, maxCharsPerLine);
            SDL_Color textColor = { 255, 255, 255, 255 };
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { windowWidth / 2 - 390, windowHeight / 2 - 30 + static_cast<int>(i / maxCharsPerLine) * lineHeight, textSurface->w, textSurface->h };
                    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
    }
}

void Engine::renderScoreboard() {
    SDL_Color textColor = { 255, 255, 255, 255 };
    TTF_Font* boldFont = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!boldFont) boldFont = font;
    TTF_SetFontStyle(boldFont, TTF_STYLE_BOLD);

    std::vector<std::string> leaderboard;
    leaderboard.push_back(" | Rank | Player         | Food | Time  |");
    leaderboard.push_back("-----------------------------------------");

    if (currentMode == MODE_1) {
        int timeTaken = 120 - timer;
        bool found = false;
        for (size_t i = 0; i < std::min(mode1Scores.size(), static_cast<size_t>(5)); ++i) {
            std::string entry = " | " + std::string(5 - std::to_string(i + 1).length(), ' ') + std::to_string(i + 1) + " | " +
                mode1Scores[i].playerName + std::string(15 - mode1Scores[i].playerName.length(), ' ') + " | " +
                std::to_string(mode1Scores[i].food) + std::string(5 - std::to_string(mode1Scores[i].food).length(), ' ') + " | " +
                std::to_string(mode1Scores[i].time) + "s" + std::string(6 - std::to_string(mode1Scores[i].time).length(), ' ');

            if (mode1Scores[i].food == 20 && mode1Scores[i].playerName == inputText && mode1Scores[i].time == timeTaken) {
                entry += " (You)";
                found = true;
                SDL_Surface* boldSurface = TTF_RenderText_Solid(boldFont, entry.c_str(), textColor);
                if (boldSurface) {
                    SDL_Texture* boldTexture = SDL_CreateTextureFromSurface(renderer, boldSurface);
                    SDL_Rect textRect = { windowWidth / 2 - boldSurface->w / 2, windowHeight / 2 + static_cast<int>(i + 2) * 20, boldSurface->w, boldSurface->h };
                    SDL_RenderCopy(renderer, boldTexture, nullptr, &textRect);
                    SDL_DestroyTexture(boldTexture);
                    SDL_FreeSurface(boldSurface);
                }
            }
            else {
                SDL_Surface* surface = TTF_RenderText_Solid(font, entry.c_str(), textColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_Rect textRect = { windowWidth / 2 - surface->w / 2, windowHeight / 2 + static_cast<int>(i + 2) * 20, surface->w, surface->h };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                    SDL_FreeSurface(surface);
                }
            }
        }
        if (!found) {
            std::string yourEntry = " | " + std::string(5 - std::to_string(mode1Scores.size() + 1).length(), ' ') + std::to_string(mode1Scores.size() + 1) + " | " +
                inputText + std::string(15 - inputText.length(), ' ') + " | " +
                std::to_string(score) + std::string(5 - std::to_string(score).length(), ' ') + " | " +
                std::to_string(120 - timer) + "s" + std::string(6 - std::to_string(120 - timer).length(), ' ') + " (You)";
            SDL_Surface* boldSurface = TTF_RenderText_Solid(boldFont, yourEntry.c_str(), textColor);
            if (boldSurface) {
                SDL_Texture* boldTexture = SDL_CreateTextureFromSurface(renderer, boldSurface);
                SDL_Rect textRect = { windowWidth / 2 - boldSurface->w / 2, windowHeight / 2 + static_cast<int>(mode1Scores.size() + 2) * 20, boldSurface->w, boldSurface->h };
                SDL_RenderCopy(renderer, boldTexture, nullptr, &textRect);
                SDL_DestroyTexture(boldTexture);
                SDL_FreeSurface(boldSurface);
            }
        }
    }
    else if (currentMode == MODE_2) {
        bool found = false;
        for (size_t i = 0; i < std::min(mode2Scores.size(), static_cast<size_t>(5)); ++i) {
            std::string entry = " | " + std::string(5 - std::to_string(i + 1).length(), ' ') + std::to_string(i + 1) + " | " +
                mode2Scores[i].playerName + std::string(15 - mode2Scores[i].playerName.length(), ' ') + " | " +
                std::to_string(mode2Scores[i].food) + std::string(5 - std::to_string(mode2Scores[i].food).length(), ' ') + " | --";

            if (mode2Scores[i].playerName == inputText && mode2Scores[i].food == score) {
                entry += " (You)";
                found = true;
                SDL_Surface* boldSurface = TTF_RenderText_Solid(boldFont, entry.c_str(), textColor);
                if (boldSurface) {
                    SDL_Texture* boldTexture = SDL_CreateTextureFromSurface(renderer, boldSurface);
                    SDL_Rect textRect = { windowWidth / 2 - boldSurface->w / 2, windowHeight / 2 + static_cast<int>(i + 2) * 20, boldSurface->w, boldSurface->h };
                    SDL_RenderCopy(renderer, boldTexture, nullptr, &textRect);
                    SDL_DestroyTexture(boldTexture);
                    SDL_FreeSurface(boldSurface);
                }
            }
            else {
                SDL_Surface* surface = TTF_RenderText_Solid(font, entry.c_str(), textColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_Rect textRect = { windowWidth / 2 - surface->w / 2, windowHeight / 2 + static_cast<int>(i + 2) * 20, surface->w, surface->h };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                    SDL_FreeSurface(surface);
                }
            }
        }
        if (!found) {
            std::string yourEntry = " | " + std::string(5 - std::to_string(mode2Scores.size() + 1).length(), ' ') + std::to_string(mode2Scores.size() + 1) + " | " +
                inputText + std::string(15 - inputText.length(), ' ') + " | " +
                std::to_string(score) + std::string(5 - std::to_string(score).length(), ' ') + " | -- (You)";
            SDL_Surface* boldSurface = TTF_RenderText_Solid(boldFont, yourEntry.c_str(), textColor);
            if (boldSurface) {
                SDL_Texture* boldTexture = SDL_CreateTextureFromSurface(renderer, boldSurface);
                SDL_Rect textRect = { windowWidth / 2 - boldSurface->w / 2, windowHeight / 2 + static_cast<int>(mode2Scores.size() + 2) * 20, boldSurface->w, boldSurface->h };
                SDL_RenderCopy(renderer, boldTexture, nullptr, &textRect);
                SDL_DestroyTexture(boldTexture);
                SDL_FreeSurface(boldSurface);
            }
        }
    }

    std::string returnText = "Press Enter or X to return";
    SDL_Surface* returnSurface = TTF_RenderText_Solid(font, returnText.c_str(), textColor);
    if (returnSurface) {
        SDL_Texture* returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
        SDL_Rect returnRect = { windowWidth / 2 - returnSurface->w / 2, windowHeight / 2 + static_cast<int>(std::max(mode1Scores.size(), mode2Scores.size()) + 3) * 20, returnSurface->w, returnSurface->h };
        SDL_RenderCopy(renderer, returnTexture, nullptr, &returnRect);
        SDL_DestroyTexture(returnTexture);
        SDL_FreeSurface(returnSurface);
    }

    if (boldFont != font) TTF_CloseFont(boldFont);
}

void Engine::updateConfetti() {
    if (!showConfetti) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - confettiStartTime > 2000) {
        showConfetti = false;
        confettiParticles.clear();
        return;
    }

    for (auto& particle : confettiParticles) {
        particle.x += particle.velocityX * deltaTime * 60.0f;
        particle.y += particle.velocityY * deltaTime * 60.0f;
        particle.velocityY += 0.1f * deltaTime * 60.0f;

        if (particle.x < 0) particle.x = 0;
        if (static_cast<int>(particle.x) > windowWidth) particle.x = static_cast<float>(windowWidth);
        if (static_cast<int>(particle.y) > windowHeight) particle.y = static_cast<float>(windowHeight);
    }
}

void Engine::renderConfetti() {
    for (const auto& particle : confettiParticles) {
        SDL_SetRenderDrawColor(renderer, particle.color.r, particle.color.g, particle.color.b, particle.color.a);
        SDL_Rect rect = {
            static_cast<int>(particle.x),
            static_cast<int>(particle.y),
            5,
            5
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Engine::setBackgroundColor(const std::string& colorName) {
    if (colorName == "white") backgroundColor = { 255, 255, 255, 255 };
    else if (colorName == "black") backgroundColor = { 0, 0, 0, 255 };
    else if (colorName == "red") backgroundColor = { 255, 0, 0, 255 };
    else if (colorName == "green") backgroundColor = { 0, 255, 0, 255 };
    else if (colorName == "blue") backgroundColor = { 0, 0, 255, 255 };
}

void Engine::update() {
    Uint32 currentTime = SDL_GetTicks();
    deltaTime = (currentTime - lastUpdateTime) / 1000.0f;
    lastUpdateTime = currentTime;

    if (snakeGameActive && !showingScoreboard) {
        updateSnakeGame();
    }
    else if (gravityMode) {
        velocityY += gravityAcceleration * deltaTime * 60.0f;
        rectY += static_cast<int>(velocityY * deltaTime * 60.0f);

        if (rectY + rectHeight >= windowHeight) {
            rectY = windowHeight - rectHeight;
            velocityY = -velocityY * bounceFactor;
            if (std::abs(velocityY) < 1.0f) {
                velocityY = 0;
                isOnGround = true;
            }
        }

        if (isOnGround) {
            velocityY *= (1.0f - groundFriction * deltaTime * 60.0f);
        }
        else {
            isOnGround = false;
        }
    }

    if (showConfetti) {
        updateConfetti();
    }

    frameCount++;
    if (currentTime - lastFPSUpdateTime >= 1000) {
        fps = frameCount;
        frameCount = 0;
        lastFPSUpdateTime = currentTime;
    }
}

void Engine::render() {
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    if (showingScoreboard) {
        renderScoreboard();
    }
    else if (snakeGameActive) {
        renderSnakeGame();
    }
    else {
        SDL_Rect rect = { rectX, rectY, rectWidth, rectHeight };
        SDL_SetRenderDrawColor(renderer, rectColor.r, rectColor.g, rectColor.b, rectColor.a);
        SDL_RenderFillRect(renderer, &rect);

        SDL_Color textColor = gravityMode ? SDL_Color{ 255, 0, 0, 255 } : SDL_Color{ 255, 255, 255, 255 };
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, gravityText.c_str(), textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = { windowWidth - 200, 10, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }

    if (showTextBox) {
        drawTextBox();
    }

    if (showConfetti) {
        renderConfetti();
    }

    SDL_RenderPresent(renderer);
}

void Engine::saveScore() {
    if (currentMode == MODE_1) {
        int timeTaken = 120 - timer;
        mode1Scores.push_back({ inputText, score, timeTaken });
        std::sort(mode1Scores.begin(), mode1Scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return (a.food == 20 && b.food == 20) ? (a.time < b.time) : (a.food > b.food);
            });
    }
    else if (currentMode == MODE_2) {
        mode2Scores.push_back({ inputText, score, 0 });
        std::sort(mode2Scores.begin(), mode2Scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.food > b.food;
            });
    }

    std::ofstream file("scores.txt");
    if (file.is_open()) {
        file << "Mode 1 Scores:\n";
        for (const auto& entry : mode1Scores) {
            file << entry.playerName << " " << entry.food << " " << entry.time << "\n";
        }
        file << "Mode 2 Scores:\n";
        for (const auto& entry : mode2Scores) {
            file << entry.playerName << " " << entry.food << "\n";
        }
        file.close();
    }
}

void Engine::loadScores() {
    std::ifstream file("scores.txt");
    if (file.is_open()) {
        std::string line;
        bool mode1Section = false;
        while (std::getline(file, line)) {
            if (line == "Mode 1 Scores:") {
                mode1Section = true;
                continue;
            }
            else if (line == "Mode 2 Scores:") {
                mode1Section = false;
                continue;
            }

            std::istringstream iss(line);
            std::string name;
            int food, time = 0;
            if (mode1Section) {
                if (iss >> name >> food >> time) {
                    mode1Scores.push_back({ name, food, time });
                }
            }
            else {
                if (iss >> name >> food) {
                    mode2Scores.push_back({ name, food, 0 });
                }
            }
        }
        file.close();

        std::sort(mode1Scores.begin(), mode1Scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return (a.food == 20 && b.food == 20) ? (a.time < b.time) : (a.food > b.food);
            });

        std::sort(mode2Scores.begin(), mode2Scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.food > b.food;
            });
    }
}

void Engine::showScoreboard() {
    std::cout << "Mode 1 Leaderboard (Fastest Time, Most Food):\n";
    for (size_t i = 0; i < std::min(mode1Scores.size(), static_cast<size_t>(5)); i++) {
        std::cout << i + 1 << " | " << mode1Scores[i].playerName << " | " << mode1Scores[i].food << " | " << mode1Scores[i].time << "s\n";
    }
    std::cout << "Mode 2 Leaderboard (Most Food):\n";
    for (size_t i = 0; i < std::min(mode2Scores.size(), static_cast<size_t>(5)); i++) {
        std::cout << i + 1 << " | " << mode2Scores[i].playerName << " | " << mode2Scores[i].food << " | --\n";
    }
}

void Engine::moveSnake() {
    if (!snakeGameActive || gameOver || isPaused) return;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 moveInterval = (snakeSpeed == snakeBoostedSpeed) ? 20 : 40;
    if (currentTime - lastSnakeMoveTime < moveInterval) return;
    lastSnakeMoveTime = currentTime;

    int newX = snakeBody[0].x;
    int newY = snakeBody[0].y;

    int stepSize = 4;
    switch (snakeDirection) {
    case UP: newY -= stepSize; break;
    case DOWN: newY += stepSize; break;
    case LEFT: newX -= stepSize; break;
    case RIGHT: newX += stepSize; break;
    }

    snakeBody.insert(snakeBody.begin(), { newX, newY });

    if (abs(newX - foodPosition.x) < 10 && abs(newY - foodPosition.y) < 10) {
        spawnFood();
        score++;
    }
    else {
        snakeBody.pop_back();
    }

    checkCollision();
}

void Engine::spawnFood() {
    foodPosition.x = rand() % (windowWidth - 10);
    foodPosition.y = rand() % (windowHeight - 10);
}

void Engine::checkCollision() {
    SDL_Point head = snakeBody[0];

    if (head.x < 0 || head.x >= windowWidth || head.y < 0 || head.y >= windowHeight) {
        gameOver = true;
        return;
    }

    for (size_t i = 1; i < snakeBody.size(); i++) {
        if (head.x == snakeBody[i].x && head.y == snakeBody[i].y) {
            gameOver = true;
            return;
        }
    }
}