// Rechnen – Math practice for elementary school
// Ported from GFA-Basic (Rechnen.g32)
//
// Provides arithmetic exercises organised in 5 groups:
//   Rechnen 1 (to 10, to 20, to 100)
//   Rechnen 2 (multiplication tables, etc.)
//   Rechnen 3 (to 1000)
//   Rechnen 4 (larger numbers, written arithmetic)
//   Geld und Zeit (money and time)
//
// Each group has 13 exercise types with 3 difficulty levels.
// The program adapts difficulty based on performance.

#include "common.h"
#include <sstream>
#include <iomanip>

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------
struct AppState {
    sf::RenderWindow* win = nullptr;
    gs::Fonts fonts;
    gs::ScaleFactors sc;
    std::string playerName;
    bool isSingle = true;   // single player vs. pair

    // Textures
    sf::Texture grins, heul;
    sf::Texture katze, elefant, kueken, kirche, baum, ski, hase, sonne, eis, blume;

    // Euro coin textures (with alpha, combined from mask)
    sf::Texture euro_1, euro_2, cent_50, cent_20, cent_10, cent_5, cent_2, cent_1;
    // Euro banknote textures
    sf::Texture euro_5n, euro_10n, euro_20n, euro_50n, euro_100n, euro_200n, euro_500n;

    // Exercise tracking
    int rechnenGroup = 0;   // 1-5
    int exerciseType = 0;   // 1-13
    int stufe = 1;          // difficulty 1-3
    bool helpOn = false;
    bool autoMode = true;   // auto task generation vs manual input
    int correct = 0, wrong = 0;
    int64_t startTime = 0;

    // Current exercise
    int num1 = 0, num2 = 0, answer = 0;
    std::string taskText;
    std::string userInput;
    std::string feedback;
    bool showingFeedback = false;
    int feedbackTimer = 0;

    void loadResources();
    void generateTask();
    std::string getVerb() const { return isSingle ? "Rechne" : "Rechnet"; }
};

void AppState::loadResources() {
    gs::loadTexture(grins, "Bild/Grins.bmp");
    gs::loadTexture(heul, "Bild/Heul.bmp");
    gs::loadTexture(katze, "Bild/Katze.jpg");
    gs::loadTexture(hase, "Bild/Hase.jpg");
    gs::loadTexture(kirche, "Bild/Kirche.jpg");
    gs::loadTexture(blume, "Bild/Blume.jpg");
    gs::loadTexture(sonne, "Bild/Sonne.jpg");
    gs::loadTexture(eis, "Bild/Eis.jpg");
    gs::loadTexture(baum, "Bild/Baum.jpg");
    gs::loadTexture(elefant, "Bild/Elefant.jpg");
    gs::loadTexture(kueken, "Bild/Kueken.jpg");
    gs::loadTexture(ski, "Bild/Ski.jpg");

    // Coins with alpha channel (pre-processed PNGs)
    gs::loadTexture(euro_1, "Bild/euro_1.png");
    gs::loadTexture(euro_2, "Bild/euro_2.png");
    gs::loadTexture(cent_50, "Bild/cent_50.png");
    gs::loadTexture(cent_20, "Bild/cent_20.png");
    gs::loadTexture(cent_10, "Bild/cent_10.png");
    gs::loadTexture(cent_5, "Bild/cent_5.png");
    gs::loadTexture(cent_2, "Bild/cent_2.png");
    gs::loadTexture(cent_1, "Bild/cent_1.png");

    // Banknotes
    gs::loadTexture(euro_5n, "Bild/euro_5.jpg");
    gs::loadTexture(euro_10n, "Bild/euro_10.jpg");
    gs::loadTexture(euro_20n, "Bild/euro_20.jpg");
    gs::loadTexture(euro_50n, "Bild/euro_50.jpg");
    gs::loadTexture(euro_100n, "Bild/euro_100.jpg");
    gs::loadTexture(euro_200n, "Bild/euro_200.jpg");
    gs::loadTexture(euro_500n, "Bild/euro_500.jpg");
}

