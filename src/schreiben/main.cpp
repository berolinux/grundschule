// Schreiben – Writing / spelling practice for elementary school
// Ported from GFA-Basic (Schreiben.g32)
//
// Provides spelling exercises organised in 6 groups:
//   Schreiben 1-4 (typing, dictation texts)
//   Wortschatz 1-2 (vocabulary by topic)

#include "common.h"
#include <fstream>
#include <sstream>

struct AppState {
    sf::RenderWindow* win = nullptr;
    gs::Fonts fonts;
    gs::ScaleFactors sc;
    std::string playerName;
    bool isSingle = true;
    bool helpOn = false;
    bool upperCase = false;  // "groß" mode
    int schreibGroup = 0;    // 1-6
    int exerciseType = 0;    // 1-13
    int correct = 0, wrong = 0;
    int64_t startTime = 0;

    sf::Texture grins, heul, katze, elefant, hase, blume, tastatur;

    void loadResources();
};

void AppState::loadResources() {
    gs::loadTexture(grins, "Bild/Grins.bmp");
    gs::loadTexture(heul, "Bild/Heul.bmp");
    gs::loadTexture(katze, "Bild/Katze.jpg");
    gs::loadTexture(elefant, "Bild/Elefant.jpg");
    gs::loadTexture(hase, "Bild/Hase.jpg");
    gs::loadTexture(blume, "Bild/Blume.jpg");
    gs::loadTexture(tastatur, "Bild/Tastatur3.bmp");
}

// Word lists for vocabulary exercises (Wortschatz)
static const std::vector<std::vector<std::string>> wortschatz1 = {
    {"arbeiten","Arbeit","Arbeiter","Arbeitgeber","Arbeitnehmer","bauen","Bauer",
     "Büro","Chef","Fabrik","fleißig","Geschäft","Handwerk","Handwerker"},
    {"billig","Einkauf","einkaufen","Geld","Geschäft","kaufen","Käufer","Kasse",
     "Kunde","Laden","Markt","Preis","teuer","verkaufen","Verkäufer"},
    {"Apfel","backen","Brot","Butter","Durst","essen","Fleisch","Gemüse",
     "Hunger","Kartoffel","kochen","Kuchen","Milch","Obst","trinken"},
    {"Bruder","Eltern","Familie","Frau","Geschwister","Großeltern","Kind",
     "Mann","Mutter","Schwester","Sohn","Tochter","Vater"},
    {"Ausflug","basteln","Ferien","Fest","frei","Freund","Geburtstag",
     "Hobby","Kino","lesen","malen","Spiel","spielen","Urlaub"},
};
static const std::vector<std::vector<std::string>> wortschatz2 = {
    {"Baum","Blatt","Blume","Garten","Gras","Pflanze","Rose","Samen","Wald","wachsen","Wiese","Wurzel"},
    {"Aufgabe","Bleistift","Buch","Heft","Klasse","Lehrer","lernen","lesen","Pause","Schule","Tafel","Unterricht"},
    {"Ball","Fahrrad","Fußball","laufen","rennen","schwimmen","Spiel","Sport","springen","Tor","turnen","Verein"},
    {"Buchstabe","Frage","lesen","Satz","schreiben","Sprache","sprechen","Wort","Wörterbuch","erzählen"},
    {"Affe","Fisch","Hase","Hund","Katze","Kuh","Pferd","Schwein","Tier","Vogel","Zoo"},
};

