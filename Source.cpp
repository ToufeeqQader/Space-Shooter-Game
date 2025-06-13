//----------------------------------------------------Space Shooters-------------------------------------
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream> 
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>  
#include <random> 

using namespace std;

enum class GameState {
    Menu,
    Instructions,
    NameInput,
    Playing,
    Paused,
    GameOver,
    HighScore
};

//---------------------------------- Invader ----------------------------------
class Invader {
protected:
    sf::Sprite sprite;
    sf::Vector2f targetPos;
    float speed = 100.f;
    bool aligned = false;

    int health = 1;

    sf::Texture texture;

    sf::Clock bombTimer;
    float bombCooldown = 5.f; // default for Alpha


    void setupTexture(const std::string& filepath, float scaleSize, float /*interval*/) {
        if (!texture.loadFromFile(filepath)) {
            std::cerr << "[ERROR] Could not load texture: " << filepath << "\n";
        }
        sprite.setTexture(texture);
        sf::Vector2u texSize = texture.getSize();
        sprite.setScale(scaleSize / texSize.x, scaleSize / texSize.y);
    }

public:
    Invader(sf::Vector2f startPos, sf::Vector2f target)
        : targetPos(target)
    {
        sprite.setPosition(startPos);
    }

    virtual ~Invader() {}

    virtual void update(float dt) {
        if (!aligned) {
            sf::Vector2f dir = targetPos - sprite.getPosition();
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist < 1.f) {
                sprite.setPosition(targetPos);
                aligned = true;
            }
            else {
                dir /= dist;
                sprite.move(dir * speed * dt);
            }
        }
    }

    virtual void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    virtual sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    virtual sf::Vector2f getBombPosition() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        return { bounds.left + bounds.width / 2.f, bounds.top + bounds.height };
    }

    bool isAligned() const { return aligned; }

    virtual float getBombSpeed() const { return 180.f; } // default speed

    void takeDamage() {
        health--;
    }

    bool isDead() const {
        return health <= 0;
    }

    virtual bool isBombReady() const {
        return aligned && bombTimer.getElapsedTime().asSeconds() >= bombCooldown;
    }

    virtual void restartBombTimer() {
        bombTimer.restart();
    }


    const sf::Sprite& getSprite() const { return sprite; }
    sf::Sprite& getSprite() { return sprite; }
};

//-----------------------alpha invader-----------------------------
class AlphaInvader : public Invader {

public:
    AlphaInvader(sf::Vector2f startPos, sf::Vector2f targetPos)
        : Invader(startPos, targetPos)
    {
        setupTexture("assets/alpha_invader.png", 50.f, 5.f); // interval now unused
        bombCooldown = 5.f;

    }
};
//-------------------betaInvader-----------------------------------
class BetaInvader : public Invader {
public:
    BetaInvader(sf::Vector2f startPos, sf::Vector2f targetPos)
        : Invader(startPos, targetPos)
    {
        setupTexture("assets/beta_invader.png", 50.f, 3.f);
        health = 2;
        bombCooldown = 3.f;

    }
};

//--------------------gammainavder---------------------------------------
class GammaInvader : public Invader {
private:
    bool isDiving = false;
    sf::Clock diveClock;
    float diveInterval = 5.f;
    float diveSpeed = 100.f;
    float returnSpeed = 80.f;
    float diveDelay = static_cast<float>(rand() % 3 + 1); // 1–3s
    sf::Vector2f originalTarget;

public:
    GammaInvader(sf::Vector2f startPos, sf::Vector2f targetPos)
        : Invader(startPos, targetPos), originalTarget(targetPos)
    {
        setupTexture("assets/gamma_invader.png", 50.f, 2.f);
        health = 2;
        bombCooldown = 2.f;

    }

    void update(float dt) override {
        if (!aligned) {
            Invader::update(dt);
            return;
        }

        float elapsed = diveClock.getElapsedTime().asSeconds();

        if (!isDiving && elapsed > (diveDelay + diveInterval)) {
            isDiving = true;
            diveClock.restart();
        }

        if (isDiving) {
            if (sprite.getPosition().y < originalTarget.y + 150.f) {
                sprite.move(0, diveSpeed * dt);
            }
            else {
                isDiving = false;
                diveClock.restart();
            }
        }
        else if (sprite.getPosition().y > originalTarget.y) {
            sprite.move(0, -returnSpeed * dt);
        }
    }

    float getBombSpeed() const override {
        return 220.f; // Gamma drops faster bombs
    }
};

//----------------------------- Monster Invader -----------------------------
class Monster : public Invader {
private:
    sf::Clock beamClock;
    bool isFiring = false;
    float beamDuration = 1.0f;
    float beamCooldown = 2.0f;
    bool alreadyDodged = false;

    int maxHealth;
    bool isMoving = true;

    sf::Texture lightningTexture;
    sf::Sprite lightningSprite;

    sf::RectangleShape healthBarBack;
    sf::RectangleShape healthBarFront;

    float direction = 1.f;
    float moveSpeed = 100.f;
    float minX = 100.f;
    float maxX = 600.f;

public:
    Monster(sf::Vector2f pos) : Invader(pos, pos)
    {
        setupTexture("assets/monster.png", 180.f, 0);  // Monster texture
        sprite.setPosition(pos);

        health = 30;
        maxHealth = 30;

        // Health bar setup
        healthBarBack.setSize(sf::Vector2f(sprite.getGlobalBounds().width, 8.f));
        healthBarBack.setFillColor(sf::Color::Red);

        healthBarFront.setSize(sf::Vector2f(sprite.getGlobalBounds().width, 8.f));
        healthBarFront.setFillColor(sf::Color::Green);

        // Load lightning beam texture
        if (!lightningTexture.loadFromFile("assets/lightning.png")) {
            std::cerr << "[ERROR] Failed to load lightning.png\n";
        }
        lightningSprite.setTexture(lightningTexture);
        lightningSprite.setScale(0.1f, 0.7f);  // Adjust this based on image size
    }

