#include "GameManager.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <string> 

using namespace std;

GameManager::GameManager()
    : mWindow()
    , mPlayer(400.f)
    , mScore(0)
    , mTimeSinceLastSpawn(sf::Time::Zero)
    , mRoadSpeed(300.f)
    , mRoadPadding(124.f)
    , mNextLevelScore(10)
    , mCurrentState(GameState::MainMenu)
    , mCountdownTime(0.f)
    , mGameOverSelection(0)
    , mStartHover(false)
{
    loadConfig("config.ini");

    mWindow.create(sf::VideoMode(mWindowWidth, mWindowHeight), "Car Racing");
    mWindow.setFramerateLimit(60);

    srand(static_cast<unsigned int>(time(0)));

    loadResources();
    setupRoad();
    setupTexts();
    loadScoreboard();
}

void GameManager::loadConfig(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw runtime_error("CLO4 Exception: Config file not found! (" + filename + ")");
    }

    string line;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        stringstream ss(line);
        string key;
        float value;
        string s_value;

        if (getline(ss, key, '='))
        {
            if (key == "road_texture_file")
            {
                getline(ss, mRoadTextureFile);
            }
            else if (ss >> value)
            {
                if (key == "width") mWindowWidth = static_cast<int>(value);
                else if (key == "height") mWindowHeight = static_cast<int>(value);
                else if (key == "road_padding") mRoadPadding = value;
                else if (key == "road_speed")
                {
                    mBaseRoadSpeed = value;
                    mRoadSpeed = value;
                }
                else if (key == "enemy_speed_min")
                {
                    mBaseEnemySpeedMin = value;
                    mEnemySpeedMin = value;
                }
                else if (key == "enemy_speed_max")
                {
                    mBaseEnemySpeedMax = value;
                    mEnemySpeedMax = value;
                }
                else if (key == "spawn_interval")
                {
                    mBaseSpawnInterval = value;
                    mSpawnInterval = value;
                }
            }
        }
    }
    file.close();
}


void GameManager::loadResources()
{
    if (!mFont.loadFromFile("arial.ttf"))
    {
        throw runtime_error("CLO4 Exception: Font file not found (arial.ttf)");
    }

    mPlayer.initTexture("player_car.png");

    sf::Texture tex1;
    if (!tex1.loadFromFile("enemy_car_1.png"))
    {
        throw runtime_error("CLO4 Exception: File Not Found: enemy_car_1.png");
    }
    mEnemyTextures.push_back(tex1);
    sf::Texture tex2;
    if (!tex2.loadFromFile("enemy_car_2.png"))
    {
        throw runtime_error("CLO4 Exception: File Not Found: enemy_car_2.png");
    }
    mEnemyTextures.push_back(tex2);

    if (mRoadTextureFile.empty())
    {
        throw runtime_error("CLO4 Exception: road_texture_file not specified in config.ini");
    }
    if (!mRoadTexture.loadFromFile(mRoadTextureFile))
    {
        throw runtime_error("CLO4 Exception: File Not Found: " + mRoadTextureFile);
    }

    if (!mBackgroundMusic.openFromFile("car-beat-349137.mp3"))
    {
        throw runtime_error("CLO4 Exception: File Not Found: car-beat-349137.mp3");
    }

    mBackgroundMusic.setLoop(true);
    mBackgroundMusic.setVolume(50);
    mBackgroundMusic.play();
}


void GameManager::setupRoad()
{
    float scaleX = (float)mWindowWidth / mRoadTexture.getSize().x;
    float roadTextureHeight = mRoadTexture.getSize().y * scaleX;

    mRoadSprite1.setTexture(mRoadTexture);
    mRoadSprite1.setPosition(0.f, 0.f);
    mRoadSprite1.setScale(scaleX, scaleX);

    mRoadSprite2.setTexture(mRoadTexture);
    mRoadSprite2.setPosition(0.f, -roadTextureHeight);
    mRoadSprite2.setScale(scaleX, scaleX);
}