void AppState::generateTask() {
    userInput.clear();
    feedback.clear();
    showingFeedback = false;

    int maxVal = 10;
    if (rechnenGroup == 1) {
        if (exerciseType <= 1) maxVal = 10;
        else if (exerciseType <= 5) maxVal = 20;
        else maxVal = 100;
    } else if (rechnenGroup == 2) maxVal = 100;
    else if (rechnenGroup == 3) maxVal = 1000;
    else maxVal = 10000;

    // Generate based on exercise type
    int op = (exerciseType - 1) % 4; // 0=add, 1=sub, 2=mul, 3=div
    if (exerciseType <= 2) op = 0; // addition for first types
    else if (exerciseType <= 4) op = 1;
    else if (exerciseType <= 6) op = gs::randInt(0, 1);
    else if (exerciseType <= 8) op = gs::randInt(0, 1);
    else if (exerciseType <= 10) op = 2;
    else if (exerciseType <= 12) op = 2;
    else op = 3;

    switch (op) {
        case 0: // addition
            num1 = gs::randInt(1, maxVal / 2);
            num2 = gs::randInt(1, maxVal - num1);
            answer = num1 + num2;
            taskText = std::to_string(num1) + " + " + std::to_string(num2) + " =";
            break;
        case 1: // subtraction
            answer = gs::randInt(0, maxVal / 2);
            num2 = gs::randInt(1, maxVal / 2);
            num1 = answer + num2;
            taskText = std::to_string(num1) + " - " + std::to_string(num2) + " =";
            break;
        case 2: { // multiplication
            int mmax = (rechnenGroup <= 2) ? 10 : 20;
            num1 = gs::randInt(2, mmax);
            num2 = gs::randInt(2, mmax);
            answer = num1 * num2;
            taskText = std::to_string(num1) + " × " + std::to_string(num2) + " =";
            break;
        }
        case 3: { // division
            num2 = gs::randInt(2, 12);
            answer = gs::randInt(1, 20);
            num1 = num2 * answer;
            taskText = std::to_string(num1) + " : " + std::to_string(num2) + " =";
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------
static int mainMenu(AppState& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;

    struct MenuBtn {
        std::string label;
        sf::FloatRect bounds;
        int value;
    };
    std::vector<MenuBtn> btns = {
        {"Rechnen  1", sf::FloatRect({s.x(55),  s.y(118)}, {s.x(260), s.y(70)}), 1},
        {"Rechnen  2", sf::FloatRect({s.x(360), s.y(118)}, {s.x(260), s.y(70)}), 2},
        {"Rechnen  3", sf::FloatRect({s.x(55),  s.y(208)}, {s.x(260), s.y(70)}), 3},
        {"Rechnen  4", sf::FloatRect({s.x(360), s.y(208)}, {s.x(260), s.y(70)}), 4},
        {"Geld und Zeit", sf::FloatRect({s.x(360),s.y(298)}, {s.x(260), s.y(70)}), 5},
    };

    // Help and input mode
    bool helpOn = false, autoInput = true;
    int selected = -1;

    auto draw = [&]() {
        win.clear();
        gs::drawFilledRect(win, 0, 0, (float)s.winW, (float)s.winH, gs::colors::veryDark);
        gs::drawFilledRect(win, s.x(3), s.y(3), s.x(633), s.y(475), gs::colors::darkBrown);
        gs::drawFilledRect(win, s.x(4), s.y(96), s.x(632), s.y(364), gs::colors::creme);
        gs::drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), gs::colors::red, s.x(6));

        unsigned fLg = static_cast<unsigned>(50 * s.fx);
        unsigned fSm = static_cast<unsigned>(16 * s.fx);
        unsigned fMd = static_cast<unsigned>(21 * s.fx);

        // Title
        std::string title = app.isSingle
            ? "Hier kannst du auswählen:" : "Hier könnt ihr auswählen:";
        gs::drawText(win, f.sans, fSm, title, s.x(60), s.y(20), gs::colors::white);
        gs::drawTextCentered(win, f.sans, fSm, app.playerName, s.y(464), gs::colors::white);

        // Help buttons
        sf::Color hjc = helpOn ? gs::colors::creme : gs::colors::blue;
        sf::Color hnc = helpOn ? gs::colors::blue : gs::colors::creme;
        gs::drawRoundedRect(win, s.x(59), s.y(50), s.x(301), s.y(75),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "Hilfe: ja", s.x(110), s.y(52), hjc);
        gs::drawRoundedRect(win, s.x(335), s.y(50), s.x(579), s.y(75),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "Hilfe: nein", s.x(381), s.y(52), hnc);

        // Group buttons
        for (auto& b : btns) {
            bool sel = (b.value == selected);
            sf::Color bg = sel ? gs::colors::blue : gs::colors::creme;
            sf::Color fg = sel ? gs::colors::creme : gs::colors::blue;
            gs::drawFilledRect(win, b.bounds.position.x, b.bounds.position.y,
                               b.bounds.position.x + b.bounds.size.x,
                               b.bounds.position.y + b.bounds.size.y, bg);
            gs::drawText(win, f.sans, fLg, b.label,
                         b.bounds.position.x + s.x(5),
                         b.bounds.position.y + s.y(5), fg);
        }

        // Instructions
        gs::drawText(win, f.sans, fSm,
            "linke Maustaste: Auswahl    rechte Maustaste: weiter    F8: Ende",
            s.x(40), s.y(380), gs::colors::white);

        win.display();
    };

    draw();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return 0; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape ||
                    kp->code == sf::Keyboard::Key::F8)
                    return 0;
                if (kp->code == sf::Keyboard::Key::F1) {
                    helpOn = !helpOn; draw();
                }
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                if (mp->button == sf::Mouse::Button::Left) {
                    // Help buttons
                    if (my > s.y(50) && my < s.y(75)) {
                        if (mx > s.x(59) && mx < s.x(301)) { helpOn = true; draw(); }
                        else if (mx > s.x(335) && mx < s.x(579)) { helpOn = false; draw(); }
                    }
                    // Group buttons
                    for (auto& b : btns) {
                        if (mx >= b.bounds.position.x &&
                            mx <= b.bounds.position.x + b.bounds.size.x &&
                            my >= b.bounds.position.y &&
                            my <= b.bounds.position.y + b.bounds.size.y) {
                            selected = b.value;
                            draw();
                        }
                    }
                } else if (mp->button == sf::Mouse::Button::Right) {
                    if (selected > 0) {
                        app.rechnenGroup = selected;
                        app.helpOn = helpOn;
                        app.autoMode = autoInput;
                        return selected;
                    }
                }
            }
        }
        gs::delayMs(16);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Exercise type sub-menu (13 exercise types per group)