    void update(float dt) override {
        // Update beam position
        float beamX = sprite.getPosition().x + sprite.getGlobalBounds().width / 2.f - lightningSprite.getGlobalBounds().width / 2.f;
        float beamY = sprite.getPosition().y + sprite.getGlobalBounds().height;
        lightningSprite.setPosition(beamX, beamY);

        // Update health bar
        sf::Vector2f pos = sprite.getPosition();
        float width = sprite.getGlobalBounds().width;
        healthBarBack.setPosition(pos.x, pos.y - 10.f);
        healthBarFront.setPosition(pos.x, pos.y - 10.f);

        float healthPercent = static_cast<float>(health) / maxHealth;
        healthBarFront.setSize(sf::Vector2f(width * healthPercent, 8.f));

        // Beam firing logic
        if (isMoving) {
            sprite.move(direction * moveSpeed * dt, 0.f);
            if (sprite.getPosition().x < minX || sprite.getPosition().x > maxX) {
                direction *= -1.f;
            }
            if (beamClock.getElapsedTime().asSeconds() >= beamCooldown) {
                isMoving = false;
                isFiring = true;
                beamClock.restart();
            }
        }
        else {
            if (beamClock.getElapsedTime().asSeconds() >= beamDuration) {
                isFiring = false;
                isMoving = true;
                beamClock.restart();
            }
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(sprite);
        if (isFiring)
            window.draw(lightningSprite);  // draw the image-based lightning beam

        window.draw(healthBarBack);
        window.draw(healthBarFront);
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    bool isBeamActive() const {
        return isFiring;
    }

    sf::FloatRect getBeamBounds() const {
        return lightningSprite.getGlobalBounds();  // updated to match sprite
    }

    bool hasDodged() const {
        return alreadyDodged;
    }

    void markDodged() {
        alreadyDodged = true;
    }
};

//-----------------------------Screen Base-Class----------------------------
class Screen {
public:
    virtual void handleEvents(sf::RenderWindow& window, GameState& state) = 0;
    virtual void update(GameState& state) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual ~Screen() {}
};

//--------------------MenuScreen----------------------------------------
class MenuScreen : public Screen {
private:
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Texture logoTexture;
    sf::Sprite logoSprite;

    sf::Font font;
    sf::Text title, playText, instructionsText, highScoreText;


public:
    MenuScreen() {
        // Load Background
        backgroundTexture.loadFromFile("assets/menu_bg.png");
        backgroundSprite.setTexture(backgroundTexture);

        // Load Logo
        logoTexture.loadFromFile("assets/space_logo.png");
        logoSprite.setTexture(logoTexture);
        logoSprite.setScale(0.55f, 0.55f);
        logoSprite.setPosition(190.f, 50.f);

        font.loadFromFile("assets/impact.ttf");

        playText.setFont(font);
        playText.setString("Play");
        playText.setCharacterSize(30);
        playText.setFillColor(sf::Color::Green);
        playText.setPosition(290, 300);
        playText.setOutlineColor(sf::Color::Black);
        playText.setOutlineThickness(2);


        instructionsText.setFont(font);
        instructionsText.setString("Instructions");
        instructionsText.setCharacterSize(30);
        instructionsText.setFillColor(sf::Color::Yellow);
        instructionsText.setPosition(290, 350);
        instructionsText.setOutlineColor(sf::Color::Black);
        instructionsText.setOutlineThickness(2);

        highScoreText.setFont(font);
        highScoreText.setString("High Scores");
        highScoreText.setCharacterSize(30);
        highScoreText.setFillColor(sf::Color::Cyan);
        highScoreText.setPosition(290, 400);
        highScoreText.setOutlineColor(sf::Color::Black);
        highScoreText.setOutlineThickness(2);

    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Mouse click handling
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                    if (playText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::NameInput;
                    }
                    else if (instructionsText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::Instructions;
                    }
                    else if (highScoreText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::HighScore;
                    }
                }
            }
        }
    }

    void update(GameState& state) override {

    }

    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(logoSprite);
        window.draw(playText);
        window.draw(instructionsText);
        window.draw(highScoreText);
        window.display();

    }
};
//--------------------InstructionScreen----------------------------------------
class InstructionScreen : public Screen {
private:
    sf::Font font;
    sf::Text  header, controls, tips, returnText;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Clock blinkClock;   // For blinking/fading animation

public:
    InstructionScreen() {
        font.loadFromFile("assets/impact.ttf");
        backgroundTexture.loadFromFile("assets/b2.png");
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
            800.f / backgroundTexture.getSize().x,
            600.f / backgroundTexture.getSize().y
        );
        //header section
        header.setFont(font);
        header.setString("MISSION BRIEFING");
        header.setCharacterSize(42);
        header.setFillColor(sf::Color(0, 255, 255));
        header.setOutlineColor(sf::Color(0, 150, 255));
        header.setOutlineThickness(3);
        header.setPosition(220.f, 40.f);

        //controls section
        controls.setFont(font);
        controls.setCharacterSize(22);
        controls.setFillColor(sf::Color::White);
        controls.setOutlineColor(sf::Color(0, 100, 200));
        controls.setOutlineThickness(2);
        controls.setString(
            "Controls:\n"
            "  ? Move Up:         W\n"
            "  ? Move Down:    S\n"
            "  ? Move Left:       A\n"
            "  ? Move Right:     D\n"
            "  ? Shoot:           Spacebar\n"
            "  ? Pause:           Escape"
        );
        controls.setPosition(100.f, 100.f);

        //tips section
        tips.setFont(font);
        tips.setCharacterSize(22);
        tips.setFillColor(sf::Color::White);
        tips.setOutlineColor(sf::Color(0, 100, 200));
        tips.setOutlineThickness(2);

        tips.setString(
            "Tips:\n"
            "  • Avoid Invader fire.\n"
            "  • Collect power-ups.\n"
            "  • Earn high scores to win badges!"
        );
        tips.setPosition(100.f, 300.f);

        //Press M to Return Prompt
        returnText.setFont(font);
        returnText.setCharacterSize(22);
        returnText.setString("? Back");
        returnText.setPosition(16.f, 20.f);
        returnText.setStyle(sf::Text::Italic);
        returnText.setFillColor(sf::Color::White);
        returnText.setOutlineThickness(2);
    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                    if (returnText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::Menu;
                    }
                }
            }

        }
    }

    void update(GameState& state) override {
         

    }

    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(header);
        window.draw(controls);
        window.draw(tips);
        window.draw(returnText);
        window.display();

    }
};
//--------------------PauseScreen----------------------------------------
class PauseScreen : public Screen {
private:
    sf::Font font;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Text pauseTitle, resumeText, quitText;


public:
    PauseScreen() {

        font.loadFromFile("assets/impact.ttf");
        backgroundTexture.loadFromFile("assets/b5.jpg");

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(800.f / backgroundTexture.getSize().x, 600.f / backgroundTexture.getSize().y);

        pauseTitle.setFont(font);
        pauseTitle.setString("GAME PAUSED");
        pauseTitle.setCharacterSize(60);
        pauseTitle.setFillColor(sf::Color(217, 130, 7));
        pauseTitle.setOutlineColor(sf::Color::Black);
        pauseTitle.setOutlineThickness(3);
        pauseTitle.setPosition(220.f, 150.f);

        resumeText.setFont(font);
        resumeText.setString("Resume");
        resumeText.setCharacterSize(26);
        resumeText.setFillColor(sf::Color(245, 197, 108));
        resumeText.setOutlineColor(sf::Color::Black);
        resumeText.setPosition(250.f, 260.f);

        quitText.setFont(font);
        quitText.setString("Quit");
        quitText.setCharacterSize(26);
        quitText.setFillColor(sf::Color(184, 126, 18));
        quitText.setOutlineColor(sf::Color::Black);
        quitText.setPosition(220.f, 310.f);

    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                    if (resumeText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::Playing;
                    }
                    else if (quitText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::Menu;
                    }
                }
            }
        }
    }

    void update(GameState& state) override {

    }

    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundSprite);

        sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 130));
        window.draw(overlay);

        window.draw(pauseTitle);
        window.draw(resumeText);
        window.draw(quitText);
        window.display();
    }
};

//------------------------GamePlayScreen---------------------------
class GamePlayScreen : public Screen {
private:
    sf::RectangleShape player;
    float speed = 200.0f; // pixels per second

public:
    GamePlayScreen() {
        player.setSize(sf::Vector2f(50, 50));
        player.setFillColor(sf::Color::Cyan);
        player.setPosition(375, 500);
    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    state = GameState::Menu;
            }
        }
    }

    void update(GameState& state) override {
        float dt = 1.0f / 60.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            player.move(-speed * dt, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            player.move(speed * dt, 0);
        }
    }

    void render(sf::RenderWindow& window) override {
        window.clear(sf::Color::Black);
        window.draw(player);
        window.display();
    }
};
//---------------------------------GameOverScreen----------------------------
class GameOverScreen : public Screen {
private:
    sf::Font font;
    sf::Text gameOverText, scoreText, backText;
    sf::RectangleShape backgroundRect;
    int finalScore = 0;

public:
    GameOverScreen() {
        font.loadFromFile("assets/Orbitron-Regular.ttf");

        // Background
        backgroundRect.setSize(sf::Vector2f(800.f, 600.f));
        backgroundRect.setFillColor(sf::Color(26, 3, 94));

        // GAME OVER Title
        gameOverText.setFont(font);
        gameOverText.setString("GAME OVER");
        gameOverText.setCharacterSize(40);
        gameOverText.setFillColor(sf::Color(230, 230, 250));
        gameOverText.setOutlineColor(sf::Color(0, 100, 255));
        gameOverText.setOutlineThickness(3);
        gameOverText.setPosition(220, 120);

        // Final Score
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color(180, 220, 255));
        scoreText.setOutlineColor(sf::Color::Black);
        scoreText.setOutlineThickness(1.5f);
        scoreText.setPosition(250, 220);