void GameManager::setupTexts()
{
    mScoreText.setFont(mFont);
    mScoreText.setCharacterSize(24);
    mScoreText.setFillColor(sf::Color::White);
    mScoreText.setPosition(10.f, 10.f);

    mGameOverText.setFont(mFont);
    mGameOverText.setCharacterSize(60);
    mGameOverText.setFillColor(sf::Color::Red);
    mGameOverText.setOutlineColor(sf::Color(255, 100, 100));
    mGameOverText.setOutlineThickness(2.f);
    mGameOverText.setString("GAME OVER!");
    mGameOverText.setStyle(sf::Text::Bold);
    sf::FloatRect goRect = mGameOverText.getLocalBounds();
    mGameOverText.setOrigin(goRect.left + goRect.width / 2.0f, goRect.top + goRect.height / 2.0f);
    mGameOverText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f - 80.f);

    mGameOverPromptText.setFont(mFont);
    mGameOverPromptText.setCharacterSize(30);
    mGameOverPromptText.setFillColor(sf::Color::White);
    mGameOverPromptText.setString("Play Again?");
    sf::FloatRect promptRect = mGameOverPromptText.getLocalBounds();
    mGameOverPromptText.setOrigin(promptRect.left + promptRect.width / 2.0f, promptRect.top + promptRect.height / 2.0f);
    mGameOverPromptText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f - 10.f);

    mGameOverYesText.setFont(mFont);
    mGameOverYesText.setCharacterSize(30);
    mGameOverYesText.setFillColor(sf::Color::Yellow);
    mGameOverYesText.setString("YES");
    sf::FloatRect yesRect = mGameOverYesText.getLocalBounds();
    mGameOverYesText.setOrigin(yesRect.left + yesRect.width / 2.0f, yesRect.top + yesRect.height / 2.0f);
    mGameOverYesText.setPosition(mWindowWidth / 2.0f - 60.f, mWindowHeight / 2.0f + 50.f);

    mGameOverNoText.setFont(mFont);
    mGameOverNoText.setCharacterSize(30);
    mGameOverNoText.setFillColor(sf::Color::White);
    mGameOverNoText.setString("NO");
    sf::FloatRect noRect = mGameOverNoText.getLocalBounds();
    mGameOverNoText.setOrigin(noRect.left + noRect.width / 2.0f, noRect.top + noRect.height / 2.0f);
    mGameOverNoText.setPosition(mWindowWidth / 2.0f + 60.f, mWindowHeight / 2.0f + 50.f);

    float padding = 30.f;
    mGameOverFrame.setSize(sf::Vector2f(goRect.width + padding * 4, 250.f));
    mGameOverFrame.setOrigin(mGameOverFrame.getSize().x / 2.0f, mGameOverFrame.getSize().y / 2.0f);
    mGameOverFrame.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f);
    mGameOverFrame.setFillColor(sf::Color(0, 0, 0, 150));
    mGameOverFrame.setOutlineColor(sf::Color::Red);
    mGameOverFrame.setOutlineThickness(5.f);

    mTitleText.setFont(mFont);
    mTitleText.setCharacterSize(60);
    mTitleText.setFillColor(sf::Color::White);
    mTitleText.setString("CAR RACING");
    sf::FloatRect titleRect = mTitleText.getLocalBounds();
    mTitleText.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    mTitleText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f - 100.f);

    mMenuText.setFont(mFont);
    mMenuText.setCharacterSize(40);
    mMenuText.setFillColor(sf::Color::White);
    mMenuText.setString("START");
    sf::FloatRect menuRect = mMenuText.getLocalBounds();
    mMenuText.setOrigin(menuRect.left + menuRect.width / 2.0f, menuRect.top + menuRect.height / 2.0f);
    mMenuText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f + 50.f);

    mPauseText.setFont(mFont);
    mPauseText.setCharacterSize(50);
    mPauseText.setFillColor(sf::Color::Yellow);
    mPauseText.setString("PAUSED");
    mPauseText.setStyle(sf::Text::Bold);
    sf::FloatRect pauseRect = mPauseText.getLocalBounds();
    mPauseText.setOrigin(pauseRect.left + pauseRect.width / 2.0f, pauseRect.top + pauseRect.height / 2.0f);
    mPauseText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f);

    mCountdownText.setFont(mFont);
    mCountdownText.setCharacterSize(80);
    mCountdownText.setFillColor(sf::Color::Green);
    mCountdownText.setStyle(sf::Text::Bold);
}