static std::vector<std::string> loadTextFile(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream f((gs::dataDir() / path).string());
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------
static int mainMenu(AppState& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;

    struct Btn { std::string label; sf::FloatRect bounds; int value; };
    std::vector<Btn> btns = {
        {"Schreiben  1", sf::FloatRect({s.x(50),  s.y(145)}, {s.x(260), s.y(70)}), 1},
        {"Schreiben  2", sf::FloatRect({s.x(355), s.y(145)}, {s.x(260), s.y(70)}), 2},
        {"Schreiben  3", sf::FloatRect({s.x(50),  s.y(225)}, {s.x(260), s.y(70)}), 3},
        {"Schreiben  4", sf::FloatRect({s.x(355), s.y(225)}, {s.x(260), s.y(70)}), 4},
        {"Wortschatz 1", sf::FloatRect({s.x(50),  s.y(305)}, {s.x(260), s.y(70)}), 5},
        {"Wortschatz 2", sf::FloatRect({s.x(355), s.y(305)}, {s.x(260), s.y(70)}), 6},
    };

    int selected = -1;
    unsigned fLg = static_cast<unsigned>(45 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    unsigned fMd = static_cast<unsigned>(21 * s.fx);

    auto draw = [&]() {
        win.clear();
        gs::drawFilledRect(win, 0, 0, (float)s.winW, (float)s.winH, gs::colors::veryDark);
        gs::drawFilledRect(win, s.x(4), s.y(126), s.x(632), s.y(364), gs::colors::creme);
        gs::drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), gs::colors::red, s.x(6));

        std::string title = app.isSingle
            ? "Hier kannst du auswählen:" : "Hier könnt ihr auswählen:";
        gs::drawText(win, f.sans, fSm, title, s.x(60), s.y(20), gs::colors::white);
        gs::drawTextCentered(win, f.sans, fSm, app.playerName, s.y(464), gs::colors::white);

        // Help / mode buttons
        gs::drawRoundedRect(win, s.x(59), s.y(50), s.x(301), s.y(75),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "Hilfe: ja", s.x(110), s.y(52),
                     app.helpOn ? gs::colors::creme : gs::colors::blue);
        gs::drawRoundedRect(win, s.x(335), s.y(50), s.x(579), s.y(75),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "Hilfe: nein", s.x(381), s.y(52),
                     app.helpOn ? gs::colors::blue : gs::colors::creme);

        gs::drawRoundedRect(win, s.x(59), s.y(85), s.x(301), s.y(110),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "normal", s.x(110), s.y(87),
                     !app.upperCase ? gs::colors::creme : gs::colors::blue);
        gs::drawRoundedRect(win, s.x(335), s.y(85), s.x(579), s.y(110),
                            gs::colors::red, s.x(3), gs::colors::creme);
        gs::drawText(win, f.sans, fMd, "groß", s.x(381), s.y(87),
                     app.upperCase ? gs::colors::creme : gs::colors::blue);

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
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return 0;
                if (kp->code == sf::Keyboard::Key::F1) { app.helpOn = !app.helpOn; draw(); }
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                if (mp->button == sf::Mouse::Button::Left) {
                    if (my > s.y(50) && my < s.y(75)) {
                        if (mx < s.x(301)) app.helpOn = true;
                        else app.helpOn = false;
                        draw();
                    }
                    if (my > s.y(85) && my < s.y(110)) {
                        if (mx < s.x(301)) app.upperCase = false;
                        else app.upperCase = true;
                        draw();
                    }
                    for (auto& b : btns) {
                        if (mx >= b.bounds.position.x &&
                            mx <= b.bounds.position.x + b.bounds.size.x &&
                            my >= b.bounds.position.y &&
                            my <= b.bounds.position.y + b.bounds.size.y) {
                            selected = b.value; draw();
                        }
                    }
                } else if (mp->button == sf::Mouse::Button::Right && selected > 0) {
                    app.schreibGroup = selected;
                    return selected;
                }
            }
        }
        gs::delayMs(16);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Exercise sub-menu
// ---------------------------------------------------------------------------
static std::vector<std::string> getExerciseNames(int group) {
    switch (group) {
        case 1: return {"(Tastatur)", "Tasten finden", "Zahlen", "kleine Buchstaben",
                        "große Buchstaben", "alle Buchstaben", "übrige Zeichen",
                        "alle Zeichen", "Merkwörter 1 A-Z", "Merkwörter 2 A-Z",
                        "Merkwörter 3 A-Z", "Merkwörter 4 A-Z", "Text 1", "Text 2"};
        case 2: return {"(Rechtschreibung)", "Farben", "a und ä", "au und äu",
                        "d oder t am Ende", "b, g oder k am Ende",
                        "Text 1","Text 2","Text 3","Text 4","Text 5","Text 6","Text 7","Text 8"};
        case 3: return {"(Wortarten)", "groß oder klein", "froh oder traurig",
                        "schnell/langsam, laut/leise", "gut oder schlecht", "wann?",
                        "Text 1","Text 2","Text 3","Text 4","Text 5","Text 6","Text 7","Text 8"};
        case 4: return {"(Wortfelder)", "Zahlen", "tun, machen",
                        "sehen, hören, merken", "sagen", "sich bewegen",
                        "Text 1","Text 2","Text 3","Text 4","Text 5","Text 6","Text 7","Text 8"};
        case 5: return {"(Wortschatz 1)", "Arbeit", "Einkaufen", "Essen und Trinken",
                        "Die Familie", "Freizeit", "Gefühle", "Gesundheit und Krankheit",
                        "Das Jahr", "Kleidung", "Der Körper", "Malen und Basteln",
                        "Mengen und Richtungen", "In der Pause"};
        case 6: return {"(Wortschatz 2)", "Pflanzen", "In der Schule", "Sport und Spiel",
                        "Die Sprache", "Tiere", "Unglück", "Verkehr", "Verreisen",
                        "Das Wetter", "Wohnen", "Die Zeit",
                        "Schwierige Zeitwörter 1", "Schwierige Zeitwörter 2"};
        default: return {"(Unbekannt)"};
    }
}