        // Menu Prompt
        backText.setFont(font);
        backText.setString("Back to Menu");
        backText.setCharacterSize(20);
        backText.setFillColor(sf::Color::White);
        backText.setOutlineColor(sf::Color(0, 120, 255));
        backText.setOutlineThickness(2);
        backText.setStyle(sf::Text::Italic);
        backText.setPosition(200, 320);
    }

    void setFinalScore(int score) {
        finalScore = score;
        scoreText.setString("Final Score: " + to_string(score));
    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                    if (backText.getGlobalBounds().contains(mousePos)) {
                        state = GameState::Menu;
                    }
                }
            }

        }
    }

    void update(GameState& state) override {
        // Nothing for now
    }

    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundRect);
        window.draw(gameOverText);
        window.draw(scoreText);
        window.draw(backText);
        window.display();
    }
};

//-------------------------NameInputScreen----------------------------------------
class NameInputScreen : public Screen {
private:
    sf::Font font;
    sf::Text promptText, nameText, infoText, subtitle;
    std::string playerName;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Clock cursorClock, fadeClock;
    ;
    sf::RectangleShape underline;
    bool showCursor = true;
    bool showWarning = false;
    sf::Text warningText;



public:
    NameInputScreen() {
        font.loadFromFile("assets/impact.ttf");
        backgroundTexture.loadFromFile("assets/b4.jpg");
        backgroundSprite.setScale(
            800.f / backgroundTexture.getSize().x,
            600.f / backgroundTexture.getSize().y
        );
        backgroundSprite.setTexture(backgroundTexture);



        underline.setSize(sf::Vector2f(250.f, 2.f));
        underline.setFillColor(sf::Color(255, 200, 90));
        underline.setPosition(30.f, 180.f);

        promptText.setFont(font);
        promptText.setString("ENTER YOUR CALLSIGN");
        promptText.setCharacterSize(28);
        promptText.setPosition(20.f, 90.f);
        promptText.setFillColor(sf::Color(207, 154, 8));
        promptText.setStyle(sf::Text::Bold);
        promptText.setOutlineColor(sf::Color::Black);
        promptText.setOutlineThickness(2);
        ;
        subtitle.setFont(font);
        subtitle.setString("Press ENTER to initiate mission");
        subtitle.setCharacterSize(16);
        subtitle.setFillColor(sf::Color(200, 200, 200));
        subtitle.setPosition(200, 350);


        nameText.setFont(font);
        nameText.setCharacterSize(24);
        nameText.setFillColor(sf::Color(255, 235, 140));
        nameText.setPosition(30.f, 156.f);
        nameText.setOutlineColor(sf::Color::Black);
        nameText.setOutlineThickness(1.5f);

        infoText.setFont(font);
        infoText.setCharacterSize(23);
        infoText.setFillColor(sf::Color::Yellow);
        infoText.setString("Press Enter to start playing");
        infoText.setPosition(40, 215);

        warningText.setFont(font); // use your existing font
        warningText.setCharacterSize(20);
        warningText.setFillColor(sf::Color::Red);
        warningText.setString("Please enter your callsign!");
        warningText.setPosition(200.f, 300.f); // adjust as needed

    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !playerName.empty()) {
                    playerName.pop_back();
                    showWarning = false;
                }
                else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                    playerName += static_cast<char>(event.text.unicode);
                    showWarning = false;
                }
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                if (!playerName.empty()) {
                    state = GameState::Playing;
                    showWarning = false;
                }
                else {
                    showWarning = true; // Show the warning message
                }
            }
        }
    }


    void update(GameState& state) override {
        if (cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            showCursor = !showCursor;
            cursorClock.restart();
        }
        int alpha = static_cast<int>(std::abs(std::sin(fadeClock.getElapsedTime().asSeconds() * 2)) * 255);
        infoText.setFillColor(sf::Color(255, 220, 150, alpha));

        std::string displayName = playerName + (showCursor ? "|" : " ");
        nameText.setString(displayName);
    }

    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(promptText);
        window.draw(nameText);
        window.draw(underline);
        window.draw(infoText);
        if (showWarning) {
            window.draw(warningText); 
        }
        window.display();
    }

    const std::string& getPlayerName() const {
        return playerName;
    }
};

//--------------------------------PlayerScore---------------------
class PlayerScore {
private:
    string name;
    int score;
    string badge;

public:
    PlayerScore() : name(""), score(0), badge("None") {}
    PlayerScore(string n, int s, string b = "None")
        : name(n), score(s), badge(b) {
    }

    string getName() const { return name; }
    int getScore() const { return score; }
    string getBadge() const { return badge; }

    void setBadge(const string& b) { badge = b; }

    bool operator<(const PlayerScore& other) const {
        return score > other.score;
    }

    string toLine() const {
        return name + " " + to_string(score) + " " + badge;
    }
    static PlayerScore fromLine(const string& line) {
        istringstream iss(line);
        string n, b;
        int s = 0;
        iss >> n >> s >> b;
        return PlayerScore(n, s, b);
    }
};

//-----------------HighScoreManager------------------------------------------
class HighScoreManager {
private:
    string fileName;
    vector<PlayerScore> scores;

public:
    HighScoreManager(string file = "highscores.txt") : fileName(file) {
        loadFromFile();
    }

    void loadFromFile() {
        scores.clear();

        ifstream in(fileName);
        if (!in.is_open()) {
            // File doesn’t exist yet — create empty one
            ofstream out(fileName);
            out.close();
            return;
        }

        string line;
        while (getline(in, line)) {
            if (!line.empty()) {
                scores.push_back(PlayerScore::fromLine(line));
            }
        }

        in.close();
    }


    void saveToFile() {
        ofstream out(fileName);
        for (auto& s : scores) {
            out << s.toLine() << "\n";
        }
        out.close();
    }

    void addNewScore(const string& name, int score) {
        scores.push_back(PlayerScore(name, score));
        sort(scores.begin(), scores.end());

        if (scores.size() > 3)
            scores.resize(3);

        assignBadges();
        saveToFile();

    }

    void assignBadges() {
        for (size_t i = 0; i < scores.size(); ++i) {
            if (i == 0) scores[i].setBadge("Gold");
            else if (i == 1) scores[i].setBadge("Silver");
            else if (i == 2) scores[i].setBadge("Bronze");
            else scores[i].setBadge("None");
        }
    }

    const vector<PlayerScore>& getScores() const {
        return scores;
    }
};

//---------------------------- HighScoreScreen ----------------------------
class HighScoreScreen : public Screen {
private:
    sf::Font font;
    sf::Text title, backText;
    sf::RectangleShape backgroundRect;

    sf::Texture goldBadgeTex, silverBadgeTex, bronzeBadgeTex;
    std::vector<sf::Sprite> badgeSprites;