void GameManager::resetGame()
{
    mCurrentState = GameState::Playing;

    mScore = 0;
    mScoreText.setString("Score: 0");
    mTimeSinceLastSpawn = sf::Time::Zero;
    mEnemies.clear();

    float playerStartX = mRoadPadding + ((mWindowWidth - mRoadPadding * 2) / 2.f);
    mPlayer.setPosition(playerStartX, mWindowHeight - mPlayer.getGlobalBounds().height * 1.5f);

    mRoadSpeed = mBaseRoadSpeed;
    mEnemySpeedMin = mBaseEnemySpeedMin;
    mEnemySpeedMax = mBaseEnemySpeedMax;
    mSpawnInterval = mBaseSpawnInterval;
    mNextLevelScore = 10;

    mGameOverSelection = 0;
    mStartHover = false;

    mBackgroundMusic.play();
}


void GameManager::run()
{
    sf::Clock clock;
    while (mWindow.isOpen())
    {
        sf::Time dt = clock.restart();
        processEvents();

        if (mCurrentState == GameState::Playing)
        {
            update(dt);
        }
        else if (mCurrentState == GameState::Paused && mCountdownTime > 0)
        {
            mCountdownTime -= mCountdownClock.restart().asSeconds();
            if (mCountdownTime <= 0)
            {
                mCountdownTime = 0;
                mCurrentState = GameState::Playing;
                mBackgroundMusic.play();
            }
        }

        render();
    }
}


void GameManager::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            mWindow.close();

        // --- XỬ LÝ BÀN PHÍM ---
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Enter)
            {
                if (mCurrentState == GameState::Playing)
                {
                    mCurrentState = GameState::Paused;
                    mCountdownTime = 0;
                    mBackgroundMusic.pause();
                }
                else if (mCurrentState == GameState::Paused)
                {
                    if (mCountdownTime > 0) mCountdownTime = 0;
                    else startCountdown();
                }
                else if (mCurrentState == GameState::MainMenu)
                {
                    resetGame();
                }
                else if (mCurrentState == GameState::GameOver)
                {
                    if (mGameOverSelection == 0) resetGame();
                    else mCurrentState = GameState::MainMenu;
                }
            }

            if (mCurrentState == GameState::GameOver)
            {
                if (event.key.code == sf::Keyboard::Left)
                {
                    mGameOverSelection = 0; // Chọn YES
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    mGameOverSelection = 1; // Chọn NO
                }
            }
        }

        // --- XỬ LÝ DI CHUỘT (HOVER) ---
        if (event.type == sf::Event::MouseMoved)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(mWindow);
            sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

            if (mCurrentState == GameState::MainMenu)
            {
                if (mMenuText.getGlobalBounds().contains(mousePosF))
                {
                    mStartHover = true;
                }
                else
                {
                    mStartHover = false;
                }
            }
            else if (mCurrentState == GameState::GameOver)
            {
                if (mGameOverYesText.getGlobalBounds().contains(mousePosF))
                {
                    mGameOverSelection = 0;
                }
                else if (mGameOverNoText.getGlobalBounds().contains(mousePosF))
                {
                    mGameOverSelection = 1;
                }
            }
        }

        // --- XỬ LÝ CLICK CHUỘT ---
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(mWindow);
                sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                if (mCurrentState == GameState::MainMenu)
                {
                    if (mMenuText.getGlobalBounds().contains(mousePosF))
                    {
                        resetGame();
                    }
                }
                else if (mCurrentState == GameState::GameOver)
                {
                    if (mGameOverYesText.getGlobalBounds().contains(mousePosF))
                    {
                        resetGame();
                    }
                    else if (mGameOverNoText.getGlobalBounds().contains(mousePosF))
                    {
                        mCurrentState = GameState::MainMenu;
                    }
                }
            }
        }
    }
}

void GameManager::startCountdown()
{
    mCountdownTime = 3.5f;
    mCountdownClock.restart();
}


