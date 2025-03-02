#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include <ctime>
#include <random>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <set>
#include <fmt/ostream.h>
#include <string>
#include <iostream>
#include <fmt/format.h>
#include <queue>

auto scoresFile = std::fstream("../assets/scores.txt", std::ios::out | std::ios::app);

struct Game {
    std::string nickname;
    int score;
    int wpm;
    int speed;
    int maxFreq;
    float time;
    std::string date;

    Game(std::string nickname, int const& score, int const& wpm, int const& speed, int const& maxFreq, float const& time, std::string date)
    : nickname(std::move(nickname)), score(score), wpm(wpm), speed(speed), maxFreq(maxFreq), time(time), date(std::move(date))  { };

    auto saveScoreToFile() {
        fmt::print(scoresFile, "{} ", nickname);
        fmt::print(scoresFile, "{} ", score);
        fmt::print(scoresFile, "{} ", wpm);
        fmt::print(scoresFile, "{} ", speed);
        fmt::print(scoresFile, "{} ", maxFreq);
        fmt::print(scoresFile, "{} ", time);
        fmt::println(scoresFile, "{}", date);
        scoresFile.flush();
    }
};

enum class Screen {
    Starting,
    Options,
    Game,
    GameOver,
    Score,
};

struct Word {
    sf::Text text;
    bool isFinished;

    Word(std::string const &str, sf::Font const &font, int const &fontSize)
            : text(str, font, fontSize), isFinished(true) {
        text.setFillColor(sf::Color::White);
    }
};