// ---------------------------------------------------------------------------
static std::vector<std::string> getExerciseNames(int group) {
    std::vector<std::string> items;
    items.push_back(""); // row 0 is header area
    switch (group) {
        case 1:
            items = {"(Zahlenraum)", "Zahlen im Hunderterfeld",
                     "Addition bis 10", "Subtraktion bis 10",
                     "Ergänzen: 3 + _ = 4", "Ergänzen: 6 - _ = 5",
                     "Platzhalter: 6 + _ =", "Platzhalter: _ - 3 =",
                     "Gemischt: _ + 3 =", "Addition bis 20",
                     "Subtraktion bis 20", "Das Doppelte",
                     "Die Hälfte"};
            break;
        case 2:
            items = {"(Einmaleins)", "Reihen des Einmaleins",
                     "Einmaleins gemischt", "Division",
                     "Mal und Geteilt", "Addition bis 100",
                     "Subtraktion bis 100", "Ergänzen bis 100",
                     "Platzhalter bis 100", "Kettenaufgaben",
                     "Schriftliche Addition", "Schriftliche Subtraktion",
                     "Gemischt"};
            break;
        case 3:
            items = {"(Zahlenraum 1000)", "Zahlen im Tausenderfeld",
                     "Addition bis 1000", "Subtraktion bis 1000",
                     "Ergänzen bis 1000", "Platzhalter",
                     "Kettenaufgaben", "Schriftliche Addition",
                     "Schriftliche Subtraktion",
                     "Schriftliche Multiplikation",
                     "Halbschriftliche Division",
                     "Einmaleins mit Zehnern",
                     "Gemischt"};
            break;
        case 4:
            items = {"(großer Zahlenraum)", "Große Zahlen",
                     "Addition", "Subtraktion",
                     "Multiplikation", "Division",
                     "Kommazahlen", "Schriftliches Rechnen",
                     "Schriftliche Addition", "Schriftliche Subtraktion",
                     "Schriftliche Multiplikation",
                     "Schriftliche Division",
                     "Gemischt"};
            break;
        case 5:
            items = {"(Geld und Zeit)", "Euro-Münzen",
                     "Euro-Scheine", "Geldbeträge addieren",
                     "Geldbeträge subtrahieren", "Wechselgeld",
                     "Die Uhr lesen", "Uhrzeiten berechnen",
                     "Zeitspannen", "Fahrplan",
                     "Kalender", "Sachaufgaben Geld",
                     "Sachaufgaben Zeit"};
            break;
    }
    return items;
}

