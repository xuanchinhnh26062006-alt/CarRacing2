#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

enum class GameState
{
    MainMenu,
    Playing,
    Paused,
    GameOver
};

class GameManager
{
public:
    GameManager();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    void render();

    void loadConfig(const std::string& filename);
    void loadResources();
    void setupTexts();
    void setupRoad();

    void spawnEnemy();
    void checkCollisions();
    void resetGame();

    void loadScoreboard();
    void saveScoreboard();

    void startCountdown();

private:
    sf::RenderWindow mWindow;
    Player mPlayer;
    sf::Music mBackgroundMusic;

    sf::Texture mRoadTexture;
    sf::Sprite mRoadSprite1;
    sf::Sprite mRoadSprite2;
    std::string mRoadTextureFile;

    float mRoadPadding;

    std::vector<sf::Texture> mEnemyTextures;
    std::vector<Enemy> mEnemies;
    std::multimap<int, std::string, std::greater<int>> mScoreboard;

    float mRoadSpeed;

    sf::Font mFont;
    sf::Text mScoreText;

    // --- Văn bản Game Over ---
    sf::Text mGameOverText;       // "GAME OVER!"
    sf::Text mGameOverPromptText; // "Play Again?"
    sf::Text mGameOverYesText;    // "YES"
    sf::Text mGameOverNoText;     // "NO"
    int mGameOverSelection;       // 0 = YES, 1 = NO
    // ---

    sf::RectangleShape mGameOverFrame;

    sf::Text mTitleText;
    sf::Text mMenuText;

    sf::Text mPauseText;
    sf::Text mCountdownText;
    sf::Clock mCountdownClock;
    float mCountdownTime;

    GameState mCurrentState;

    int mScore;

    int mWindowWidth;
    int mWindowHeight;
    float mEnemySpeedMin;
    float mEnemySpeedMax;
    float mSpawnInterval;
    sf::Time mTimeSinceLastSpawn;

    float mBaseRoadSpeed;
    float mBaseEnemySpeedMin;
    float mBaseEnemySpeedMax;
    float mBaseSpawnInterval;

    int mNextLevelScore;

    bool mStartHover; // <<< THÊM MỚI: Biến theo dõi hover nút Start
};