    vector<sf::Text> scoreTexts;
    HighScoreManager& manager;

public:
    HighScoreScreen(HighScoreManager& mgr) : manager(mgr) {
        font.loadFromFile("assets/Orbitron-Regular.ttf");

        backgroundRect.setSize(sf::Vector2f(800.f, 600.f));  // Full window
        backgroundRect.setFillColor(sf::Color(26, 3, 94));  // Custom RGB color




        title.setFont(font);
        title.setString("TOP 3 HIGH SCORES");
        title.setCharacterSize(32);
        title.setFillColor(sf::Color::White);
        title.setPosition(220, 50);

        goldBadgeTex.loadFromFile("assets/gold_badge.png");
        silverBadgeTex.loadFromFile("assets/silver-badge.png");
        bronzeBadgeTex.loadFromFile("assets/bronze_badge.png");


        backText.setFont(font);
        backText.setString("Press M to return to Menu");
        backText.setCharacterSize(22);
        backText.setFillColor(sf::Color::Yellow);
        backText.setPosition(230, 500);
    }

    void handleEvents(sf::RenderWindow& window, GameState& state) override {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M)
                state = GameState::Menu;
        }
    }

    void update(GameState& state) override {
        scoreTexts.clear();
        badgeSprites.clear();

        const auto& scores = manager.getScores();

        if (scores.empty()) {
            sf::Text emptyText;
            emptyText.setFont(font);
            emptyText.setCharacterSize(22);
            emptyText.setFillColor(sf::Color::Red);
            emptyText.setPosition(220, 200);
            emptyText.setString("No high scores yet. Play to set one!");
            scoreTexts.push_back(emptyText);
        }
        else {
            for (size_t i = 0; i < scores.size(); ++i) {
                const auto& p = scores[i];

                // 1. Create text
                sf::Text text;
                text.setFont(font);
                text.setCharacterSize(24);
                text.setFillColor(sf::Color::White);
                text.setPosition(150.f, 130.f + i * 50);
                text.setString(p.getName() + " - " + to_string(p.getScore()));
                scoreTexts.push_back(text);

                // 2. Assign badge image
                sf::Sprite badge;
                if (i == 0) badge.setTexture(goldBadgeTex);
                else if (i == 1) badge.setTexture(silverBadgeTex);
                else badge.setTexture(bronzeBadgeTex);

                badge.setScale(0.08f, 0.08f);
                badge.setPosition(100.f, 130.f + i * 50);
                badgeSprites.push_back(badge);
            }
        }
    }



    void render(sf::RenderWindow& window) override {
        window.clear();
        window.draw(backgroundRect);
        window.draw(title);

        for (size_t i = 0; i < scoreTexts.size(); ++i) {
            window.draw(badgeSprites[i]);
            window.draw(scoreTexts[i]);
        }
        window.draw(backText);
        window.display();

    }
};
//explosion
class Explosion {
private:
    sf::Sprite sprite;
    static sf::Texture sharedTexture;  // shared across all explosions
    sf::Clock clock;
    float duration = 0.6f;
    bool finished = false;

public:
    Explosion(sf::Vector2f position, float scale = 0.08f) {
        if (sharedTexture.getSize().x == 0) {
            if (!sharedTexture.loadFromFile("assets/explosion2.png")) {
                std::cerr << "[ERROR] Failed to load explosion texture\n";
            }
        }
        sprite.setTexture(sharedTexture);
        sprite.setPosition(position);
        sprite.setScale(scale, scale);
    }

    void update() {
        if (clock.getElapsedTime().asSeconds() > duration) finished = true;
    }

    void draw(sf::RenderWindow& window) {
        if (!finished) window.draw(sprite);
    }

    bool isFinished() const { return finished; }
};

// Define static texture outside the class
sf::Texture Explosion::sharedTexture;

//---------------------------------- Bullet ----------------------------------
class Bullet {
public:
    sf::Sprite sprite;
    sf::RectangleShape fallbackShape;
    sf::Vector2f direction;
    float speed;
    bool useSprite = false;

    Bullet(float x, float y, sf::Vector2f dir, sf::Texture* texture = nullptr)
        : direction(dir), speed(10.f)
    {
        if (texture && texture->getSize().x > 0) {
            sprite.setTexture(*texture);
            sprite.setScale(0.05f, 0.05f); // Adjust based on your image size
            sprite.setPosition(x, y);
            useSprite = true;
        }
        else {
            fallbackShape.setSize(sf::Vector2f(5.f, 15.f));
            fallbackShape.setFillColor(sf::Color::Yellow);
            fallbackShape.setPosition(x, y);
        }
    }

    void move() {
        if (useSprite)
            sprite.move(direction * speed);
        else
            fallbackShape.move(direction * speed);
    }

    void draw(sf::RenderWindow& window) {
        if (useSprite)
            window.draw(sprite);
        else
            window.draw(fallbackShape);
    }

    sf::FloatRect getBounds() const {
        return useSprite ? sprite.getGlobalBounds() : fallbackShape.getGlobalBounds();
    }

    sf::Vector2f getPosition() const {
        return useSprite ? sprite.getPosition() : fallbackShape.getPosition();
    }
};

//---------------------------------- Bomb (Invader Drop) ----------------------------------
class Bomb {
private:
    sf::Sprite sprite;
    static sf::Texture texture;
    float speed;

public:
    Bomb(float x, float y, float spd = 100.f, sf::Texture* tex = nullptr)
        : speed(spd)
    {
        if (tex && tex->getSize().x > 0) {
            sprite.setTexture(*tex);
        }
        else {
            // fallback if texture fails
            if (texture.getSize().x == 0)
                texture.loadFromFile("assets/bomb.png");

            sprite.setTexture(texture);
        }

        sprite.setScale(0.03f, 0.03f);  // adjust size as needed
        sprite.setPosition(x, y);
    }

    void move() {
        sprite.move(0.f, speed * (1.f / 60.f));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }
};

// ? define static texture outside the class
sf::Texture Bomb::texture;


//---------------------------------- Spaceship ----------------------------------
class Spaceship {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed;
    int lives;
    bool isPoweredUp = false;
    bool isOnFire = false;
    sf::Clock powerClock, fireClock;

    Spaceship() {
        if (!texture.loadFromFile("assets/sp.png")) {
            cerr << "[ERROR] Could not load spaceship image.\n";
        }
        sprite.setTexture(texture);
        sprite.setScale(0.10f, 0.10f); // Makes it smaller
        sprite.setPosition(370.f, 500.f);
        speed = 6.f;
        lives = 3;
    }

    void move() {
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) movement.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) movement.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) movement.y += speed;

        sprite.move(movement);

        // Wrap-around
        sf::Vector2f pos = sprite.getPosition();
        if (pos.x < -40) sprite.setPosition(800, pos.y);
        if (pos.x > 800) sprite.setPosition(-40, pos.y);
        if (pos.y < -40) sprite.setPosition(pos.x, 600);
        if (pos.y > 600) sprite.setPosition(pos.x, -40);

        if (isPoweredUp && powerClock.getElapsedTime().asSeconds() > 5) isPoweredUp = false;
        if (isOnFire && fireClock.getElapsedTime().asSeconds() > 5) isOnFire = false;
    }

    void activatePowerUp() {
        isPoweredUp = true;
        powerClock.restart();
    }

    void activateFireMode() {
        isOnFire = true;
        fireClock.restart();
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};


//---------------------------------- AddOn ----------------------------------
class AddOn {
protected:
    sf::Sprite sprite;
    sf::Texture texture;
    float speed = 2.f;

public:
    virtual ~AddOn() {}

    virtual void fall() {
        sprite.move(0, speed);
    }

    virtual sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    virtual void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    // Updated to support bullet parameter
    virtual void applyEffect(Spaceship& player, int& score, GameState& state,
        HighScoreManager& manager, const std::string& name,
        std::vector<Bullet>& bullets) = 0;

    virtual bool isDangerous() const { return false; }

    virtual bool isOutOfScreen() const {
        return sprite.getPosition().y > 600;
    }
};