void GameManager::update(sf::Time dt)
{
    float roadTextureHeight = mRoadTexture.getSize().y * mRoadSprite1.getScale().y;

    mRoadSprite1.move(0, mRoadSpeed * dt.asSeconds());
    mRoadSprite2.move(0, mRoadSpeed * dt.asSeconds());

    if (mRoadSprite1.getPosition().y > mWindowHeight)
    {
        mRoadSprite1.setPosition(0.f, mRoadSprite2.getPosition().y - roadTextureHeight);
    }
    if (mRoadSprite2.getPosition().y > mWindowHeight)
    {
        mRoadSprite2.setPosition(0.f, mRoadSprite1.getPosition().y - roadTextureHeight);
    }

    mPlayer.update(dt);
    sf::Vector2f playerPos = mPlayer.getPosition();

    sf::FloatRect playerBounds = mPlayer.getGlobalBounds();
    float playerHalfWidth = playerBounds.width / 2.f;
    float playerHalfHeight = playerBounds.height / 2.f;

    float leftLimit = mRoadPadding + playerHalfWidth;
    float rightLimit = mWindowWidth - mRoadPadding - playerHalfWidth;

    if (playerPos.x < leftLimit)
        playerPos.x = leftLimit;
    if (playerPos.x > rightLimit)
        playerPos.x = rightLimit;

    float topLimit = 0 + playerHalfHeight;
    float bottomLimit = mWindowHeight - playerHalfHeight;

    if (playerPos.y < topLimit)
        playerPos.y = topLimit;
    if (playerPos.y > bottomLimit)
        playerPos.y = bottomLimit;

    mPlayer.setPosition(playerPos.x, playerPos.y);


    mTimeSinceLastSpawn += dt;
    if (mTimeSinceLastSpawn.asSeconds() > mSpawnInterval)
    {
        spawnEnemy();
        mTimeSinceLastSpawn = sf::Time::Zero;
    }

    for (auto it = mEnemies.begin(); it != mEnemies.end(); )
    {
        it->update(dt);
        if (it->getPosition().y > mWindowHeight + it->getGlobalBounds().height)
        {
            it = mEnemies.erase(it);
            mScore++;

            if (mScore >= mNextLevelScore)
            {
                mRoadSpeed *= 1.1f;
                mEnemySpeedMin *= 1.1f;
                mEnemySpeedMax *= 1.1f;
                if (mSpawnInterval > 0.4f)
                {
                    mSpawnInterval *= 0.95f;
                }
                mNextLevelScore += 10;
            }
        }
        else
        {
            ++it;
        }
    }

    mScoreText.setString("Score: " + to_string(mScore));

    checkCollisions();
}


void GameManager::render()
{
    mWindow.clear(sf::Color::Black);

    mWindow.draw(mRoadSprite1);
    mWindow.draw(mRoadSprite2);

    if (mCurrentState == GameState::Playing)
    {
        mPlayer.draw(mWindow);
        for (auto& enemy : mEnemies)
        {
            enemy.draw(mWindow);
        }
        mWindow.draw(mScoreText);
    }
    else if (mCurrentState == GameState::MainMenu)
    {
        if (mStartHover)
        {
            mMenuText.setFillColor(sf::Color::Yellow);
        }
        else
        {
            mMenuText.setFillColor(sf::Color::White);
        }

        mWindow.draw(mTitleText);
        mWindow.draw(mMenuText);
    }
    else if (mCurrentState == GameState::Paused)
    {
        mPlayer.draw(mWindow);
        for (auto& enemy : mEnemies)
        {
            enemy.draw(mWindow);
        }
        mWindow.draw(mScoreText);

        if (mCountdownTime > 0)
        {
            int displayTime = static_cast<int>(ceil(mCountdownTime - 0.5f));
            if (displayTime > 0)
            {
                mCountdownText.setString(to_string(displayTime));
                sf::FloatRect cdRect = mCountdownText.getLocalBounds();
                mCountdownText.setOrigin(cdRect.left + cdRect.width / 2.0f, cdRect.top + cdRect.height / 2.0f);
                mCountdownText.setPosition(mWindowWidth / 2.0f, mWindowHeight / 2.0f);
                mWindow.draw(mCountdownText);
            }
        }
        else
        {
            // <<< LỖI Ở ĐÂY (đã sửa)
            mWindow.draw(mPauseText);
        }
    }
    else if (mCurrentState == GameState::GameOver)
    {
        mPlayer.draw(mWindow);
        for (auto& enemy : mEnemies)
        {
            enemy.draw(mWindow);
        }
        mWindow.draw(mScoreText);

        if (mGameOverSelection == 0) // YES được chọn
        {
            mGameOverYesText.setFillColor(sf::Color::Yellow);
            mGameOverNoText.setFillColor(sf::Color::White);
        }
        else // NO được chọn
        {
            mGameOverYesText.setFillColor(sf::Color::White);
            mGameOverNoText.setFillColor(sf::Color::Yellow);
        }

        mWindow.draw(mGameOverFrame);
        mWindow.draw(mGameOverText);
        mWindow.draw(mGameOverPromptText);
        mWindow.draw(mGameOverYesText);
        mWindow.draw(mGameOverNoText);
    }

    mWindow.display();
}