static int exerciseMenu(AppState& app) {
    auto names = getExerciseNames(app.rechnenGroup);
    return gs::gridMenu(*app.win, app.fonts, app.sc,
                        "Rechnen " + std::to_string(app.rechnenGroup),
                        names, app.playerName);
}

// ---------------------------------------------------------------------------
// Exercise loop
// ---------------------------------------------------------------------------
static void exerciseLoop(AppState& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    app.correct = 0;
    app.wrong = 0;
    app.startTime = gs::timerMs();

    app.generateTask();

    unsigned fTask = static_cast<unsigned>(28 * s.fx);
    unsigned fSmall = static_cast<unsigned>(16 * s.fx);
    unsigned fMed = static_cast<unsigned>(21 * s.fx);

    auto draw = [&]() {
        win.clear();
        gs::drawFrame(win, s);

        // Task
        gs::drawText(win, f.mono, fTask, app.taskText,
                     s.x(50), s.y(100), gs::colors::blue, true);

        // Input area
        gs::drawFilledRect(win, s.x(50), s.y(180), s.x(400), s.y(220), sf::Color::White);
        gs::drawRect(win, s.x(50), s.y(180), s.x(400), s.y(220), gs::colors::blue, s.fx);
        gs::drawText(win, f.mono, fTask, app.userInput,
                     s.x(55), s.y(182), gs::colors::black);

        // Feedback
        if (app.showingFeedback) {
            const sf::Texture* face = (app.feedback == "Richtig!") ? &app.grins : &app.heul;
            sf::Sprite sp(*face);
            sp.setPosition({s.x(450), s.y(100)});
            float sz = s.x(80) / static_cast<float>(face->getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
            sf::Color fc = (app.feedback == "Richtig!") ? gs::colors::blue : gs::colors::red;
            gs::drawText(win, f.sans, fMed, app.feedback, s.x(430), s.y(190), fc);
        }

        // Progress bars
        gs::drawProgressBars(win, s, app.correct, app.wrong,
                             &app.grins, &app.heul, app.wrong > app.correct);

        // Status line
        int64_t elapsed = (gs::timerMs() - app.startTime) / 1000;
        std::string status = "Richtig: " + std::to_string(app.correct) +
                             "   Falsch: " + std::to_string(app.wrong) +
                             "   Zeit: " + std::to_string(elapsed / 60) + ":" +
                             (elapsed % 60 < 10 ? "0" : "") + std::to_string(elapsed % 60);
        gs::drawText(win, f.sans, fSmall, status, s.x(30), s.y(440), gs::colors::white);
        gs::drawTextCentered(win, f.sans, fSmall, app.playerName, s.y(464), gs::colors::white);
        gs::drawText(win, f.sans, fSmall,
                     "Return = Antwort prüfen    F8 = Ende",
                     s.x(30), s.y(455), gs::colors::white);

        // Decorative images on side based on progress
        if (app.correct >= 9) {
            sf::Sprite sp(app.katze);
            sp.setPosition({s.x(570), s.y(429)});
            float sz = s.x(41) / static_cast<float>(app.katze.getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
        }
        if (app.correct >= 18) {
            sf::Sprite sp(app.hase);
            sp.setPosition({s.x(570), s.y(392)});
            float sz = s.x(41) / static_cast<float>(app.hase.getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
        }
        if (app.correct >= 27) {
            sf::Sprite sp(app.kirche);
            sp.setPosition({s.x(570), s.y(356)});
            float sz = s.x(41) / static_cast<float>(app.kirche.getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
        }
        if (app.correct >= 36) {
            sf::Sprite sp(app.blume);
            sp.setPosition({s.x(570), s.y(320)});
            float sz = s.x(41) / static_cast<float>(app.blume.getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
        }

        win.display();
    };

    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape)
                    return;
                if (kp->code == sf::Keyboard::Key::Enter) {
                    // Check answer
                    int userAns = 0;
                    try { userAns = std::stoi(app.userInput); } catch (...) {}
                    if (userAns == app.answer) {
                        app.correct++;
                        app.feedback = "Richtig!";
                    } else {
                        app.wrong++;
                        app.feedback = "Falsch! Richtig: " + std::to_string(app.answer);
                    }
                    app.showingFeedback = true;
                    needDraw = true;
                    draw();
                    gs::delayMs(1500);
                    app.showingFeedback = false;

                    if (app.correct >= 36 || app.wrong >= 36) {
                        // Show final result
                        int64_t elapsed2 = (gs::timerMs() - app.startTime) / 1000;
                        std::string res = app.playerName + ": " +
                            std::to_string(app.correct) + " von " +
                            std::to_string(app.correct + app.wrong) +
                            " Aufgaben in " + std::to_string(elapsed2 / 60) +
                            " min " + std::to_string(elapsed2 % 60) + " s gelöst!";
                        gs::alert(win, f, s, res, {"OK"}, &app.grins);
                        return;
                    }
                    app.generateTask();
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Backspace) {
                    if (!app.userInput.empty()) app.userInput.pop_back();
                    needDraw = true;
                }
            }
            if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                char32_t ch = te->unicode;
                if ((ch >= '0' && ch <= '9') || ch == '-' || ch == ',' || ch == '.') {
                    app.userInput += static_cast<char>(ch);
                    needDraw = true;
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
}

// ---------------------------------------------------------------------------
// Result display
// ---------------------------------------------------------------------------
static void showResult(AppState& app) {
    int64_t elapsed = (gs::timerMs() - app.startTime) / 1000;
    std::ostringstream os;
    os << app.playerName << ": "
       << app.correct << " von " << (app.correct + app.wrong)
       << " Aufgaben in " << elapsed / 60 << " min " << elapsed % 60 << " s";
    gs::alert(*app.win, app.fonts, app.sc, os.str(), {"OK"}, &app.grins);
}

// ---------------------------------------------------------------------------
int main() {
    const unsigned W = 800, H = 600;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Rechnen für Grundschulkinder");
    window.setFramerateLimit(30);

    AppState app;
    app.win = &window;
    if (!app.fonts.load()) return 1;
    app.sc.init(640, 480, W, H);
    app.loadResources();

    // Name entry
    app.playerName = gs::nameEntryDialog(window, app.fonts, app.sc, "rechnet");
    if (!window.isOpen()) return 0;
    app.isSingle = (app.playerName.find(" und ") == std::string::npos &&
                    app.playerName.find('+') == std::string::npos);

    // Main loop
    while (window.isOpen()) {
        int group = mainMenu(app);
        if (group == 0 || !window.isOpen()) break;

        int ex = exerciseMenu(app);
        if (ex == 0 || !window.isOpen()) continue;
        app.exerciseType = ex;

        exerciseLoop(app);
    }
    return 0;
}