class PowerUpAddOn : public AddOn {
public:
    PowerUpAddOn(float x) {
        if (!texture.loadFromFile("assets/powerUp.png")) {
            std::cerr << "[ERROR] PowerUp texture not found.\n";
        }
        sprite.setTexture(texture);
        sprite.setScale(0.05f, 0.05f);
        sprite.setPosition(x, 0.f);
    }
        void applyEffect(Spaceship& player, int& score, GameState&,
            HighScoreManager&, const std::string&,
            std::vector<Bullet>&) override
        {
            player.activatePowerUp();
            // No bullets here any more!
        }
    };

class ExtraLifeAddOn : public AddOn {
public:
    ExtraLifeAddOn(float x) {
        if (!texture.loadFromFile("assets/extra_life.png")) {
            std::cerr << "[ERROR] ExtraLife texture not found.\n";
        }
        sprite.setTexture(texture);
        sprite.setScale(0.04f, 0.04f);
        sprite.setPosition(x, 0.f);
    }

    void applyEffect(Spaceship& player, int& score, GameState&, HighScoreManager&, const std::string&, std::vector<Bullet>&) override {
        player.lives++;
    }
};

class DangerAddOn : public AddOn {
public:
    DangerAddOn(float x) {
        if (!texture.loadFromFile("assets/danger_sign.png")) {
            std::cerr << "[ERROR] DangerSign texture not found.\n";
        }
        sprite.setTexture(texture);
        sprite.setScale(0.008f, 0.008f);
        sprite.setPosition(x, 0.f);
    }

    void applyEffect(Spaceship& player, int& score, GameState&, HighScoreManager&, const std::string&, std::vector<Bullet>&) override {
        player.lives--;
    }

    bool isDangerous() const override { return true; }
};

//-----------------------------levelManager-----------------------------
class LevelManager {
private:
    int currentLevel = 1;
    int currentWave = 1;

public:
    bool waveJustChanged = false;
    int getLevel() const { return currentLevel; }
    int getWave() const { return currentWave; }

    void advanceWave() {
        int maxWaves = getWavesForCurrentLevel();

        currentWave++;
        if (currentWave > maxWaves) {
            currentWave = 1;
            currentLevel++;
        }
    }

    int getWavesForCurrentLevel() const {
        if (currentLevel == 1 || currentLevel == 2) return 3;
        if (currentLevel == 3) return 3; // we removed old wave 3
        return 0;
    }
    //rectangle
    void createWave1(std::vector<Invader*>& invaders) {
        const int rows = 4;
        const int cols = 10;
        const float spacingX = 60.f;
        const float spacingY = 60.f;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (row == 0 || row == rows - 1 || col == 0 || col == cols - 1) {
                    sf::Vector2f startPos(-50.f, 50.f + row * spacingY);
                    sf::Vector2f targetPos(100.f + col * spacingX, 50.f + row * spacingY);
                    invaders.push_back(new AlphaInvader(startPos, targetPos));
                }
            }
        }
    }
    //triangle
    void createWave2(std::vector<Invader*>& invaders) {
        const int rows = 5;
        const float spacingX = 60.f;
        const float spacingY = 60.f;
        const float centerX = 400.f;

        for (int row = 0; row < rows; ++row) {
            int numInRow = 2 * row + 1;
            float startX = centerX - (numInRow / 2.f) * spacingX;

            for (int i = 0; i < numInRow; ++i) {
                if (i == 0 || i == numInRow - 1 || row == rows - 1) {
                    sf::Vector2f startPos(850.f, 50.f + row * spacingY);
                    sf::Vector2f targetPos(startX + i * spacingX, 50.f + row * spacingY);
                    invaders.push_back(new AlphaInvader(startPos, targetPos));
                }
            }
        }
    }
    void createLevel1Wave3_Mirror(std::vector<Invader*>& invaders) {
        const int rows = 5;
        const float spacingY = 60.f;

        for (int i = 0; i < rows; ++i) {
            float y = 80.f + i * spacingY;

            // Left side enemy ? moves right to center
            sf::Vector2f startLeft(-50.f, y);
            sf::Vector2f targetLeft(300.f, y);
            invaders.push_back(new AlphaInvader(startLeft, targetLeft));

            // Right side enemy ? moves left to center
            sf::Vector2f startRight(850.f, y);
            sf::Vector2f targetRight(500.f, y);
            invaders.push_back(new AlphaInvader(startRight, targetRight));
        }
    }

    //circle
    void createLevel2Wave1(std::vector<Invader*>& invaders) {
        const int enemyCount = 18;
        const float radius = 150.f;
        const float centerX = 400.f;
        const float centerY = 200.f;

        for (int i = 0; i < enemyCount; ++i) {
            float angle = i * (2 * 3.14159265f / enemyCount);
            float targetX = centerX + radius * std::cos(angle);
            float targetY = centerY + radius * std::sin(angle);

            sf::Vector2f startPos(-50.f, targetY);
            sf::Vector2f targetPos(targetX, targetY);

            if (i % 2 == 0)
                invaders.push_back(new AlphaInvader(startPos, targetPos));
            else
                invaders.push_back(new BetaInvader(startPos, targetPos));
        }
    }
    //diamond
    void createLevel2Wave2(std::vector<Invader*>& invaders) {
        const int rows = 7;
        const int maxCols = 7;
        const float spacingX = 55.f;
        const float spacingY = 55.f;
        const float centerX = 400.f;

        for (int row = 0; row < rows; ++row) {
            int half = rows / 2;
            int numInRow = row <= half ? row * 2 + 1 : (rows - row - 1) * 2 + 1;

            float startX = centerX - (numInRow / 2.f) * spacingX;
            float y = 60.f + row * spacingY;

            for (int col = 0; col < numInRow; ++col) {

                if (col != 0 && col != numInRow - 1)
                    continue;

                float x = startX + col * spacingX;


                if ((row + col) % 2 == 0)
                    invaders.push_back(new AlphaInvader({ 850.f, y }, { x, y }));
                else
                    invaders.push_back(new BetaInvader({ 850.f, y }, { x, y }));
            }
        }
    }
    void createLevel2Wave3_SlideInColumns(std::vector<Invader*>& invaders) {
        const int columns = 5;
        const int enemiesPerColumn = 4;
        const float spacingX = 100.f;
        const float spacingY = 60.f;

        for (int col = 0; col < columns; ++col) {
            float x = 100.f + col * spacingX;

            for (int row = 0; row < enemiesPerColumn; ++row) {
                float y = 60.f + row * spacingY;

                if (col % 2 == 0) {
                    // Left slide-in
                    invaders.push_back(new BetaInvader({ -50.f, y }, { x, y }));
                }
                else {
                    // Right slide-in
                    invaders.push_back(new BetaInvader({ 850.f, y }, { x, y }));
                }
            }
        }
    }

    //filled rectangle
    void createLevel3Wave1(std::vector<Invader*>& invaders) {
        const int rows = 3;
        const int cols = 10;
        const float spacingX = 60.f;
        const float spacingY = 60.f;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                sf::Vector2f start(-50.f, 50.f + row * spacingY);
                sf::Vector2f target(100.f + col * spacingX, 50.f + row * spacingY);

                int choice = (row + col) % 3;
                if (choice == 0) invaders.push_back(new AlphaInvader(start, target));
                else if (choice == 1) invaders.push_back(new BetaInvader(start, target));
                else invaders.push_back(new GammaInvader(start, target));
            }
        }
    }
    //filled triangle
    void createLevel3Wave2(std::vector<Invader*>& invaders) {
        const int rows = 4;
        const float spacingX = 55.f;
        const float spacingY = 55.f;
        const float centerX = 400.f;

        for (int row = 0; row < rows; ++row) {
            int numInRow = 2 * row + 1;
            float startX = centerX - (numInRow / 2.f) * spacingX;

            for (int col = 0; col < numInRow; ++col) {
                sf::Vector2f start(850.f, 60.f + row * spacingY);
                sf::Vector2f target(startX + col * spacingX, 60.f + row * spacingY);

                int choice = (row + col) % 3;
                if (choice == 0) invaders.push_back(new AlphaInvader(start, target));
                else if (choice == 1) invaders.push_back(new BetaInvader(start, target));
                else invaders.push_back(new GammaInvader(start, target));
            }
        }
    }
    //filled circle-> require modification
    void createLevel3Wave3(std::vector<Invader*>& invaders) {
        const float centerX = 400.f;
        const float centerY = 200.f;
        const float maxRadius = 150.f;
        const int rings = 3; // Number of concentric rings
        const int baseEnemies = 6; // Enemies in innermost ring

        for (int r = 1; r <= rings; ++r) {
            float radius = (r / static_cast<float>(rings)) * maxRadius;
            int enemiesInRing = baseEnemies + r * 2; // Increase enemies for outer rings

            for (int i = 0; i < enemiesInRing; ++i) {
                float angle = i * (2 * 3.14159265f / enemiesInRing);
                float x = centerX + radius * std::cos(angle);
                float y = centerY + radius * std::sin(angle);

                sf::Vector2f startPos(-50.f, y); // Enter from left
                sf::Vector2f targetPos(x, y);

                // Alternate types for visual variety
                if ((i + r) % 3 == 0)
                    invaders.push_back(new AlphaInvader(startPos, targetPos));
                else if ((i + r) % 3 == 1)
                    invaders.push_back(new BetaInvader(startPos, targetPos));
                else
                    invaders.push_back(new GammaInvader(startPos, targetPos));
            }
        }
    }


    //filled diamond
    void createLevel3Wave4(std::vector<Invader*>& invaders) {
        const int rows = 7;
        const float spacingX = 55.f;
        const float spacingY = 55.f;
        const float centerX = 400.f;

        for (int row = 0; row < rows; ++row) {
            int half = rows / 2;
            int numInRow = row <= half ? row * 2 + 1 : (rows - row - 1) * 2 + 1;

            float startX = centerX - (numInRow / 2.f) * spacingX;
            float y = 60.f + row * spacingY;

            for (int i = 0; i < numInRow; ++i) {
                sf::Vector2f start(850.f, y);
                sf::Vector2f target(startX + i * spacingX, y);

                int choice = (row + i) % 3;
                if (choice == 0) invaders.push_back(new AlphaInvader(start, target));
                else if (choice == 1) invaders.push_back(new BetaInvader(start, target));
                else invaders.push_back(new GammaInvader(start, target));
            }
        }
    }

    void nextWaveOrLevel(std::vector<Invader*>& invaders) {
        if (currentLevel == 1 && currentWave == 1) {
            advanceWave();
            createWave2(invaders);  
        }
        else if (currentLevel == 1 && currentWave == 2) {
            advanceWave();
            createLevel1Wave3_Mirror(invaders);  
        }
        else if (currentLevel == 2 && currentWave == 1) {
            advanceWave();
            createLevel2Wave2(invaders);  
        }
        else if (currentLevel == 2 && currentWave == 2) {
            advanceWave();
            createLevel2Wave3_SlideInColumns(invaders);  
        }
        else if (currentLevel == 3 && currentWave == 1) {
            advanceWave();
            createLevel3Wave2(invaders); 
        }
        else if (currentLevel == 3 && currentWave == 2) {
            advanceWave();
            createLevel3Wave4(invaders);  
        }
        else {
            advanceWave();
            if (currentLevel == 2)
                createLevel2Wave1(invaders);
            else if (currentLevel == 3)
                createLevel3Wave1(invaders);
        }
    }


    void updateDisplay(sf::RenderWindow& window) {
        sf::Font font;
        if (!font.loadFromFile("assets/Orbitron-Regular.ttf")) return;

        sf::Text levelText;
        levelText.setFont(font);
        levelText.setCharacterSize(16);
        levelText.setFillColor(sf::Color::Cyan);
        levelText.setPosition(320, 10);
        levelText.setString("Level " + std::to_string(currentLevel) + " - Wave " + std::to_string(currentWave));
        window.draw(levelText);
    }
};