namespace my_library {
    auto equalsIgnoreCase(std::string const &left, std::string const &right) -> bool {
        if (left.size() != right.size()) return false;
        return std::equal(left.begin(), left.end(), right.begin(), right.end(),
                          [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    }

    auto getRandomInt(int from, int to) { //stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(from, to);
        return dis(gen);
    }

    auto intersects(const Word &toCheckWord, const Word &existingWord) -> bool {
        sf::FloatRect toCheckBounds = toCheckWord.text.getGlobalBounds();
        sf::FloatRect existingBounds = existingWord.text.getGlobalBounds();

        return toCheckBounds.intersects(existingBounds);
    }

    auto timeToStr(std::tm const& time) {
        std::ostringstream oss;
        oss << std::put_time(&time, "%d.%m.%Y");
        return oss.str();
    }

    struct CompareScore {
        bool operator()(const Game& a, const Game& b) {
                 return a.score > b.score;
        }
    };

    auto getTopScorers() {
        std::ifstream infile("../assets/scores.txt");
        std::priority_queue<Game, std::vector<Game>, CompareScore> pq; //en.cppreference.com/w/cpp/container/priority_queue
        std::string line;

        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string nickname, date;
            int score, wpm, speed, maxFreq;
            float time;

            iss >> nickname >> score >> wpm >> speed >> maxFreq >> time >> date;
            Game game(nickname, score, wpm, speed, maxFreq, time, date);

            if (pq.size() < 10) {
                pq.push(game);
            } else if (game.score > pq.top().score) {
                pq.pop();
                pq.push(game);
            }
        }

        std::vector<Game> topScorers;
        while (!pq.empty()) {
            topScorers.push_back(pq.top());
            pq.pop();
        }

        std::ranges::sort(topScorers.begin(), topScorers.end(), [](const Game& a, const Game& b) {
            return a.score > b.score;
        });
        std::ranges::reverse(topScorers.begin(), topScorers.end());
        return topScorers;
    }
}

// Main function
int main() {

    // Load animation frames
    std::vector<sf::Texture> animationFrames;
    for (int i = 1; i <= 19; i++) {
        sf::Texture frame;
        frame.loadFromFile("../assets/bg_animation/frame_2." + std::to_string(i) + ".1.png");
        animationFrames.push_back(frame);
    }

    // Setup sprites
    sf::Sprite animationSprite;
    animationSprite.move(0, -100);
    int currentFrame = 0;
    sf::Clock animationClock;
    const float frameDuration = 0.0416667f;

    sf::Sprite menuSprite;
    menuSprite.setTexture(animationFrames[0]);
    menuSprite.move(0, -100);



    // Words source
    std::vector<std::string> wordsSource = {
            "apple", "run", "beautiful", "he", "quickly", "and", "under", "wow", "cat", "jump",
            "tall", "she", "very", "but", "over", "oops", "dog", "eat", "small", "they", "slowly",
            "because", "in", "hey", "house", "swim", "smart", "we", "happily", "if", "on", "oh",
            "tree", "walk", "strong", "you", "softly", "or", "with", "ah", "sun", "fly", "kind",
            "it", "loudly", "while", "at", "ouch", "moon", "sit", "fast", "carefully",
            "although", "by", "car", "read", "bright", "us", "gently", "unless", "near", "sky",
            "write", "dark", "him", "eagerly", "since", "of", "star", "sleep", "sweet", "her",
            "quietly", "though", "between", "ugh", "flower", "sing", "brave", "them", "silently",
            "whether", "during", "river", "dance", "warm", "me", "yet", "before", "hooray", "grass",
            "talk", "cold", "this", "really", "after", "behind", "hill", "play", "sharp", "that",
            "simply", "when", "through", "hello", "bird", "listen", "soft", "mine", "until",
            "against", "boat", "laugh", "clear", "yours", "once", "above", "fish", "study", "smooth",
            "his", "seldom", "as", "along", "book", "climb", "funny", "weep", "clearly", "nor",
            "beside", "chair", "dig", "hard", "whom", "eventually", "therefore", "beneath", "table",
            "drive", "easy", "myself", "indeed", "instead", "beyond", "music", "cook", "happy",
            "herself", "recently", "nevertheless", "inside", "window", "smile", "itself", "moreover",
            "outside", "door", "open", "wide", "ourselves", "finally", "hence", "roof", "close",
            "short", "anybody", "suddenly", "thus", "across", "wall", "throw", "rough", "somebody",
            "just", "accordingly", "below", "ground", "hold", "nobody", "maybe", "also", "within",
            "ceiling", "catch", "narrow", "sometimes", "otherwise", "without", "path", "build",
            "thick", "everybody", "furthermore", "upon", "street", "draw", "thin", "often",
            "meanwhile", "alongside", "road", "cut", "heavy", "immediately", "likewise", "underneath",
            "bridge", "chase", "light", "anything", "always", "anyway", "among", "mountain", "seek",
            "whichever", "henceforward", "forest", "lift", "themselves", "nonetheless", "throughout",
            "ocean", "taste", "clean", "whomever", "hardly", "around", "yay"
    };
    std::ranges::shuffle(wordsSource, std::random_device());

    // SFML Window
    sf::RenderWindow window(sf::VideoMode(800, 600), "StanTyper");

    // Setting icon
    sf::Image icon;
    if (icon.loadFromFile("../assets/icon.jpg")) {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    // Game variables
    bool scoreSaved = false;
    bool scoreDownloaded = false;
    std::vector<Game> topScorers;
    Screen gameState = Screen::Starting;
    Screen previousGameState = gameState;
    std::string nickname;

    // Load fonts
    sf::Font arialFont;
    arialFont.loadFromFile("../assets/fonts/arial.ttf");

    sf::Font interfaceFont;
    interfaceFont.loadFromFile("../assets/fonts/BROMPH_TOWN.ttf");

    sf::Font astonpolizFont;
    astonpolizFont.loadFromFile("../assets/fonts/Astonpoliz.ttf");

    sf::Font bubblegumFont;
    bubblegumFont.loadFromFile("../assets/fonts/Bubblegum.ttf");

    sf::Font helloSamosaFont;
    helloSamosaFont.loadFromFile("../assets/fonts/Hello_Samosa.ttf");

    // Store fonts and font texts
    std::vector<sf::Font> fonts = {arialFont, interfaceFont, astonpolizFont, bubblegumFont, helloSamosaFont};

    std::vector<sf::Text> fontsTexts = {
            sf::Text("Arial", arialFont, 36),
            sf::Text("Bromph Town", interfaceFont, 36),
            sf::Text("Astonpoliz", astonpolizFont, 36),
            sf::Text("Bubblegum", bubblegumFont, 36),
            sf::Text("Hello Samosa", helloSamosaFont, 36)
    };

    for (auto& text : fontsTexts)
        text.setPosition(520, 360);

    int currentFontIndex = 0;

    // Game Over overlay and text
    sf::RectangleShape gameOverOverlay(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
    gameOverOverlay.setFillColor(sf::Color(0, 0, 0, 200));

    sf::Text gameOverText("Game Over", interfaceFont, 50);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(290, static_cast<float>(window.getSize().y) / 2 - 200);

    // Score variables
    int score = 0;
    sf::Text scoreText("Score: ", interfaceFont, 24);
    scoreText.setPosition(370, static_cast<float>(window.getSize().y - scoreText.getCharacterSize() * 1.4));

    sf::Text scoreValueText("", interfaceFont, 24);
    scoreValueText.setPosition(scoreText.getPosition().x + 5, scoreText.getPosition().y);
    scoreValueText.setFillColor(sf::Color(255, 255, 140));

    sf::Text wordsText("Words: ", interfaceFont, 24);
    wordsText.setPosition(500, static_cast<float>(window.getSize().y - wordsText.getCharacterSize() * 1.4));

    sf::Text wordsValueText("", interfaceFont, 24);
    wordsValueText.setPosition(wordsText.getPosition().x + 15, wordsText.getPosition().y);
    wordsValueText.setFillColor(sf::Color(255, 255, 140));

    int wpm = 0;
    sf::Text wpmText("WPM: ", interfaceFont, 24);
    wpmText.setPosition(353, 250);

    // Words for the game
    std::vector<Word> words;
    for (const auto& text : wordsSource) {
        Word newWord(text, interfaceFont, 24);
        newWord.text.setPosition(-newWord.text.getGlobalBounds().width, 0);
        newWord.text.setFillColor(sf::Color(10, 255, 140));
        words.push_back(newWord);
    }

    auto wordIter = words.begin();

    // User input decorations and cursor
    sf::Text decor_userInput("[                            ]", interfaceFont, 24);
    decor_userInput.setPosition(5, static_cast<float>(window.getSize().y - decor_userInput.getCharacterSize() * 1.5));

    sf::Text userInput("", interfaceFont, 24);

    sf::RectangleShape cursor(sf::Vector2f(14, 2));
    cursor.setFillColor(userInput.getFillColor());

    sf::Clock cursorClock;
    bool cursorVisible = true;

    std::string inputStr;

    // Time and missedCount texts
    sf::Text timeText("Time: ", interfaceFont, 24);
    timeText.setPosition(static_cast<float>(window.getSize().x - timeText.getCharacterSize() * 6 + 5), static_cast<float>(window.getSize().y - timeText.getCharacterSize() * 1.4));

    sf::Text timeValueText("", interfaceFont, 24);
    timeValueText.setPosition(timeText.getPosition());
    timeValueText.setFillColor(sf::Color(255, 255, 140));

    int missedCount = 0;
    sf::Text missedText("Missed: ", interfaceFont, 24);
    missedText.setPosition(decor_userInput.getGlobalBounds().width + 20, static_cast<float>(window.getSize().y - missedText.getCharacterSize() * 1.4));

    sf::Text missedValueText("        ", interfaceFont, 24);
    missedValueText.setPosition(missedText.getPosition().x + 5, missedText.getPosition().y);
    missedValueText.setFillColor(sf::Color(255, 255, 140));

    // Game settings
    float wordSpeed = 0.03f;
    float wordFrequency = 1.0f;
    float maxWordFrequency = 0.7f;
    int wordsFinished = 0;

    sf::Clock wordClock;
    sf::Time wordTime;

    sf::Clock gameClock;

    sf::Texture screenTexture;
    sf::Sprite screenSprite;

    // Main title
    sf::Text stanTyperText("StanTyper", interfaceFont, 90);
    stanTyperText.setPosition(230, 75);
    stanTyperText.setFillColor(sf::Color(255, 255, 140));

    // Starting screen buttons
    std::vector<sf::Text> startingScreen_Buttons = {
            sf::Text("  Play", interfaceFont, 36),
            sf::Text("Scoreboard", interfaceFont, 36),
            sf::Text(" Options", interfaceFont, 36),
            sf::Text("  Quit", interfaceFont, 36)
    };

    int startingScreen_CurrentIndex = 0;
    int startingScreen_PreviousIndex = 0;

    startingScreen_Buttons[0].setPosition(360, 240);
    startingScreen_Buttons[1].setPosition(330, 300);
    startingScreen_Buttons[2].setPosition(345, 360);
    startingScreen_Buttons[3].setPosition(360, 420);

    // Options screen buttons
    std::vector<sf::Text> optionsScreen_Buttons = {
            sf::Text("Nickname: ", interfaceFont, 36),
            sf::Text("Speed: ", interfaceFont, 36),
            sf::Text("Max. word frequency: ", interfaceFont, 36),
            sf::Text("Font: ", interfaceFont, 36)
    };

    int optionsScreen_CurrentIndex = 0;

    sf::Text speedText(std::to_string(wordSpeed), interfaceFont, 36);
    speedText.setPosition(500, 240);
    speedText.setFillColor(sf::Color(255, 255, 140));

    sf::Text maxWordFrequencyText(std::to_string(maxWordFrequency), interfaceFont, 36);
    maxWordFrequencyText.setPosition(500, 300);
    maxWordFrequencyText.setFillColor(sf::Color(255, 255, 140));

    sf::Text pointerText("<", interfaceFont, 36);
    pointerText.setPosition(495, 240);

    optionsScreen_Buttons[0].setPosition(100, 120);
    optionsScreen_Buttons[1].setPosition(100, 240);
    optionsScreen_Buttons[2].setPosition(100, 300);
    optionsScreen_Buttons[3].setPosition(100, 360);

    // Game over screen buttons
    int gameOverScreen_CurrentIndex = 0;
    std::vector<sf::Text> gameOverScreen_Buttons = {
            sf::Text("Play Again", interfaceFont, 36),
            sf::Text("Scoreboard", interfaceFont, 30),
            sf::Text("Options", interfaceFont, 30),
            sf::Text("Quit", interfaceFont, 30)
    };

    gameOverScreen_Buttons[0].setPosition(315, 320);
    gameOverScreen_Buttons[1].setPosition(320, 380);
    gameOverScreen_Buttons[2].setPosition(345, 440);
    gameOverScreen_Buttons[3].setPosition(360, 520);


    sf::Text versionText("v0.1.0-alpha", arialFont, 12);
    versionText.setPosition(5, static_cast<float>(window.getSize().y - versionText.getCharacterSize() * 1.5));
    versionText.setFillColor(sf::Color(120, 120, 120));
    versionText.setStyle(sf::Text::Italic);

    sf::Clock timeBetweenMenus;

    // Filling top scores vector with empty strings for now
    std::vector<sf::Text> topScoresTexts;
    std::vector<sf::Text> topScoresHeaderTexts;
    for (int i = 0; i < 10; ++i) {
        sf::Text text("", interfaceFont, 24);
        text.setPosition(10, static_cast<float>(window.getSize().y - (10 - i) * text.getCharacterSize() * 2));
        topScoresTexts.push_back(text);
    }

    // Predefined positions for topScoresTexts columns
    float nicknameX = 10;
    float scoreX = nicknameX + 150;
    float wpmX = scoreX + 100;
    float speedX = wpmX + 100;
    float maxFreqX = speedX + 100;
    float timeX = maxFreqX + 100;
    float dateX = timeX + 100;

    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            startingScreen_PreviousIndex = startingScreen_CurrentIndex;
            if (gameState == Screen::Starting && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && timeBetweenMenus.getElapsedTime().asSeconds() > 1) {
                    if (startingScreen_CurrentIndex == 0) {
                        nickname = userInput.getString();
                        inputStr.clear();
                        userInput.setCharacterSize(24);
                        userInput.setString("");
                        userInput.setPosition(13, static_cast<float>(window.getSize().y - userInput.getCharacterSize() * 1.5));
                        userInput.setFillColor(sf::Color::White);
                        cursor.setSize(sf::Vector2f(14, 2));

                        if (nickname.empty())
                            nickname = "Guest" + std::to_string(my_library::getRandomInt(1000, 9999));

                        scoreSaved = false;
                        scoreDownloaded = false;
                        gameClock.restart();
                        gameState = Screen::Game;
                        timeBetweenMenus.restart();
                    } else if (startingScreen_CurrentIndex == 1) {
                        gameState = Screen::Score;
                        timeBetweenMenus.restart();
                    }
                    else if (startingScreen_CurrentIndex == 2) {
                        userInput.setCharacterSize(36);
                        userInput.setPosition(455, 123);
                        inputStr = nickname;
                        userInput.setFillColor(sf::Color(255, 255, 140));
                        cursor.setSize(sf::Vector2f(21, 3));
                        gameState = Screen::Options;
                        timeBetweenMenus.restart();
                    } else if (startingScreen_CurrentIndex == 3) {
                        window.close();
                    }
                    previousGameState = Screen::Starting;
                } else if (event.key.code == sf::Keyboard::Up) {
                    if (startingScreen_CurrentIndex == 0)
                        startingScreen_CurrentIndex = static_cast<int>(startingScreen_Buttons.size()) - 1;
                    else
                        startingScreen_CurrentIndex--;
                } else if (event.key.code == sf::Keyboard::Down) {
                    if (startingScreen_CurrentIndex == startingScreen_Buttons.size() - 1)
                        startingScreen_CurrentIndex = 0;
                    else
                        startingScreen_CurrentIndex++;
                }
            }
            if (gameState == Screen::Options && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && timeBetweenMenus.getElapsedTime().asSeconds() > 1) {
                    if (previousGameState == Screen::Starting) {
                        nickname = userInput.getString();
                        gameState = Screen::Starting;
                        timeBetweenMenus.restart();
                    } else if (previousGameState == Screen::GameOver) {
                        nickname = userInput.getString();
                        gameState = Screen::GameOver;
                        timeBetweenMenus.restart();
                    }
                    previousGameState = Screen::Options;
                } else if (event.key.code == sf::Keyboard::Up) {
                    if (optionsScreen_CurrentIndex == 0)
                        optionsScreen_CurrentIndex = static_cast<int>(optionsScreen_Buttons.size()) - 1;
                    else
                        optionsScreen_CurrentIndex--;
                } else if (event.key.code == sf::Keyboard::Down) {
                    if (optionsScreen_CurrentIndex == optionsScreen_Buttons.size() - 1)
                        optionsScreen_CurrentIndex = 0;
                    else
                        optionsScreen_CurrentIndex++;
                } else if (event.key.code == sf::Keyboard::Left) {
                    if (optionsScreen_CurrentIndex == 1) {
                        wordSpeed = std::max(wordSpeed - 0.01f, 0.01f);
                    } else if (optionsScreen_CurrentIndex == 2) {
                        maxWordFrequency = std::ceil((maxWordFrequency - 0.1f) * 10.0f) / 10.0f;    // Rounding to fix floating-point issues
                        maxWordFrequency = std::max(maxWordFrequency, 0.5f);
                    } else if (optionsScreen_CurrentIndex == 3) {
                        if (currentFontIndex != 0)
                            currentFontIndex--;
                    }
                } else if (event.key.code == sf::Keyboard::Right) {
                    if (optionsScreen_CurrentIndex == 1) {
                        wordSpeed = std::min(wordSpeed + 0.01f, 0.1f);
                    } else if (optionsScreen_CurrentIndex == 2) {
                        maxWordFrequency = std::min(maxWordFrequency + 0.1f, 1.0f);
                    } else if (optionsScreen_CurrentIndex == 3) {
                        if (currentFontIndex != fonts.size() - 1)
                            currentFontIndex++;
                    }
                }
            }

            // Setting fonts
            for (auto& word : words)
                word.text.setFont(fonts[currentFontIndex]);

            speedText.setString("  " + std::to_string(static_cast<int>(wordSpeed * 100)));
            maxWordFrequencyText.setString("  " + std::to_string(static_cast<int>(maxWordFrequency * 10)));

            if (gameState == Screen::Options && optionsScreen_CurrentIndex == 0 && event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128 && event.text.unicode != ' ') {

                    //Handling: Backspace
                    if (event.text.unicode == '\b') {
                        if (!inputStr.empty())
                            inputStr.pop_back();
                    } else if (inputStr.size() <= 20 && event.text.unicode != '\r') {
                        inputStr += static_cast<char>(event.text.unicode);
                    }
                }
            } else if (gameState == Screen::Game && event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128 && event.text.unicode != ' ') {

                    //Handling: Backspace
                    if (event.text.unicode == '\b') {
                        if (!inputStr.empty())
                            inputStr.pop_back();

                        // Handling: Enter
                    } else if (event.text.unicode == '\r') {
                        for (auto& word : words) {
                            if (my_library::equalsIgnoreCase(word.text.getString(), inputStr) and !word.isFinished) {
                                score = wordsFinished * static_cast<int>(static_cast<float>(wpm) * wordSpeed * maxWordFrequency * 20); // score aktualizowany po trafieniu slowa
                                word.isFinished = true;
                                wordsFinished++;
                                break;
                            }
                        }
                        inputStr.clear();
                    } else {
                        if (userInput.getGlobalBounds().width < decor_userInput.getGlobalBounds().width - 36)
                            inputStr += static_cast<char>(event.text.unicode);
                    }
                }
            } else if (gameState == Screen::GameOver && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && timeBetweenMenus.getElapsedTime().asSeconds() > 1) {
                    if (gameOverScreen_CurrentIndex == 0) {
                        scoreText.setPosition(370, static_cast<float>(window.getSize().y - scoreText.getCharacterSize() * 1.4));
                        scoreValueText.setPosition(scoreText.getPosition().x + 5, scoreText.getPosition().y);
                        scoreValueText.setFillColor(sf::Color(255, 255, 140));

                        userInput.setCharacterSize(24);
                        userInput.setString("");
                        userInput.setPosition(13, static_cast<float>(window.getSize().y - userInput.getCharacterSize() * 1.5));
                        userInput.setFillColor(sf::Color::White);
                        cursor.setSize(sf::Vector2f(14, 2));

                        std::ranges::shuffle(words, std::random_device());

                        for (auto& word : words) {
                            word.isFinished = true;
                            word.text.setPosition(-word.text.getGlobalBounds().width, 0);
                            word.text.setFillColor(sf::Color(10, 255, 140));
                            wordIter = words.begin();
                        }
                        score = 0;
                        wpm = 0;
                        missedCount = 0;
                        wordsFinished = 0;
                        scoreSaved = false;
                        scoreDownloaded = false;

                        gameClock.restart();
                        gameState = Screen::Game;
                    } else if (gameOverScreen_CurrentIndex == 1) {
                        scoreDownloaded = false;
                        gameState = Screen::Score;
                    } else if (gameOverScreen_CurrentIndex == 2) {
                        userInput.setCharacterSize(36);
                        userInput.setPosition(455, 123);
                        inputStr = nickname;

                        userInput.setFillColor(sf::Color(255, 255, 140));
                        cursor.setSize(sf::Vector2f(21, 3));

                        gameState = Screen::Options;
                    } else if (gameOverScreen_CurrentIndex == 3) {
                        window.close();
                    }
                    previousGameState = Screen::GameOver;
                } else if (event.key.code == sf::Keyboard::Up) {
                    if (gameOverScreen_CurrentIndex == 0)
                        gameOverScreen_CurrentIndex = static_cast<int>(gameOverScreen_Buttons.size()) - 1;
                    else
                        gameOverScreen_CurrentIndex--;
                } else if (event.key.code == sf::Keyboard::Down) {
                    if (gameOverScreen_CurrentIndex == gameOverScreen_Buttons.size() - 1)
                        gameOverScreen_CurrentIndex = 0;
                    else {
                        gameOverScreen_CurrentIndex++;
                    }
                }
            } else if (gameState == Screen::Score && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && timeBetweenMenus.getElapsedTime().asSeconds() > 1) {
                    if (previousGameState == Screen::Starting) {
                        gameState = Screen::Starting;
                        timeBetweenMenus.restart();
                    }
                    else if (previousGameState == Screen::GameOver) {
                        gameState = Screen::GameOver;
                        timeBetweenMenus.restart();
                    }
                    previousGameState = Screen::Score;
                }
            }
        }

        // Render logic based on game state
        if (gameState == Screen::Starting) {
            window.clear();

            window.draw(menuSprite);

            pointerText.setPosition(300, static_cast<float>(240 + 60 * startingScreen_CurrentIndex));
            for (const auto& text : startingScreen_Buttons) {
                window.draw(text);
            }
            window.draw(stanTyperText);
            window.draw(pointerText);
            window.draw(versionText);
            window.display();
        } else if (gameState == Screen::Options) {
            window.clear();

            window.draw(menuSprite);

            userInput.setString(inputStr);
            cursor.setPosition(userInput.getGlobalBounds().width + 458, userInput.getPosition().y + 36);

            pointerText.setPosition(optionsScreen_CurrentIndex == 0 ? 425 : 490, static_cast<float>(
                    optionsScreen_CurrentIndex == 0 ? 125 :
                    optionsScreen_CurrentIndex == 1 ? 240 :
                    optionsScreen_CurrentIndex == 2 ? 300 : 360)
            );

            for (const auto& text : optionsScreen_Buttons) {
                window.draw(text);
            }

            // Displaying cursor
            if (optionsScreen_CurrentIndex == 0) {
                if (cursorClock.getElapsedTime().asSeconds() >= 0.3f) {
                    cursorVisible = !cursorVisible;
                    cursorClock.restart();
                }
                if (cursorVisible)
                    window.draw(cursor);
            }

            window.draw(userInput);
            window.draw(speedText);
            window.draw(maxWordFrequencyText);
            window.draw(fontsTexts[currentFontIndex]);
            window.draw(pointerText);
            window.display();
        } else if (gameState == Screen::Game) {
            window.clear();

            // Drawing animation
            if (animationClock.getElapsedTime().asSeconds() >= frameDuration) {
                currentFrame = (currentFrame + 1) % static_cast<int>(animationFrames.size());
                animationSprite.setTexture(animationFrames[currentFrame]);
                animationClock.restart();
            }
            window.draw(animationSprite);

            userInput.setString(inputStr);
            cursor.setPosition(userInput.getGlobalBounds().width + 16, userInput.getPosition().y + 24);
            wordTime += wordClock.restart();

            // Adding words
            if (wordTime.asSeconds() >= wordFrequency && wordIter != words.end()) {
                bool positionFound = false;
                while (!positionFound) {
                    wordIter->text.setPosition(-wordIter->text.getGlobalBounds().width, static_cast<float>(my_library::getRandomInt(20, 530)));
                    positionFound = true;

                    // Checking for intersections
                    for (auto iter = words.begin(); iter != wordIter; iter++) {
                        if (iter != wordIter && my_library::intersects(*iter, *wordIter)) {
                            positionFound = false;
                            break;
                        }
                    }
                }
                wordIter->isFinished = false;
                wordIter++;
                wordFrequency = std::max(wordFrequency - 0.05f, maxWordFrequency);
                wordTime = sf::Time::Zero;
            }


            // Displaying words
            for (auto iterator = words.begin(); iterator != wordIter; iterator++) {
                if (!iterator->isFinished) {
                    window.draw(iterator->text);
                    iterator->text.move(wordSpeed, 0);

                    // Coloring words
                    if (iterator->text.getPosition().x >= window.getSize().x / 1.8)
                        iterator->text.setFillColor(sf::Color(255, 255, 140)); // Yellow
                    if (iterator->text.getPosition().x >= window.getSize().x / 1.2)
                        iterator->text.setFillColor(sf::Color(255, 110, 110)); // Red

                        // When word leaves the screen
                    if (iterator->text.getPosition().x > static_cast<float>(window.getSize().x)) {
                        bool positionFound = false;
                        while (!positionFound) {
                            iterator->text.setPosition(-iterator->text.getGlobalBounds().width, static_cast<float>(my_library::getRandomInt(20, 530)));
                            positionFound = true;

                            // Checking for intersections
                            for (auto iter = words.begin(); iter != wordIter; iter++) {
                                if (iterator != iter && my_library::intersects(*iterator, *iter)) {
                                    positionFound = false;
                                    break;
                                }
                            }
                        }

                        // Color reset
                        iterator->text.setFillColor(sf::Color(10, 255, 140));
                        missedCount++;
                    }
                }
            }

            // Setting time string
            if (gameClock.getElapsedTime().asSeconds() >= 10)
                timeValueText.setString("        " + fmt::format("{:.2f}", gameClock.getElapsedTime().asSeconds()));
            else if (gameClock.getElapsedTime().asSeconds() >= 100)
                timeValueText.setString("         " + fmt::format("{:.2f}", gameClock.getElapsedTime().asSeconds()));
            else if (gameClock.getElapsedTime().asSeconds() >= 1000)
                timeValueText.setString("          " + fmt::format("{:.2f}", gameClock.getElapsedTime().asSeconds()));
            else
                timeValueText.setString("           " + fmt::format("{:.2f}", gameClock.getElapsedTime().asSeconds()));

            wpm = gameClock.getElapsedTime().asSeconds() != 0 ? static_cast<int>(static_cast<float>(wordsFinished) / gameClock.getElapsedTime().asSeconds() * 60) : 0;
            missedValueText.setString("         " + std::to_string(missedCount));

            // Displaying cursor
            if (cursorClock.getElapsedTime().asSeconds() >= 0.3f) {
                cursorVisible = !cursorVisible;
                cursorClock.restart();
            }
            if (cursorVisible)
                window.draw(cursor);

            scoreValueText.setString("       " + std::to_string(score));
            wordsValueText.setString("       " + std::to_string(wordsFinished) + "/" + std::to_string(words.size()));

            window.draw(scoreText);
            window.draw(scoreValueText);
            window.draw(missedText);
            window.draw(missedValueText);
            window.draw(wordsText);
            window.draw(wordsValueText);
            window.draw(timeText);
            window.draw(timeValueText);
            window.draw(decor_userInput);
            window.draw(userInput);

            window.display();

            if (missedCount == 10) {
                gameState = Screen::GameOver;

                gameOverText.setString("Game Over");
                gameOverText.setFillColor(sf::Color::Red);

                screenTexture.create(window.getSize().x, window.getSize().y);
                screenTexture.update(window);
                screenSprite.setTexture(screenTexture);

                if (!scoreSaved) {
                    std::time_t t = std::time(nullptr);
                    std::tm* date = std::localtime(&t);

                    auto game = Game(nickname, score, wpm, static_cast<int>(wordSpeed * 100), static_cast<int>(maxWordFrequency * 10), gameClock.getElapsedTime().asSeconds(), my_library::timeToStr(*date));
                    game.saveScoreToFile();
                    scoreSaved = true;
                }
            } else if (wordsFinished == words.size()) {
                gameState = Screen::GameOver;

                gameOverText.setString("You win!");
                gameOverText.setFillColor(sf::Color(10, 255, 140));

                screenTexture.create(window.getSize().x, window.getSize().y);
                screenTexture.update(window);
                screenSprite.setTexture(screenTexture);

                if (!scoreSaved) {
                    std::time_t t = std::time(nullptr);
                    std::tm* date = std::localtime(&t);

                    auto game = Game(nickname, score, wpm, static_cast<int>(wordSpeed * 100), static_cast<int>(maxWordFrequency * 10), gameClock.getElapsedTime().asSeconds(), my_library::timeToStr(*date));
                    game.saveScoreToFile();
                    scoreSaved = true;
                }
            }
        } else if (gameState == Screen::GameOver) {
            window.clear();

            if (!scoreDownloaded) {
                auto tempVec = my_library::getTopScorers();

                for (int i = 0; i < tempVec.size() && i < 11; ++i) {
                    float currentY = static_cast<float>(window.getSize().y) - static_cast<float>(i) * 52;
                    if (i == 0) {
                        sf::Text nicknameHeaderText("Nickname:", interfaceFont, 30);
                        nicknameHeaderText.setPosition(nicknameX, 10);
                        nicknameHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(nicknameHeaderText);

                        sf::Text scoreHeaderText("Score:", interfaceFont, 30);
                        scoreHeaderText.setPosition(scoreX, 10);
                        scoreHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(scoreHeaderText);

                        sf::Text wpmHeaderText("WPM:", interfaceFont, 30);
                        wpmHeaderText.setPosition(wpmX, 10);
                        wpmHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(wpmHeaderText);

                        sf::Text speedHeaderText("Speed:", interfaceFont, 30);
                        speedHeaderText.setPosition(speedX, 10);
                        speedHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(speedHeaderText);

                        sf::Text maxFreqHeaderText("Freq.:", interfaceFont, 30);
                        maxFreqHeaderText.setPosition(maxFreqX, 10);
                        maxFreqHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(maxFreqHeaderText);

                        sf::Text timeHeaderText("Time:", interfaceFont, 30);
                        timeHeaderText.setPosition(timeX, 10);
                        timeHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(timeHeaderText);

                        sf::Text dateHeaderText("Date:", interfaceFont, 30);
                        dateHeaderText.setPosition(dateX, 10);
                        dateHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(dateHeaderText);
                    } else {
                        if (!tempVec.empty()) {
                            sf::Text nicknameValText(tempVec[i - 1].nickname, interfaceFont, 24);
                            nicknameValText.setPosition(nicknameX, currentY);
                            topScoresTexts.push_back(nicknameValText);

                            sf::Text scoreValText(std::to_string(tempVec[i - 1].score), interfaceFont, 24);
                            scoreValText.setPosition(scoreX, currentY);
                            topScoresTexts.push_back(scoreValText);

                            sf::Text wpmValText(std::to_string(tempVec[i - 1].wpm), interfaceFont, 24);
                            wpmValText.setPosition(wpmX, currentY);
                            topScoresTexts.push_back(wpmValText);

                            sf::Text speedValText(std::to_string(tempVec[i - 1].speed), interfaceFont, 24);
                            speedValText.setPosition(speedX, currentY);
                            topScoresTexts.push_back(speedValText);

                            sf::Text maxFreqValText(std::to_string(tempVec[i - 1].maxFreq), interfaceFont, 24);
                            maxFreqValText.setPosition(maxFreqX, currentY);
                            topScoresTexts.push_back(maxFreqValText);

                            sf::Text timeValText(fmt::format("{:.2f}s", tempVec[i - 1].time), interfaceFont, 24);
                            timeValText.setPosition(timeX, currentY);
                            topScoresTexts.push_back(timeValText);

                            sf::Text dateValText(tempVec[i - 1].date, interfaceFont, 24);
                            dateValText.setPosition(dateX, currentY);
                            topScoresTexts.push_back(dateValText);
                        }
                    }
                }
                scoreDownloaded = true;
            }

            pointerText.setPosition(280, static_cast<float>(
                    gameOverScreen_CurrentIndex == 0 ? 320 :
                    gameOverScreen_CurrentIndex == 1 ? 380 :
                    gameOverScreen_CurrentIndex == 2 ? 440 : 520)
            );

            scoreText.setPosition(350, 200);
            scoreValueText.setFillColor(sf::Color::White);
            scoreValueText.setPosition(scoreText.getPosition().x + 5, scoreText.getPosition().y);
            wpmText.setString("WPM: " + std::to_string(wpm));

            window.draw(screenSprite);
            window.draw(gameOverOverlay);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(scoreValueText);
            window.draw(wpmText);
            window.draw(pointerText);

            for (const auto& text : gameOverScreen_Buttons) {
                window.draw(text);
            }
            window.display();
        } else if (gameState == Screen::Score) {
            window.clear();
            window.draw(menuSprite);

            if (!scoreDownloaded) {
                topScoresTexts.clear();
                topScoresHeaderTexts.clear();
                auto tempVec = my_library::getTopScorers();

                for (int i = 0; i <= tempVec.size() && i <= 11; ++i) {
                    float currentY = static_cast<float>(window.getSize().y) - static_cast<float>(i) * 52;
                    if (i == 0) {
                        sf::Text nicknameHeaderText("Nickname:", interfaceFont, 30);
                        nicknameHeaderText.setPosition(nicknameX, 10);
                        nicknameHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(nicknameHeaderText);

                        sf::Text scoreHeaderText("Score:", interfaceFont, 30);
                        scoreHeaderText.setPosition(scoreX, 10);
                        scoreHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(scoreHeaderText);

                        sf::Text wpmHeaderText("WPM:", interfaceFont, 30);
                        wpmHeaderText.setPosition(wpmX, 10);
                        wpmHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(wpmHeaderText);

                        sf::Text speedHeaderText("Speed:", interfaceFont, 30);
                        speedHeaderText.setPosition(speedX, 10);
                        speedHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(speedHeaderText);

                        sf::Text maxFreqHeaderText("Freq.:", interfaceFont, 30);
                        maxFreqHeaderText.setPosition(maxFreqX, 10);
                        maxFreqHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(maxFreqHeaderText);

                        sf::Text timeHeaderText("Time:", interfaceFont, 30);
                        timeHeaderText.setPosition(timeX, 10);
                        timeHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(timeHeaderText);

                        sf::Text dateHeaderText("Date:", interfaceFont, 30);
                        dateHeaderText.setPosition(dateX, 10);
                        dateHeaderText.setFillColor(sf::Color(255, 255, 140));
                        topScoresHeaderTexts.push_back(dateHeaderText);
                    } else {
                        if (!tempVec.empty()) {
                            sf::Text nicknameValText(tempVec[i - 1].nickname, interfaceFont, 24);
                            nicknameValText.setPosition(nicknameX, currentY);
                            topScoresTexts.push_back(nicknameValText);

                            sf::Text scoreValText(std::to_string(tempVec[i - 1].score), interfaceFont, 24);
                            scoreValText.setPosition(scoreX, currentY);
                            topScoresTexts.push_back(scoreValText);

                            sf::Text wpmValText(std::to_string(tempVec[i - 1].wpm), interfaceFont, 24);
                            wpmValText.setPosition(wpmX, currentY);
                            topScoresTexts.push_back(wpmValText);

                            sf::Text speedValText(std::to_string(tempVec[i - 1].speed), interfaceFont, 24);
                            speedValText.setPosition(speedX, currentY);
                            topScoresTexts.push_back(speedValText);

                            sf::Text maxFreqValText(std::to_string(tempVec[i - 1].maxFreq), interfaceFont, 24);
                            maxFreqValText.setPosition(maxFreqX, currentY);
                            topScoresTexts.push_back(maxFreqValText);

                            sf::Text timeValText(fmt::format("{:.2f}s", tempVec[i - 1].time), interfaceFont, 24);
                            timeValText.setPosition(timeX, currentY);
                            topScoresTexts.push_back(timeValText);

                            sf::Text dateValText(tempVec[i - 1].date, interfaceFont, 24);
                            dateValText.setPosition(dateX, currentY);
                            topScoresTexts.push_back(dateValText);
                        }
                    }
                }
                scoreDownloaded = true;
            }
            for (auto const& element : topScoresTexts) {
                window.draw(element);
            }
            for (auto const& element : topScoresHeaderTexts) {
                window.draw(element);
            }
            window.display();
        }
    }
}