// ---------------------------------------------------------------------------
// Writing exercise loop
// ---------------------------------------------------------------------------
static void exerciseLoop(AppState& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    app.correct = 0; app.wrong = 0;
    app.startTime = gs::timerMs();

    // Get words for this exercise
    std::vector<std::string> words;
    if (app.schreibGroup == 5 && app.exerciseType >= 2 && app.exerciseType <= 6) {
        int idx = app.exerciseType - 2;
        if (idx < (int)wortschatz1.size()) words = wortschatz1[idx];
    } else if (app.schreibGroup == 6 && app.exerciseType >= 2 && app.exerciseType <= 6) {
        int idx = app.exerciseType - 2;
        if (idx < (int)wortschatz2.size()) words = wortschatz2[idx];
    } else if (app.schreibGroup <= 4 && app.exerciseType >= 6) {
        // Load from text file
        int textNum = app.exerciseType - 5;
        std::string fname = "Texte/Text" + std::to_string(app.schreibGroup) +
                            "_" + std::to_string(textNum + 5) + ".txt";
        auto lines = loadTextFile(fname);
        for (auto& l : lines) {
            std::istringstream iss(l);
            std::string w;
            while (iss >> w) words.push_back(w);
        }
    }

    // If no words found, generate simple typing exercise
    if (words.empty()) {
        if (app.exerciseType == 1) {
            // Key finding exercise
            words = {"a","s","d","f","g","h","j","k","l","q","w","e","r","t",
                     "z","u","i","o","p","y","x","c","v","b","n","m"};
        } else if (app.exerciseType == 2) {
            words = {"1","2","3","4","5","6","7","8","9","0"};
        } else {
            words = {"Mama","Papa","Oma","Opa","Hund","Katze","Haus","Baum",
                     "Sonne","Mond","Stern","Blume","Schule","Kind","Buch"};
        }
    }

    // Shuffle
    for (int i = (int)words.size() - 1; i > 0; i--) {
        int j = gs::randInt(0, i);
        std::swap(words[i], words[j]);
    }

    int wordIdx = 0;
    std::string currentWord = words[0];
    std::string userInput;
    std::string feedback;
    bool showFeedback = false;

    unsigned fBig = static_cast<unsigned>(25 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);

    auto draw = [&]() {
        win.clear();
        // Keyboard image
        if (app.tastatur.getSize().x > 0) {
            sf::Sprite sp(app.tastatur);
            sp.setPosition({s.x(43), s.y(27)});
            float scx = s.x(550) / static_cast<float>(app.tastatur.getSize().x);
            float scy = s.y(190) / static_cast<float>(app.tastatur.getSize().y);
            sp.setScale({scx, scy});
            win.draw(sp);
        }

        // Writing area
        gs::drawFilledRect(win, s.x(3), s.y(228), s.x(633), s.y(475), gs::colors::creme);
        gs::drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), gs::colors::red, s.x(6));
        gs::drawLine(win, s.x(3), s.y(227), s.x(633), s.y(227), gs::colors::blue, s.x(2));

        // Word to type
        if (app.helpOn || userInput.empty()) {
            std::string display = app.upperCase ? currentWord : currentWord;
            // Convert to upper if needed (simplified)
            if (app.upperCase) {
                for (auto& c : display) c = std::toupper(static_cast<unsigned char>(c));
            }
            gs::drawText(win, f.sans, fBig, "Schreibe: " + display,
                         s.x(30), s.y(240), gs::colors::blue);
        }

        // User input
        gs::drawText(win, f.sans, fBig, userInput,
                     s.x(30), s.y(280), gs::colors::black);
        // Cursor
        float cx = s.x(30) + gs::textWidth(f.sans, fBig, userInput);
        gs::drawRect(win, cx, s.y(282), cx + s.x(14), s.y(305), gs::colors::red, 1.f);

        // Feedback
        if (showFeedback) {
            const sf::Texture* face = (feedback == "Richtig!") ? &app.grins : &app.heul;
            sf::Sprite sp(*face);
            sp.setPosition({s.x(500), s.y(350)});
            float sz = s.x(60) / static_cast<float>(face->getSize().x);
            sp.setScale({sz, sz});
            win.draw(sp);
            sf::Color fc = (feedback == "Richtig!") ? gs::colors::blue : gs::colors::red;
            gs::drawText(win, f.sans, fBig, feedback, s.x(30), s.y(340), fc);
        }

        // Progress
        gs::drawProgressBars(win, s, app.correct, app.wrong,
                             &app.grins, &app.heul, showFeedback && feedback != "Richtig!");

        // Status
        gs::drawText(win, f.sans, fSm,
                     "Richtig: " + std::to_string(app.correct) +
                     "   Falsch: " + std::to_string(app.wrong),
                     s.x(30), s.y(440), gs::colors::white);
        gs::drawTextCentered(win, f.sans, fSm, app.playerName, s.y(464), gs::colors::white);

        win.display();
    };

    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return;
                if (kp->code == sf::Keyboard::Key::F1) {
                    app.helpOn = !app.helpOn; needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Backspace) {
                    if (!userInput.empty()) userInput.pop_back();
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Enter) {
                    // Trim trailing spaces
                    while (!userInput.empty() && userInput.back() == ' ')
                        userInput.pop_back();
                    // Check
                    if (userInput == currentWord) {
                        app.correct++;
                        feedback = "Richtig!";
                    } else {
                        app.wrong++;
                        feedback = "Falsch! Richtig: " + currentWord;
                    }
                    showFeedback = true;
                    needDraw = true;
                    draw();
                    gs::delayMs(1500);
                    showFeedback = false;
                    userInput.clear();
                    wordIdx++;
                    if (wordIdx >= (int)words.size() || app.correct >= 36 || app.wrong >= 36) {
                        int64_t elapsed = (gs::timerMs() - app.startTime) / 1000;
                        std::string res = app.playerName + ": " +
                            std::to_string(app.correct) + " von " +
                            std::to_string(app.correct + app.wrong) +
                            " richtig in " + std::to_string(elapsed / 60) +
                            " min " + std::to_string(elapsed % 60) + " s";
                        gs::alert(win, f, s, res, {"OK"}, &app.grins);
                        return;
                    }
                    currentWord = words[wordIdx];
                    needDraw = true;
                }
            }
            if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                char32_t ch = te->unicode;
                if (ch >= 32 && ch != 127) {
                    userInput += static_cast<char>(ch);
                    needDraw = true;
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
}

// ---------------------------------------------------------------------------
int main() {
    const unsigned W = 800, H = 600;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Schreiben für Grundschulkinder");
    window.setFramerateLimit(30);

    AppState app;
    app.win = &window;
    if (!app.fonts.load()) return 1;
    app.sc.init(640, 480, W, H);
    app.loadResources();

    app.playerName = gs::nameEntryDialog(window, app.fonts, app.sc, "schreibt");
    if (!window.isOpen()) return 0;
    app.isSingle = (app.playerName.find(" und ") == std::string::npos &&
                    app.playerName.find('+') == std::string::npos);

    while (window.isOpen()) {
        int group = mainMenu(app);
        if (group == 0 || !window.isOpen()) break;

        auto names = getExerciseNames(app.schreibGroup);
        int ex = gs::gridMenu(window, app.fonts, app.sc,
                              names[0], names, app.playerName);
        if (ex == 0 || !window.isOpen()) continue;
        app.exerciseType = ex;

        exerciseLoop(app);
    }
    return 0;
}