//---------------------------------- Game ----------------------------------
class Game
{
private:
    sf::RenderWindow window;
    Spaceship player;
    vector<Bullet> bullets;
    sf::Texture bulletTexture;
    vector<Invader*> invaders;
    sf::Texture InvaderTexture;
    vector<AddOn*> addons;
    vector<Bomb> bombs;
    sf::Texture bombTexture;
    GameState currentState;
    Screen* currentScreen;
    MenuScreen menuScreen;
    InstructionScreen instructionScreen;
    GamePlayScreen gamePlayScreen;
    PauseScreen pauseScreen;
    GameOverScreen gameOverScreen;
    HighScoreManager highScoreManager;
    HighScoreScreen highScoreScreen{ highScoreManager };
    NameInputScreen nameInputScreen;

    std::vector<Explosion> explosions;

    LevelManager levelManager;
    sf::Text waveText;
    sf::Clock waveTextClock;
    bool showWaveText = false;

    sf::Clock InvaderClock, addonClock;
    string playerName;
    bool gameStarting = false;
    sf::Clock gameStartClock;
    sf::Font font;
    sf::Text scoreText, livesText;
    int score;


    Monster* monster = nullptr;
    bool monsterActive = false;
    sf::Clock monsterTriggerClock;
    sf::Clock monsterLifetimeClock;
    float monsterDuration = 25.f;
    bool monsterScoreGiven = false;
    float monsterTriggerTime = 10.f + rand() % 10; // Random between 10-20s
    bool showMonsterWarning = false;

    sf::Clock globalBombClock;
    float globalBombInterval = 1.5f; // Try to drop bombs every 1.5 seconds

    sf::Clock monsterWarningClock;
    sf::Text monsterMessage;
    sf::Clock monsterMessageClock;
    bool showMonsterMessage = false;
    bool monsterHasAppeared = false;


public:
    Game() : window(sf::VideoMode(800, 600), "Space Invaders"), score(0) {
        window.setFramerateLimit(60);
        if (!font.loadFromFile("assets/Orbitron-Regular.ttf")) {
            cerr << "[ERROR] Could not load font.\n";
        }
        if (!bulletTexture.loadFromFile("assets/bullet.png")) {
            cerr << "[ERROR] Could not load bullet.png, fallback to shape.\n";
        }
        if (!bombTexture.loadFromFile("assets/bomb.png")) {
            cerr << "[ERROR] Could not load bomb.png\n";
        }
        currentState = GameState::Menu;
        currentScreen = &menuScreen;



        scoreText.setFont(font);
        scoreText.setCharacterSize(18);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);

        livesText.setFont(font);
        livesText.setCharacterSize(18);
        livesText.setFillColor(sf::Color::White);
        livesText.setPosition(680, 10);

        srand(static_cast<unsigned>(time(0)));
        waveText.setFont(font);
        waveText.setCharacterSize(24);
        waveText.setFillColor(sf::Color::Yellow);
        waveText.setPosition(280, 300);