void GameManager::spawnEnemy()
{
    float speed = mEnemySpeedMin + (rand() % static_cast<int>(mEnemySpeedMax - mEnemySpeedMin + 1));

    if (mEnemyTextures.empty()) return;
    int textureIndex = rand() % mEnemyTextures.size();

    float refEnemyWidth = mEnemyTextures[textureIndex].getSize().x;
    float refEnemyHalfWidth = refEnemyWidth / 2.f;

    int spawnableWidth = (mWindowWidth - mRoadPadding * 2) - static_cast<int>(refEnemyWidth);
    float xPos;
    if (spawnableWidth <= 0)
    {
        xPos = mRoadPadding + ((mWindowWidth - mRoadPadding * 2) / 2.f);
    }
    else
    {
        xPos = static_cast<float>(rand() % spawnableWidth) + mRoadPadding + refEnemyHalfWidth;
    }

    for (const auto& enemy : mEnemies)
    {
        float safeDistanceY = refEnemyWidth * 2.0f;
        float safeDistanceX = refEnemyWidth * 1.5f;

        if (enemy.getPosition().y < safeDistanceY)
        {
            if (abs(enemy.getPosition().x - xPos) < safeDistanceX)
            {
                return;
            }
        }
    }

    mEnemies.emplace_back(speed, mEnemyTextures[textureIndex]);
    Enemy& newEnemy = mEnemies.back();
    newEnemy.setPosition(xPos, -newEnemy.getGlobalBounds().height);
}

void GameManager::checkCollisions()
{
    sf::FloatRect playerBounds = mPlayer.getGlobalBounds();
    float paddingX = playerBounds.width * 0.15f;
    float paddingY = playerBounds.height * 0.1f;

    // <<< LỖI Ở ĐÂY (đã sửa)
    sf::FloatRect playerHitbox(
        playerBounds.left + paddingX,
        playerBounds.top + paddingY,
        playerBounds.width - (paddingX * 2),
        playerBounds.height - (paddingY * 2)
    );

    for (const auto& enemy : mEnemies)
    {
        sf::FloatRect enemyBounds = enemy.getGlobalBounds();
        float enemyPaddingX = enemyBounds.width * 0.15f;
        float enemyPaddingY = enemyBounds.height * 0.1f;

        sf::FloatRect enemyHitbox(
            enemyBounds.left + enemyPaddingX,
            enemyBounds.top + enemyPaddingY,
            enemyBounds.width - (enemyPaddingX * 2),
            enemyBounds.height - (enemyPaddingY * 2)
        );

        if (playerHitbox.intersects(enemyHitbox))
        {
            mCurrentState = GameState::GameOver;
            mBackgroundMusic.stop();
            mScoreboard.insert({ mScore, "Player" });
            saveScoreboard();
            break;
        }
    }
}

void GameManager::loadScoreboard()
{
    ifstream file("scoreboard.txt");
    if (!file.is_open())
    {
        return;
    }

    int score;
    string name;
    while (file >> score >> name)
    {
        mScoreboard.insert({ score, name });
    }
    file.close();
}


void GameManager::saveScoreboard()
{
    ofstream file("scoreboard.txt");
    if (!file.is_open())
    {
        cerr << "Warning: Could not save scoreboard." << endl;
        return;
    }

    int count = 0;
    for (const auto& pair : mScoreboard)
    {
        file << pair.first << " " << pair.second << "\n";
        if (++count >= 10) break;
    }
    file.close();
}