        levelManager.createWave1(invaders);
        showWaveText = true;
        waveText.setString("LEVEL 1 - WAVE 1");
        waveTextClock.restart();


    }

    void resetGame() {
        score = 0;
        bullets.clear();
        for (auto* e : invaders) delete e;
        invaders.clear();
        bombs.clear();
        addons.clear();
        player.lives = 3;
        player.isPoweredUp = false;
        player.isOnFire = false;

        levelManager.createWave1(invaders);
        showWaveText = true;
        waveText.setString("LEVEL 1 - WAVE 1");
        waveTextClock.restart();

        monsterMessage.setFont(font);
        monsterMessage.setCharacterSize(26);
        monsterMessage.setFillColor(sf::Color::White);
        monsterMessage.setOutlineColor(sf::Color::Black);
        monsterMessage.setOutlineThickness(1.5f);
        monsterMessage.setPosition(200.f, 280.f);


        if (monster) {
            delete monster;
            monster = nullptr;
            monsterActive = false;
            monsterTriggerClock.restart();
            monsterTriggerTime = 10.f + rand() % 10;
        }



    }
    void createInvaderFormation() {
        for (auto* e : invaders) delete e;
        invaders.clear();

        const int rows = 3;
        const int cols = 10;
        const float spacingX = 60.f;
        const float spacingY = 60.f;

        int level = levelManager.getLevel();
        int wave = levelManager.getWave();

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                sf::Vector2f startPos(-50.f, 50.f + row * spacingY);
                sf::Vector2f targetPos(100.f + col * spacingX, 50.f + row * spacingY);
                invaders.push_back(new AlphaInvader(startPos, targetPos));

            }
        }
    }

    void start() {
        while (window.isOpen()) {
            switch (currentState) {
            case GameState::NameInput:
                nameInputScreen.handleEvents(window, currentState);
                nameInputScreen.update(currentState);
                nameInputScreen.render(window);

                if (currentState == GameState::Playing) {
                    playerName = nameInputScreen.getPlayerName();

                    // Check if name is empty
                    if (playerName.empty()) {
                        // Prevent transition — go back to name input screen
                        currentState = GameState::NameInput;
                    }
                    else {
                        // Name is valid, proceed with starting the game
                        score = 0;
                        bullets.clear();
                        addons.clear();
                        player.lives = 3;
                        player.isPoweredUp = false;
                        player.isOnFire = false;

                        resetGame();
                        gameStarting = true;
                        gameStartClock.restart();
                    }
                }
                break;
            case GameState::Playing:
                handleEvents();
                update();
                render();
                break;

            case GameState::Paused:
                currentScreen = &pauseScreen;
                [[fallthrough]];
            case GameState::Menu:
            case GameState::Instructions:
            case GameState::GameOver:
            case GameState::HighScore:
                currentScreen->handleEvents(window, currentState);
                currentScreen->update(currentState);
                currentScreen->render(window);
                break;
            }

            // Update screen pointer
            if (currentState == GameState::Instructions)
                currentScreen = &instructionScreen;
            else if (currentState == GameState::Menu)
                currentScreen = &menuScreen;
            else if (currentState == GameState::Paused)
                currentScreen = &pauseScreen;
            else if (currentState == GameState::GameOver)
                currentScreen = &gameOverScreen;
            else if (currentState == GameState::HighScore)
                currentScreen = &highScoreScreen;
        }
    }


    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    fireBullets();
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    currentState = GameState::Paused;
                }
            }
        }
    }

    void fireBullets() {
        sf::Vector2f pos = player.getPosition();

        if (player.isPoweredUp) {
            int numBullets = 7;
            float startDeg = -60.f;
            float endDeg = 60.f;
            float step = (endDeg - startDeg) / (numBullets - 1);

            for (int i = 0; i < numBullets; ++i) {
                float angleDeg = startDeg + i * step;
                float angleRad = angleDeg * 3.14159f / 180.f;
                sf::Vector2f dir(std::sin(angleRad), -std::cos(angleRad));

                bullets.emplace_back(pos.x + 30, pos.y, dir, &bulletTexture);
            }
        }
        else {
            int bulletCount = 1;

            int level = levelManager.getLevel();
            if (level == 2) bulletCount = 2;
            else if (level >= 3) bulletCount = 3;

            if (bulletCount == 1) {
                bullets.emplace_back(pos.x + 30.f, pos.y, sf::Vector2f(0.f, -1.f), &bulletTexture);
            }
            else {
                // Fire bullets in small horizontal spread
                float spacing = 10.f;
                float startX = pos.x + 30.f - spacing * (bulletCount - 1) / 2.f;

                for (int i = 0; i < bulletCount; ++i) {
                    float angle = -10.f + 10.f * i;  // e.g., -10, 0, 10 for 3 bullets
                    float rad = angle * 3.14159f / 180.f;
                    sf::Vector2f dir(std::sin(rad), -std::cos(rad));
                    bullets.emplace_back(startX + i * spacing, pos.y, dir, &bulletTexture);
                }
            }
        }

    }

    void update() {
        float dt = 1.0f / 60.f;
        if (InvaderClock.getElapsedTime().asSeconds() > 0.016f)
            dt = InvaderClock.restart().asSeconds();

        player.move();

        // Monster warning phase
        if (!monsterHasAppeared && !monsterActive && !showMonsterWarning && levelManager.getLevel() == 1 &&
            monsterTriggerClock.getElapsedTime().asSeconds() >= monsterTriggerTime)
        {
            showMonsterWarning = true;
            monsterWarningClock.restart();
        }

        // After 2s warning, spawn monster
        if (showMonsterWarning && monsterWarningClock.getElapsedTime().asSeconds() >= 2.f) {
            monster = new Monster(sf::Vector2f(300.f, 100.f));
            monsterActive = true;
            monsterHasAppeared = true;
            monsterScoreGiven = false;
            monsterLifetimeClock.restart();
            showMonsterWarning = false;

            for (auto* inv : invaders) delete inv;
            invaders.clear();
        }

        for (auto& b : bullets) b.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet& b) {
            return b.getPosition().y < -10 || b.getPosition().x < -10 || b.getPosition().x > 810;
            }), bullets.end());

        // Monster behavior
        if (monsterActive && monster) {
            monster->update(dt);

            if (monster->isBeamActive() &&
                monster->getBeamBounds().intersects(player.getBounds()) &&
                !player.isPoweredUp)
            {
                player.lives--;
                if (player.lives <= 0) {
                    highScoreManager.addNewScore(playerName, score);
                    gameOverScreen.setFinalScore(score);
                    currentState = GameState::GameOver;
                    return;
                }
            }

            // Bullet hits Monster
            for (size_t i = 0; i < bullets.size(); ++i) {
                if (bullets[i].getBounds().intersects(monster->getBounds())) {
                    monster->takeDamage();
                    bullets.erase(bullets.begin() + i);
                    break;
                }
            }

            // Monster destroyed
            if (monster->isDead()) {
                score += 80;

                sf::Vector2f monsterPos = monster->getPosition(); 
                explosions.emplace_back(monsterPos + sf::Vector2f(40.f, 40.f), 0.5f); 

                delete monster;
                monster = nullptr;
                monsterActive = false;

                monsterTriggerClock.restart();
                monsterTriggerTime = 10.f + rand() % 10;

                monsterMessage.setString("Monster Destroyed!");
                monsterMessage.setPosition(200, 210);
                monsterMessageClock.restart();
                showMonsterMessage = true;
            }

             // Monster dodged (time passed)
            else if (monsterLifetimeClock.getElapsedTime().asSeconds() >= monsterDuration && !monsterScoreGiven) {
                score += 40;
                monsterScoreGiven = true;
                delete monster;
                monster = nullptr;
                monsterActive = false;

                monsterTriggerClock.restart();
                monsterTriggerTime = 10.f + rand() % 10;

                monsterMessage.setString("Monster Escaped!");
                monsterMessage.setPosition(200, 210);
                monsterMessageClock.restart();
                showMonsterMessage = true;
            }

        }

        if (!monsterActive) {
            for (auto* e : invaders)
                e->update(dt);

            if (globalBombClock.getElapsedTime().asSeconds() >= globalBombInterval) {
                globalBombClock.restart();

                std::vector<Invader*> readyInvaders;
                for (auto* e : invaders) {
                    if (e->isBombReady()) {
                        readyInvaders.push_back(e);
                    }
                }

                // Shuffle and pick up to 3 to actually drop
                std::shuffle(readyInvaders.begin(), readyInvaders.end(), std::mt19937(std::random_device{}()));

                int maxDrops = 3;
                for (int i = 0; i < std::min(maxDrops, static_cast<int>(readyInvaders.size())); ++i) {
                    sf::Vector2f pos = readyInvaders[i]->getBombPosition();
                    bombs.emplace_back(pos.x, pos.y, readyInvaders[i]->getBombSpeed(), &bombTexture);
                    readyInvaders[i]->restartBombTimer();
                }
            }


        }

        for (auto& bomb : bombs)
            bomb.move();

        bombs.erase(remove_if(bombs.begin(), bombs.end(), [](Bomb& b) {
            return b.getPosition().y > 600;
            }), bombs.end());

        if (addonClock.getElapsedTime().asSeconds() > 6.f) {
            float x = static_cast<float>(rand() % 760);
            int type = rand() % 3;
            if (type == 0) addons.push_back(new PowerUpAddOn(x));
            else if (type == 1) addons.push_back(new DangerAddOn(x));
            else addons.push_back(new ExtraLifeAddOn(x));
            addonClock.restart();
        }

        for (size_t i = 0; i < addons.size();) {
            AddOn* addon = addons[i];
            addon->fall();

            if (addon->getBounds().intersects(player.getBounds())) {
                addon->applyEffect(player, score, currentState, highScoreManager, playerName, bullets);
                if (player.lives <= 0) {
                    highScoreManager.addNewScore(playerName, score);
                    gameOverScreen.setFinalScore(score);
                    currentState = GameState::GameOver;
                    return;
                }
                delete addon;
                addons.erase(addons.begin() + i);
            }
            else if (addon->isOutOfScreen()) {
                if (addon->isDangerous()) score += 5;
                delete addon;
                addons.erase(addons.begin() + i);
            }
            else {
                ++i;
            }
        }

        if (!monsterActive) {
            std::vector<size_t> bulletsToErase;
            std::vector<size_t> invadersToErase;

            for (size_t i = 0; i < bullets.size(); ++i) {
                for (size_t j = 0; j < invaders.size(); ++j) {
                    if (bullets[i].getBounds().intersects(invaders[j]->getBounds())) {
                        invaders[j]->takeDamage();
                        bulletsToErase.push_back(i);

                        if (invaders[j]->isDead()) {
                            if (dynamic_cast<AlphaInvader*>(invaders[j])) score += 10;
                            else if (dynamic_cast<BetaInvader*>(invaders[j])) score += 20;
                            else if (dynamic_cast<GammaInvader*>(invaders[j])) score += 30;

                            explosions.emplace_back(invaders[j]->getSprite().getPosition());
                            invadersToErase.push_back(j);
                        }

                        break;  // Bullet hit only one invader
                    }
                }
            }

            // Erase bullets in reverse order to avoid invalidating indices
            std::sort(bulletsToErase.rbegin(), bulletsToErase.rend());
            for (size_t idx : bulletsToErase) {
                if (idx < bullets.size())
                    bullets.erase(bullets.begin() + idx);
            }

            // Erase and delete invaders
            std::sort(invadersToErase.rbegin(), invadersToErase.rend());
            for (size_t idx : invadersToErase) {
                if (idx < invaders.size()) {
                    delete invaders[idx];
                    invaders.erase(invaders.begin() + idx);
                }
            }

            for (auto* e : invaders) {
                if (e->getBounds().intersects(player.getBounds()) && !player.isPoweredUp) {
                    e->getSprite().setPosition(-100, -100);
                    player.lives--;
                    if (player.lives <= 0) {
                        highScoreManager.addNewScore(playerName, score);
                        gameOverScreen.setFinalScore(score);
                        currentState = GameState::GameOver;
                        return;
                    }
                }
            }

            if (invaders.empty()) {
                levelManager.nextWaveOrLevel(invaders);
                showWaveText = true;
                waveText.setString("LEVEL " + std::to_string(levelManager.getLevel()) +
                    " - WAVE " + std::to_string(levelManager.getWave()));
                waveTextClock.restart();
            }
            else {
                levelManager.waveJustChanged = false;
            }
        }

        for (auto& bomb : bombs) {
            if (bomb.getBounds().intersects(player.getBounds()) && !player.isPoweredUp) {
                bomb.setPosition(-100, -100);
                player.lives--;
                if (player.lives <= 0) {
                    highScoreManager.addNewScore(playerName, score);
                    gameOverScreen.setFinalScore(score);
                    currentState = GameState::GameOver;
                    return;
                }
            }
        }

        if (!explosions.empty()) {
            for (auto& exp : explosions)
                exp.update();

            explosions.erase(
                std::remove_if(explosions.begin(), explosions.end(),
                    [](const Explosion& e) { return e.isFinished(); }),
                explosions.end());
        }

        scoreText.setString("Score: " + to_string(score));
        livesText.setString("Lives: " + to_string(player.lives));
    }


    void render() {

        window.clear();
        player.draw(window);
        const auto& topScores = highScoreManager.getScores();
        for (size_t i = 0; i < 3 && i < topScores.size(); ++i) {
            sf::Text badgeText;
            badgeText.setFont(font);
            badgeText.setCharacterSize(16);
            badgeText.setFillColor(sf::Color::White);
            badgeText.setString(topScores[i].getName() + ": " + topScores[i].getBadge());
            badgeText.setPosition(10.f, 40.f + i * 20.f);
            window.draw(badgeText);
        }

        for (auto& b : bullets)
            b.draw(window);
        for (auto* e : invaders) e->draw(window);
        for (auto& exp : explosions) {
            if (!exp.isFinished())
                exp.draw(window);
        }
        for (auto* a : addons) a->draw(window);
        for (auto& bomb : bombs) bomb.draw(window);
        window.draw(scoreText);
        window.draw(livesText);
        if (gameStarting && gameStartClock.getElapsedTime().asSeconds() < 2.f) {
            sf::Text startingText;
            startingText.setFont(font);
            startingText.setCharacterSize(28);
            startingText.setFillColor(sf::Color::Yellow);
            startingText.setString("GAME STARTING...");
            startingText.setPosition(250, 250);
            window.draw(startingText);
        }
        else {
            gameStarting = false;
        }
        levelManager.updateDisplay(window);

        if (showWaveText && waveTextClock.getElapsedTime().asSeconds() < 2.f) {
            window.draw(waveText);
        }
        else {
            showWaveText = false;
        }
        if (monsterActive && monster) {
            monster->draw(window);
        }
        if (showMonsterWarning) {
            sf::Text warningText;
            warningText.setFont(font);
            warningText.setCharacterSize(30);
            warningText.setFillColor(sf::Color::Red);
            warningText.setOutlineColor(sf::Color::Black);
            warningText.setOutlineThickness(2);
            warningText.setString("MONSTER APPROACHING");
            warningText.setPosition(200, 210);
            window.draw(warningText);
        }
        if (showMonsterMessage && monsterMessageClock.getElapsedTime().asSeconds() <= 2.f) {
            window.draw(monsterMessage);
        }
        else {
            showMonsterMessage = false;
        }

        window.display();
    }
};

//---------------------------------- Main ----------------------------------
int main() {
    Game game;
    game.start();
    return 0;
}