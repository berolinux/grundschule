// Lesen – Reading exercises for elementary school
// Ported from GFA-Basic (Lesen.g32)
//
// An interactive reading program with 81 picture/word pairs and
// 6 difficulty levels.  13 exercise types in four groups:
//   1-2  : Drag words to matching pictures (+ memory variant)
//   3-5  : Drag letter-sounds to matching pictures (+ memory variants)
//   6-9  : Click the correct letter-sound (beginning, middle, end, all)
//   10-13: Type the correct letter-sound   (beginning, middle, end, all)
//
// The child learns to match images with German words and practises
// recognising individual letters and multi-character sounds (Ei, Sch, …).

#include "common.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <numeric>

// ─── Data ──────────────────────────────────────────────────────────────────
// 81 picture/word pairs and 38 letter-sound entries copied from the
// original "fibel" and "tastaturmalen" procedures in Lesen.g32.

struct FibelEntry {
    const char* imagePath;   // e.g. "Bild/ABCD/mama.jpg"
    const char* word;        // e.g. "Mama"
    const char* letterUpper; // initial letter-sound uppercase, e.g. "M"
};

static const FibelEntry kFibel[] = {
    {"Bild/ABCD/mama.jpg",      "Mama",     "M"},   //  0
    {"Bild/ABCD/oma.jpg",       "Oma",      "O"},   //  1
    {"Bild/ABCD/ampel.jpg",     "Ampel",    "A"},   //  2
    {"Bild/ABCD/sonne.jpg",     "Sonne",    "S"},   //  3
    {"Bild/ABCD/lampe.jpg",     "Lampe",    "L"},   //  4
    {"Bild/ABCD/tomate.jpg",    "Tomate",   "T"},   //  5
    {"Bild/ABCD/nest.jpg",      "Nest",     "N"},   //  6
    {"Bild/ABCD/igel.jpg",      "Igel",     "I"},   //  7
    {"Bild/ABCD/esel.jpg",      "Esel",     "E"},   //  8
    {"Bild/ABCD/rose.jpg",      "Rose",     "R"},   //  9
    {"Bild/ABCD/pinsel.jpg",    "Pinsel",   "P"},   // 10
    {"Bild/ABCD/fenster.jpg",   "Fenster",  "F"},   // 11
    {"Bild/ABCD/kamel.jpg",     "Kamel",    "K"},   // 12
    {"Bild/ABCD/hamster.jpg",   "Hamster",  "H"},   // 13
    {"Bild/ABCD/uhr.jpg",       "Uhr",      "U"},   // 14
    {"Bild/ABCD/auto.jpg",      "Auto",     "Au"},  // 15
    {"Bild/ABCD/eis.jpg",       "Eis",      "Ei"},  // 16
    {"Bild/ABCD/geige.jpg",     "Geige",    "G"},   // 17
    {"Bild/ABCD/ball.jpg",      "Ball",     "B"},   // 18
    {"Bild/ABCD/biene.jpg",     "Biene",    "B"},   // 19
    {"Bild/ABCD/buch.jpg",      "Buch",     "B"},   // 20
    {"Bild/ABCD/dach.jpg",      "Dach",     "D"},   // 21
    {"Bild/ABCD/zebra.jpg",     "Zebra",    "Z"},   // 22
    {"Bild/ABCD/schere.jpg",    "Schere",   "Sch"}, // 23
    {"Bild/ABCD/wolke.jpg",     "Wolke",    "W"},   // 24
    {"Bild/ABCD/vogel.jpg",     "Vogel",    "V"},   // 25
    {"Bild/ABCD/zange.jpg",     "Zange",    "Z"},   // 26
    {"Bild/ABCD/junge.jpg",     "Junge",    "J"},   // 27
    {"Bild/ABCD/hexe.jpg",      "Hexe",     "X"},   // 28  (H → but alph uses X)
    {"Bild/ABCD/kaese.jpg",     u8"Käse",   u8"Ä"}, // 29
    {"Bild/ABCD/loewe.jpg",     u8"Löwe",   u8"Ö"}, // 30
    {"Bild/ABCD/kueken.jpg",    u8"Küken",  u8"Ü"}, // 31
    {"Bild/ABCD/haeuser.jpg",   u8"Häuser", u8"äu"},// 32
    {"Bild/ABCD/strasse.jpg",   u8"Straße", u8"ß"}, // 33
    {"Bild/ABCD/Feuer.jpg",     "Feuer",    "Eu"},  // 34
    {"Bild/ABCD/qualm.jpg",     "Qualm",    "Qu"},  // 35
    {"Bild/ABCD/baby.jpg",      "Baby",     "B"},   // 36
    {"Bild/ABCD/computer.jpg",  "Computer", "C"},   // 37
    {"Bild/ABCD/salami.jpg",    "Salami",   "S"},   // 38
    {"Bild/ABCD/tasse.jpg",     "Tasse",    "T"},   // 39
    {"Bild/ABCD/affe.jpg",      "Affe",     "A"},   // 40
    {"Bild/ABCD/apfel.jpg",     "Apfel",    "A"},   // 41
    {"Bild/ABCD/elefant.jpg",   "Elefant",  "E"},   // 42
    {"Bild/ABCD/familie.jpg",   "Familie",  "F"},   // 43
    {"Bild/ABCD/tafel.jpg",     "Tafel",    "T"},   // 44
    {"Bild/ABCD/telefon.jpg",   "Telefon",  "T"},   // 45
    {"Bild/ABCD/rakete.jpg",    "Rakete",   "R"},   // 46
    {"Bild/ABCD/hammer.jpg",    "Hammer",   "H"},   // 47
    {"Bild/ABCD/hase.jpg",      "Hase",     "H"},   // 48
    {"Bild/ABCD/nashorn.jpg",   "Nashorn",  "N"},   // 49
    {"Bild/ABCD/ohr.jpg",       "Ohr",      "O"},   // 50
    {"Bild/ABCD/kaktus.jpg",    "Kaktus",   "K"},   // 51
    {"Bild/ABCD/kuh.jpg",       "Kuh",      "K"},   // 52
    {"Bild/ABCD/uhu.jpg",       "Uhu",      "U"},   // 53
    {"Bild/ABCD/haus.jpg",      "Haus",     "H"},   // 54
    {"Bild/ABCD/maus.jpg",      "Maus",     "M"},   // 55
    {"Bild/ABCD/leiter.jpg",    "Leiter",   "L"},   // 56
    {"Bild/ABCD/gans.jpg",      "Gans",     "G"},   // 57
    {"Bild/ABCD/giraffe.jpg",   "Giraffe",  "G"},   // 58
    {"Bild/ABCD/regen.jpg",     "Regen",    "R"},   // 59
    {"Bild/ABCD/tiger.jpg",     "Tiger",    "T"},   // 60
    {"Bild/ABCD/bananen.jpg",   "Bananen",  "B"},   // 61
    {"Bild/ABCD/baum.jpg",      "Baum",     "B"},   // 62
    {"Bild/ABCD/boot.jpg",      "Boot",     "B"},   // 63
    {"Bild/ABCD/gabel.jpg",     "Gabel",    "G"},   // 64
    {"Bild/ABCD/tiere.jpg",     "Tiere",    "T"},   // 65
    {"Bild/ABCD/kuchen.jpg",    "Kuchen",   "K"},   // 66
    {"Bild/ABCD/daumen.jpg",    "Daumen",   "D"},   // 67
    {"Bild/ABCD/hund.jpg",      "Hund",     "H"},   // 68
    {"Bild/ABCD/indianer.jpg",  "Indianer", "I"},   // 69
    {"Bild/ABCD/mond.jpg",      "Mond",     "M"},   // 70
    {"Bild/ABCD/radio.jpg",     "Radio",    "R"},   // 71
    {"Bild/ABCD/pilz.jpg",      "Pilz",     "P"},   // 72
    {"Bild/ABCD/zahn.jpg",      "Zahn",     "Z"},   // 73
    {"Bild/ABCD/zitrone.jpg",   "Zitrone",  "Z"},   // 74
    {"Bild/ABCD/fisch.jpg",     "Fisch",    "F"},   // 75
    {"Bild/ABCD/frosch.jpg",    "Frosch",   "F"},   // 76
    {"Bild/ABCD/schloss.jpg",   "Schloss",  "Sch"}, // 77
    {"Bild/ABCD/wolf.jpg",      "Wolf",     "W"},   // 78
    {"Bild/ABCD/wuerfel.jpg",   u8"Würfel", "W"},   // 79
    {"Bild/ABCD/teddy.jpg",     "Teddy",    "T"},   // 80
};
static constexpr int kFibelCount = 81;

// Letter-sound buttons for the on-screen keyboard (from tastaturmalen).
// Each entry is a pair/triplet displayed on its button.
static const char* kLetterSounds[] = {
    "Mm",  "Oo",  "Aa",  "Ss",  "Ll",  "Tt",  "Nn",  "Ii",      //  0- 7
    "Ee",  "Rr",  "Pp",  "Ff",  "Kk",  "Hh",  "Uu",             //  8-14
    "Au au","Ei ei","Gg", "Bb",  "ie",  "ch",  "Dd",              // 15-21
    "Zz",  "Sch sch","Ww","Vv",  "ng",  "Jj",  "Xx",             // 22-28
    u8"Ää",u8"Öö", u8"Üü",u8"äu","ß", "Eu eu","Qu qu",          // 29-35
    "Yy",  "Cc"                                                    // 36-37
};
static constexpr int kSoundCount = 38;

// Level determines how many "Merkwörter" (core words) are in play:
//   Level 1: 8, 2: 12, 3: 16, 4: 20, 5: 24, 6: 38 (all)
static int coreWordCount(int level) {
    switch (level) {
        case 1: return 8;  case 2: return 12; case 3: return 16;
        case 4: return 20; case 5: return 24; default: return 38;
    }
}

// With supplementary words (for "alle Wörter" mode):
static int totalWordCount(int level) {
    switch (level) {
        case 1: return 8;  case 2: return 20; case 3: return 34;
        case 4: return 48; case 5: return 64; default: return 81;
    }
}

// ─── Application state ─────────────────────────────────────────────────────
struct App {
    sf::RenderWindow* win = nullptr;
    gs::Fonts fonts;
    gs::ScaleFactors sc;
    std::string playerName;
    bool isSingle = true;

    // Textures
    sf::Texture fibelTex[kFibelCount];
    sf::Texture grins, heul, katze, hase, kirche, blume;

    // Menu selections
    int level = 1;       // 1-6 (reading level / word range)
    int exerciseType = 1;// 1-13
    bool allWords = true;// true = alle Wörter, false = nur Merkwörter
    bool helpOn = true;

    // Exercise tracking
    float correctF = 0;  // fractional (level 1 gives 36/20 per correct)
    int correct1 = 0;    // integer count for display
    int wrong = 0;
    bool lastError = false;
    int64_t startTime = 0;

    void loadResources();
    int wordPoolSize() const { return allWords ? totalWordCount(level) : coreWordCount(level); }
    float correctInc() const { return (level == 1) ? 36.f / 20.f : 1.f; }
};

void App::loadResources() {
    for (int i = 0; i < kFibelCount; i++)
        gs::loadTexture(fibelTex[i], kFibel[i].imagePath);
    gs::loadTexture(grins,  "Bild/Grins.bmp");
    gs::loadTexture(heul,   "Bild/Heul.bmp");
    gs::loadTexture(katze,  "Bild/Katze.jpg");
    gs::loadTexture(hase,   "Bild/Hase.jpg");
    gs::loadTexture(kirche, "Bild/Kirche.jpg");
    gs::loadTexture(blume,  "Bild/Blume.jpg");
}

// ─── Random helpers ─────────────────────────────────────────────────────────
// Pick n distinct random indices from [0, poolSize).
static std::vector<int> pickDistinct(int poolSize, int n) {
    std::vector<int> pool(poolSize);
    std::iota(pool.begin(), pool.end(), 0);
    for (int i = 0; i < n && i < poolSize; i++) {
        int j = i + gs::randInt(0, poolSize - 1 - i);
        std::swap(pool[i], pool[j]);
    }
    pool.resize(std::min(n, poolSize));
    return pool;
}

// Map indices from "Merkwörter only" pool to actual fibel indices.
// The original code remaps indices > threshold by adding an offset,
// which interleaves core words with supplementary words.  We simplify
// by using indices directly: core words are 0..coreCount-1 in the fibel.
static int mapIndex(int idx, int level, bool allWords) {
    if (allWords) {
        // For "alle Wörter" mode, the original code remaps supplementary
        // words (index > coreCount-1) by adding an offset so that they
        // map into the 38..80 range.  We just clamp to kFibelCount.
        int core = coreWordCount(level);
        if (idx >= core) {
            // Map supplementary index: offset depends on level
            int offset = 38 - core;
            return std::min(idx + offset, kFibelCount - 1);
        }
    }
    return std::min(idx, kFibelCount - 1);
}

// ─── Draw helpers ───────────────────────────────────────────────────────────
static void drawBackground(sf::RenderWindow& win, const gs::ScaleFactors& s) {
    gs::drawFilledRect(win, 0, 0, (float)s.winW, (float)s.winH, gs::colors::veryDark);
    gs::drawFilledRect(win, s.x(3), s.y(3), s.x(633), s.y(475), gs::colors::creme);
    // Upper half: dark brown
    gs::drawFilledRect(win, s.x(3), s.y(3), s.x(633), s.y(238), gs::colors::darkBrown);
    // Separator
    gs::drawLine(win, s.x(6), s.y(238), s.x(632), s.y(238), gs::colors::red, s.x(3));
    // Frame
    gs::drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), gs::colors::red, s.x(6));
}

static void drawPicture(sf::RenderWindow& win, const sf::Texture& tex,
                        float x, float y, float w, float h) {
    sf::Sprite sp(tex);
    sp.setPosition({x, y});
    sp.setScale({w / (float)tex.getSize().x, h / (float)tex.getSize().y});
    win.draw(sp);
}

static void drawProgressColumn(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    // Correct bar (blue)
    int bars = static_cast<int>(app.correctF);
    if (bars > 0 && bars <= 36)
        gs::drawFilledRect(win, s.x(570), s.y(460) - bars * s.y(4),
                           s.x(611), s.y(467), gs::colors::blue);
    // Wrong bar (grey)
    if (app.wrong > 0 && app.wrong <= 36)
        gs::drawFilledRect(win, s.x(512), s.y(467),
                           s.x(552), s.y(461) - app.wrong * s.y(4),
                           gs::colors::grey);
    // Smiley
    const sf::Texture& face = app.lastError ? app.heul : app.grins;
    drawPicture(win, face, s.x(536), s.y(256), s.x(50), s.y(56));

    // Milestone images
    if (bars >= 9)  drawPicture(win, app.katze,  s.x(570), s.y(429), s.x(41), s.y(33));
    if (bars >= 18) drawPicture(win, app.hase,   s.x(570), s.y(392), s.x(41), s.y(33));
    if (bars >= 27) drawPicture(win, app.kirche, s.x(570), s.y(356), s.x(41), s.y(33));
    if (bars >= 36) drawPicture(win, app.blume,  s.x(570), s.y(320), s.x(41), s.y(33));

    // Player name
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    gs::drawTextCentered(win, app.fonts.sans, fSm, app.playerName,
                         s.y(464), gs::colors::white);
}

// ─── Main menu (level selection) ────────────────────────────────────────────
static bool mainMenu(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    unsigned fMd = static_cast<unsigned>(21 * s.fx);
    unsigned fLg = static_cast<unsigned>(39 * s.fx);

    int selected = app.level;
    int wordMode = app.allWords ? 2 : 1; // 1=nur Merkwörter, 2=alle

    struct Cell { const char* label; int row; int col; int val; };
    Cell cells[] = {
        {u8"Mama → Igel",     0, 0, 1},
        {u8"Mama → Fenster",  0, 1, 2},
        {u8"Mama → Auto",     1, 0, 3},
        {u8"Mama → Biene",    1, 1, 4},
        {u8"Mama → Schere",   2, 0, 5},
        {u8"Mama → Computer", 2, 1, 6},
    };

    auto draw = [&]() {
        win.clear();
        gs::drawFilledRect(win, 0, 0, (float)s.winW, (float)s.winH, gs::colors::veryDark);
        gs::drawFilledRect(win, s.x(3), s.y(3), s.x(633), s.y(475), gs::colors::darkBrown);
        gs::drawFilledRect(win, s.x(4), s.y(96), s.x(632), s.y(364), gs::colors::creme);
        gs::drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), gs::colors::red, s.x(6));

        // Word mode buttons
        gs::drawRoundedRect(win, s.x(60), s.y(55), s.x(270), s.y(80),
                            gs::colors::red, s.x(3),
                            wordMode == 2 ? gs::colors::blue : gs::colors::creme);
        gs::drawText(win, f.sans, fMd, u8"alle Wörter", s.x(119), s.y(58),
                     wordMode == 2 ? gs::colors::creme : gs::colors::blue);

        gs::drawRoundedRect(win, s.x(366), s.y(55), s.x(576), s.y(80),
                            gs::colors::red, s.x(3),
                            wordMode == 1 ? gs::colors::blue : gs::colors::creme);
        gs::drawText(win, f.sans, fMd, u8"nur Merkwörter", s.x(415), s.y(58),
                     wordMode == 1 ? gs::colors::creme : gs::colors::blue);

        // Title
        std::string title = app.isSingle
            ? u8"Hier kannst du auswählen:" : u8"Hier könnt ihr auswählen:";
        gs::drawText(win, f.sans, fSm, title, s.x(40), s.y(30), gs::colors::white);

        // Grid lines
        gs::drawLine(win, s.x(4), s.y(95),  s.x(632), s.y(95),  gs::colors::red, s.x(3));
        gs::drawLine(win, s.x(4), s.y(185), s.x(632), s.y(185), gs::colors::red, s.x(3));
        gs::drawLine(win, s.x(4), s.y(275), s.x(632), s.y(275), gs::colors::red, s.x(3));
        gs::drawLine(win, s.x(4), s.y(365), s.x(632), s.y(365), gs::colors::red, s.x(3));
        gs::drawLine(win, s.x(318), s.y(95), s.x(318), s.y(365), gs::colors::red, s.x(3));

        // Cells
        for (auto& c : cells) {
            float cx = (c.col == 0) ? s.x(25) : s.x(330);
            float cy = s.y(125) + c.row * s.y(90);
            bool sel = (c.val == selected);
            if (sel) {
                float bx0 = (c.col == 0) ? s.x(12) : s.x(320);
                float bx1 = (c.col == 0) ? s.x(316) : s.x(626);
                float by0 = s.y(97) + c.row * s.y(90);
                float by1 = by0 + s.y(86);
                gs::drawFilledRect(win, bx0, by0, bx1, by1, gs::colors::blue);
            }
            sf::Color fg = sel ? gs::colors::creme : gs::colors::blue;
            gs::drawText(win, f.sans, fLg, c.label, cx, cy, fg);
        }

        // Instructions
        gs::drawText(win, f.sans, fSm,
            "linke Maustaste: Auswahl    rechte Maustaste: weiter    F8: Ende",
            s.x(40), s.y(380), gs::colors::white);
        gs::drawTextCentered(win, f.sans, fSm, app.playerName,
                             s.y(464), gs::colors::white);
        win.display();
    };
    draw();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape)
                    return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = (float)mp->position.x, my = (float)mp->position.y;
                if (mp->button == sf::Mouse::Button::Left) {
                    // Word mode
                    if (my > s.y(55) && my < s.y(80)) {
                        if (mx > s.x(60)  && mx < s.x(270)) { wordMode = 2; draw(); }
                        if (mx > s.x(366) && mx < s.x(576)) { wordMode = 1; draw(); }
                    }
                    // Cells
                    for (auto& c : cells) {
                        float bx0 = (c.col == 0) ? s.x(4) : s.x(318);
                        float bx1 = (c.col == 0) ? s.x(318) : s.x(632);
                        float by0 = s.y(95) + c.row * s.y(90);
                        float by1 = by0 + s.y(90);
                        if (mx >= bx0 && mx <= bx1 && my >= by0 && my <= by1) {
                            selected = c.val;
                            draw();
                        }
                    }
                } else if (mp->button == sf::Mouse::Button::Right) {
                    app.level = selected;
                    app.allWords = (wordMode == 2);
                    return true;
                }
            }
        }
        gs::delayMs(16);
    }
    return false;
}

// ─── Exercise sub-menu ──────────────────────────────────────────────────────
static bool exerciseMenu(App& app) {
    std::vector<std::string> items = {
        u8"(Übungen)",
        u8"Wörter schieben",
        "Memory 1",
        "Merkbuchstaben schieben",
        "Memory 2 (rubbeln)",
        "Memory 3",
        "Buchstaben suchen: Anfang",
        "Buchstaben suchen: Mitte",
        "Buchstaben suchen: Ende",
        "alle Buchstaben suchen",
        "Buchstaben schreiben: Anfang",
        "Buchstaben schreiben: Mitte",
        "Buchstaben schreiben: Ende",
        "alle Buchstaben schreiben",
    };
    int sel = gs::gridMenu(*app.win, app.fonts, app.sc,
                           "Lesen " + std::to_string(app.level), items,
                           app.playerName);
    if (sel > 0) app.exerciseType = sel;
    return sel > 0;
}

// ─── Exercise: word matching (art 1-2) ──────────────────────────────────────
// Show 4 pictures and 5 word labels.  The child clicks a word, then clicks
// the matching picture.  In Memory mode (art==2) the pictures are hidden
// initially and revealed only when the word is dropped on them.

static bool exerciseWords(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMd = static_cast<unsigned>(25 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    bool memoryMode = (app.exerciseType == 2);

    while (win.isOpen() && app.correctF < 36 && app.wrong < 36) {
        // Pick 4 pictures + 1 extra word
        auto indices = pickDistinct(app.wordPoolSize(), 5);
        for (auto& idx : indices) idx = mapIndex(idx, app.level, app.allWords);

        // The 4 pictures are indices[0..3], all 5 words are shuffled
        std::vector<int> wordOrder = {0, 1, 2, 3, 4};
        for (int i = 4; i > 0; i--) std::swap(wordOrder[i], wordOrder[gs::randInt(0, i)]);

        bool picRevealed[4] = {!memoryMode, !memoryMode, !memoryMode, !memoryMode};
        bool slotDone[4] = {false, false, false, false};
        int selectedWord = -1;

        // Picture positions
        float picX[4], picY = s.y(30), picW = s.x(118), picH = s.y(157);
        picX[0] = s.x(40);  picX[1] = s.x(187);
        picX[2] = s.x(334); picX[3] = s.x(481);

        // Word slot positions (below pictures)
        float slotY = s.y(200);
        float slotCX[4] = {s.x(99), s.x(246), s.x(393), s.x(540)};

        // Word label positions (stacked on the left)
        float labelX[5], labelY[5];
        for (int i = 0; i < 5; i++) {
            labelX[i] = s.x(37 + i * 50);
            labelY[i] = s.y(250 + i * 40);
        }

        auto draw = [&]() {
            win.clear();
            drawBackground(win, s);

            // Pictures
            for (int i = 0; i < 4; i++) {
                if (picRevealed[i] || slotDone[i])
                    drawPicture(win, app.fibelTex[indices[i]],
                                picX[i], picY, picW, picH);
                else
                    gs::drawFilledRect(win, picX[i], picY,
                                       picX[i] + picW, picY + picH,
                                       gs::colors::darkBrown);
                // Slot box
                gs::drawRect(win, picX[i] - s.x(3), slotY - s.y(3),
                             picX[i] + picW + s.x(3), slotY + s.y(27),
                             gs::colors::red, s.x(3));
                gs::drawFilledRect(win, picX[i], slotY,
                                   picX[i] + picW, slotY + s.y(25),
                                   sf::Color::White);
                // Show matched word
                if (slotDone[i])
                    gs::drawText(win, f.sans, fMd, kFibel[indices[i]].word,
                                 slotCX[i] - gs::textWidth(f.sans, fMd, kFibel[indices[i]].word) / 2.f,
                                 slotY, gs::colors::black);
            }

            // Word labels
            for (int i = 0; i < 5; i++) {
                int wi = wordOrder[i];
                // Don't show words that are already matched
                bool matched = false;
                for (int j = 0; j < 4; j++)
                    if (slotDone[j] && indices[j] == indices[wi] && wi == j) matched = true;

                sf::Color bg = (i == selectedWord) ? gs::colors::blue : sf::Color::White;
                sf::Color fg = (i == selectedWord) ? gs::colors::creme : gs::colors::black;
                gs::drawFilledRect(win, labelX[i], labelY[i],
                                   labelX[i] + s.x(120), labelY[i] + s.y(30), bg);
                gs::drawRect(win, labelX[i], labelY[i],
                             labelX[i] + s.x(120), labelY[i] + s.y(30),
                             gs::colors::red, s.x(3));
                gs::drawText(win, f.sans, fMd, kFibel[indices[wi]].word,
                             labelX[i] + s.x(5), labelY[i] + s.y(2), fg);
            }

            // Help: show answer words in red above pictures
            if (app.helpOn) {
                for (int i = 0; i < 4; i++) {
                    if (!slotDone[i])
                        gs::drawText(win, f.sans, fSm, kFibel[indices[i]].word,
                                     slotCX[i] - gs::textWidth(f.sans, fSm, kFibel[indices[i]].word) / 2.f,
                                     slotY + s.y(1), gs::colors::red);
                }
            }

            drawProgressColumn(app);
            gs::drawText(win, f.sans, fSm, "F1: Hilfe   F8/Esc: Ende   Return: fertig",
                         s.x(30), s.y(450), gs::colors::white);
            win.display();
        };
        draw();

        bool roundDone = false, quit = false;
        while (win.isOpen() && !roundDone && !quit) {
            while (auto event = win.pollEvent()) {
                if (event->is<sf::Event::Closed>()) { win.close(); return false; }
                if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::F8 ||
                        kp->code == sf::Keyboard::Key::Escape) { quit = true; break; }
                    if (kp->code == sf::Keyboard::Key::F1) {
                        app.helpOn = !app.helpOn; draw();
                    }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        if (slotDone[0] && slotDone[1] && slotDone[2] && slotDone[3])
                            roundDone = true;
                    }
                }
                if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    float mx = (float)mp->position.x, my = (float)mp->position.y;
                    if (mp->button == sf::Mouse::Button::Left) {
                        // Click on a word label to select it
                        for (int i = 0; i < 5; i++) {
                            if (mx >= labelX[i] && mx <= labelX[i] + s.x(120) &&
                                my >= labelY[i] && my <= labelY[i] + s.y(30)) {
                                selectedWord = i;
                                draw();
                            }
                        }
                        // Click on a picture slot to place the selected word
                        if (selectedWord >= 0) {
                            for (int pi = 0; pi < 4; pi++) {
                                if (slotDone[pi]) continue;
                                if (mx >= picX[pi] && mx <= picX[pi] + picW &&
                                    my >= picY && my <= slotY + s.y(27)) {
                                    int wi = wordOrder[selectedWord];
                                    if (wi < 4 && indices[wi] == indices[pi]) {
                                        // Correct!
                                        slotDone[pi] = true;
                                        picRevealed[pi] = true;
                                        app.correctF += app.correctInc();
                                        app.correct1++;
                                        app.lastError = false;
                                    } else {
                                        app.wrong++;
                                        app.lastError = true;
                                    }
                                    selectedWord = -1;
                                    draw();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            gs::delayMs(16);
        }
        if (quit) return false;
    }
    return true;
}

// ─── Exercise: letter-sound clicking (art 6-9) ─────────────────────────────
// Show a picture and ask the child to click the correct letter-sound
// button on an on-screen keyboard.
//
// art 6: find the first letter
// art 7: find a middle letter
// art 8: find the last letter
// art 9: find ALL letters in sequence

static bool exerciseLetterClick(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMd = static_cast<unsigned>(25 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    int art = app.exerciseType;

    // Layout for letter-sound buttons (simplified keyboard)
    struct SoundBtn {
        int idx;
        float x, y, w, h;
    };
    std::vector<SoundBtn> buttons;
    {
        int maxSounds = std::min(kSoundCount, (int)((art < 10) ? kSoundCount : 30));
        float btnH = s.y(27);
        // Row 0: indices 0-9
        for (int i = 0; i < 10 && i < maxSounds; i++)
            buttons.push_back({i, s.x(20) + i * s.x(49), s.y(290), s.x(44), btnH});
        // Row 1: indices 10-19
        for (int i = 10; i < 20 && i < maxSounds; i++)
            buttons.push_back({i, s.x(20) + (i - 10) * s.x(49), s.y(330), s.x(44), btnH});
        // Row 2: indices 20-29
        for (int i = 20; i < 30 && i < maxSounds; i++)
            buttons.push_back({i, s.x(20) + (i - 20) * s.x(49), s.y(370), s.x(44), btnH});
        // Row 3: multi-char sounds 30-37
        if (art < 10) {
            float rx = s.x(20);
            for (int i = 30; i < kSoundCount; i++) {
                float w = s.x(55);
                buttons.push_back({i, rx, s.y(410), w, btnH});
                rx += w + s.x(5);
            }
        }
    }

    int activeSounds = coreWordCount(app.level);

    while (win.isOpen() && app.correctF < 36 && app.wrong < 36) {
        auto picks = pickDistinct(app.wordPoolSize(), 1);
        int wordIdx = mapIndex(picks[0], app.level, app.allWords);
        std::string word = kFibel[wordIdx].word;
        std::string letterUpper = kFibel[wordIdx].letterUpper;

        // For art 9, we need to walk through all letters
        int pos = 0; // current position in word (for art 9)
        bool done = false;

        auto getExpectedLetter = [&]() -> std::string {
            if (art == 6 || art == 10) return letterUpper;
            if (art == 8 || art == 12) {
                // Last letter
                if (word.size() >= 3) {
                    std::string last3 = word.substr(word.size() - 3);
                    // Check multi-char sounds
                    for (auto* s : {"Sch", "sch"})
                        if (last3 == s) return s;
                }
                if (word.size() >= 2) {
                    std::string last2 = word.substr(word.size() - 2);
                    for (auto* s : {"ch", "ng", "ie", u8"äu"})
                        if (last2 == s) return s;
                }
                return word.substr(word.size() - 1);
            }
            // art 7/11: middle letter - just accept any letter that appears in the middle
            return ""; // checked differently
        };

        auto draw = [&]() {
            win.clear();
            drawBackground(win, s);

            // Show picture
            drawPicture(win, app.fibelTex[wordIdx],
                        s.x(260), s.y(30), s.x(118), s.y(157));

            // Word box
            gs::drawRect(win, s.x(257), s.y(197), s.x(381), s.y(227),
                         gs::colors::red, s.x(3));
            gs::drawFilledRect(win, s.x(259), s.y(199), s.x(379), s.y(225),
                               sf::Color::White);

            // Help: show word in red
            if (app.helpOn) {
                gs::drawText(win, f.sans, fMd, word,
                             s.x(319) - gs::textWidth(f.sans, fMd, word) / 2.f,
                             s.y(200), gs::colors::red);
            }

            // Instructions
            std::string instr;
            if (art == 6)      instr = "Wie beginnt das Wort? Klicke an!";
            else if (art == 7) instr = "Suche einen Buchstaben aus der Mitte!";
            else if (art == 8) instr = "Wie endet das Wort?";
            else               instr = "Suche alle Buchstaben der Reihe nach!";
            gs::drawText(win, f.sans, fSm, instr, s.x(20), s.y(245), gs::colors::blue);

            // Keyboard buttons
            for (auto& btn : buttons) {
                bool active = (btn.idx < activeSounds || app.level == 6);
                sf::Color bg = active ? sf::Color::White : sf::Color(220, 220, 220);
                gs::drawFilledRect(win, btn.x, btn.y, btn.x + btn.w, btn.y + btn.h, bg);
                gs::drawRect(win, btn.x, btn.y, btn.x + btn.w, btn.y + btn.h,
                             gs::colors::blue, s.x(2));
                sf::Color fc = active ? gs::colors::black : sf::Color(200, 200, 200);
                gs::drawText(win, f.sans, fMd, kLetterSounds[btn.idx],
                             btn.x + s.x(3), btn.y + s.y(1), fc);
            }

            drawProgressColumn(app);
            gs::drawText(win, f.sans, fSm, "F1: Hilfe   F8/Esc: Ende   Return: weiter",
                         s.x(30), s.y(450), gs::colors::white);
            win.display();
        };
        draw();

        bool quit = false;
        while (win.isOpen() && !done && !quit) {
            while (auto event = win.pollEvent()) {
                if (event->is<sf::Event::Closed>()) { win.close(); return false; }
                if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::F8 ||
                        kp->code == sf::Keyboard::Key::Escape) { quit = true; break; }
                    if (kp->code == sf::Keyboard::Key::F1) {
                        app.helpOn = !app.helpOn; draw();
                    }
                }
                if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button != sf::Mouse::Button::Left) continue;
                    float mx = (float)mp->position.x, my = (float)mp->position.y;
                    for (auto& btn : buttons) {
                        if (mx >= btn.x && mx <= btn.x + btn.w &&
                            my >= btn.y && my <= btn.y + btn.h) {
                            // Determine the single-letter representation
                            std::string sound = kLetterSounds[btn.idx];
                            // Get uppercase first letter of the sound
                            std::string upper1;
                            if (sound.size() >= 2 && (unsigned char)sound[0] >= 0xC0)
                                upper1 = sound.substr(0, 2); // UTF-8 2-byte
                            else
                                upper1 = sound.substr(0, 1);
                            // Also get lowercase
                            std::string lower1;
                            if (sound.find(' ') != std::string::npos)
                                lower1 = sound.substr(sound.find(' ') + 1);
                            else if (sound.size() > 1 && (unsigned char)sound[1] < 0x80)
                                lower1 = sound.substr(1, 1);
                            else
                                lower1 = upper1;

                            bool correct = false;
                            if (art == 6) {
                                // First letter
                                correct = (letterUpper == upper1 || letterUpper == lower1 ||
                                           word.substr(0, upper1.size()) == upper1);
                            } else if (art == 7) {
                                // Middle letter - any letter that's not first or last
                                std::string wlower = word;
                                // Simple: check if lower1 appears in the middle of the word
                                if (word.size() > 2) {
                                    std::string mid = word.substr(1, word.size() - 2);
                                    // Case-insensitive check for the sound in the middle
                                    correct = (mid.find(lower1) != std::string::npos);
                                    if (!correct && lower1 != upper1)
                                        correct = (mid.find(upper1) != std::string::npos);
                                }
                            } else if (art == 8) {
                                // Last letter
                                correct = (word.size() >= lower1.size() &&
                                           word.substr(word.size() - lower1.size()) == lower1);
                                if (!correct && upper1 != lower1)
                                    correct = (word.size() >= upper1.size() &&
                                               word.substr(word.size() - upper1.size()) == upper1);
                            } else { // art == 9
                                // Sequential: check if this is the next expected letter
                                std::string expected;
                                if (pos == 0)
                                    expected = letterUpper;
                                else if ((size_t)pos < word.size())
                                    expected = word.substr(pos, lower1.size());
                                correct = (expected == upper1 || expected == lower1);
                                if (correct) {
                                    pos += (int)lower1.size();
                                    if ((size_t)pos >= word.size()) {
                                        // Completed the whole word
                                    }
                                }
                            }

                            if (correct) {
                                app.correctF += app.correctInc();
                                app.correct1++;
                                app.lastError = false;
                                if (art != 9 || (size_t)pos >= word.size())
                                    done = true;
                            } else {
                                app.wrong++;
                                app.lastError = true;
                                if (art == 9) pos = 0; // reset
                            }
                            draw();
                            if (done) gs::delayMs(500);
                            break;
                        }
                    }
                }
            }
            gs::delayMs(16);
        }
        if (quit) return false;
    }
    return true;
}

// ─── Exercise: letter typing (art 10-13) ────────────────────────────────────
// Same as 6-9 but the child types the letter on the physical keyboard
// instead of clicking on-screen buttons.

static bool exerciseLetterType(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMd = static_cast<unsigned>(25 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    int art = app.exerciseType;

    while (win.isOpen() && app.correctF < 36 && app.wrong < 36) {
        auto picks = pickDistinct(app.wordPoolSize(), 1);
        int wordIdx = mapIndex(picks[0], app.level, app.allWords);
        std::string word = kFibel[wordIdx].word;
        std::string letterUpper = kFibel[wordIdx].letterUpper;
        std::string typed;
        int pos = 0;
        bool done = false;

        auto draw = [&]() {
            win.clear();
            drawBackground(win, s);
            drawPicture(win, app.fibelTex[wordIdx],
                        s.x(260), s.y(30), s.x(118), s.y(157));

            gs::drawRect(win, s.x(257), s.y(197), s.x(381), s.y(227),
                         gs::colors::red, s.x(3));
            gs::drawFilledRect(win, s.x(259), s.y(199), s.x(379), s.y(225),
                               sf::Color::White);
            if (app.helpOn)
                gs::drawText(win, f.sans, fMd, word,
                             s.x(319) - gs::textWidth(f.sans, fMd, word) / 2.f,
                             s.y(200), gs::colors::red);

            std::string instr;
            if (art == 10)     instr = "Schreibe den ersten Buchstaben des Worts!";
            else if (art == 11) instr = "Schreibe einen Buchstaben aus der Mitte!";
            else if (art == 12) instr = "Schreibe den letzten Buchstaben des Worts!";
            else                instr = "Schreibe alle Buchstaben der Reihe nach!";
            gs::drawText(win, f.sans, fSm, instr, s.x(20), s.y(245), gs::colors::blue);

            // Show what has been typed so far
            if (!typed.empty()) {
                gs::drawText(win, f.sans, static_cast<unsigned>(30 * s.fx), typed,
                             s.x(200), s.y(300), gs::colors::blue);
            }

            // Cursor
            float cx = s.x(200) + gs::textWidth(f.sans, static_cast<unsigned>(30 * s.fx), typed);
            gs::drawRect(win, cx, s.y(302), cx + s.x(14), s.y(330), gs::colors::red, 1.f);

            drawProgressColumn(app);
            gs::drawText(win, f.sans, fSm, "F1: Hilfe   F8/Esc: Ende   Return: weiter",
                         s.x(30), s.y(450), gs::colors::white);
            win.display();
        };
        draw();

        bool quit = false;
        while (win.isOpen() && !done && !quit) {
            while (auto event = win.pollEvent()) {
                if (event->is<sf::Event::Closed>()) { win.close(); return false; }
                if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::F8 ||
                        kp->code == sf::Keyboard::Key::Escape) { quit = true; break; }
                    if (kp->code == sf::Keyboard::Key::F1) {
                        app.helpOn = !app.helpOn; draw();
                    }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        if (typed.empty()) continue;
                        // Check the typed letter
                        bool correct = false;
                        std::string input = typed;

                        if (art == 10) {
                            // First letter
                            correct = (word.substr(0, input.size()) == input ||
                                       letterUpper == input);
                        } else if (art == 11) {
                            // Middle letter
                            if (word.size() > 2) {
                                std::string mid = word.substr(1, word.size() - 2);
                                correct = (mid.find(input) != std::string::npos);
                            }
                        } else if (art == 12) {
                            // Last letter
                            correct = (word.size() >= input.size() &&
                                       word.substr(word.size() - input.size()) == input);
                        } else { // art == 13
                            // All letters in sequence
                            std::string expected;
                            if (pos == 0 && word.size() >= input.size())
                                expected = word.substr(0, input.size());
                            else if ((size_t)pos < word.size())
                                expected = word.substr(pos, input.size());
                            // Case-insensitive for pos > 0
                            if (pos > 0) {
                                std::string el = expected, il = input;
                                auto toLower = [](std::string& s2) {
                                    for (auto& c : s2)
                                        if (c >= 'A' && c <= 'Z') c += 32;
                                };
                                toLower(el); toLower(il);
                                correct = (el == il);
                            } else {
                                correct = (expected == input);
                            }
                            if (correct) pos += (int)input.size();
                        }

                        if (correct) {
                            app.correctF += app.correctInc();
                            app.correct1++;
                            app.lastError = false;
                            if (art != 13 || (size_t)pos >= word.size())
                                done = true;
                        } else {
                            app.wrong++;
                            app.lastError = true;
                            if (art == 13) pos = 0;
                        }
                        typed.clear();
                        draw();
                        if (done) gs::delayMs(500);
                    }
                    if (kp->code == sf::Keyboard::Key::Backspace && !typed.empty()) {
                        typed.pop_back();
                        draw();
                    }
                }
                if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                    char32_t ch = te->unicode;
                    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                        ch == 0xC4 || ch == 0xE4 || ch == 0xD6 || ch == 0xF6 ||
                        ch == 0xDC || ch == 0xFC || ch == 0xDF) {
                        // Append character (UTF-8)
                        if (ch < 0x80) {
                            typed += (char)ch;
                        } else if (ch < 0x800) {
                            typed += (char)(0xC0 | (ch >> 6));
                            typed += (char)(0x80 | (ch & 0x3F));
                        }
                        draw();
                    }
                }
            }
            gs::delayMs(16);
        }
        if (quit) return false;
    }
    return true;
}

// ─── Exercise: letter-sound matching / drag (art 3-5) ───────────────────────
// Similar to word matching but with letter-sound buttons instead of words.
// Simplified from the complex GDI drag to click-select + click-place.

static bool exerciseLetterMatch(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMd = static_cast<unsigned>(25 * s.fx);
    unsigned fSm = static_cast<unsigned>(16 * s.fx);
    bool memoryMode = (app.exerciseType == 4 || app.exerciseType == 5);

    while (win.isOpen() && app.correctF < 36 && app.wrong < 36) {
        auto indices = pickDistinct(coreWordCount(app.level), 4);
        for (auto& idx : indices) idx = std::min(idx, kFibelCount - 1);

        bool picRevealed[4] = {!memoryMode, !memoryMode, !memoryMode, !memoryMode};
        bool slotDone[4] = {false, false, false, false};
        int selectedBtn = -1;

        float picX[4] = {s.x(40), s.x(187), s.x(334), s.x(481)};
        float picY = s.y(30), picW = s.x(118), picH = s.y(157);
        float slotY = s.y(200);
        float slotCX[4] = {s.x(99), s.x(246), s.x(393), s.x(540)};

        // Sound buttons for the 4 pictures (shuffled)
        std::vector<int> btnOrder = {0, 1, 2, 3};
        for (int i = 3; i > 0; i--) std::swap(btnOrder[i], btnOrder[gs::randInt(0, i)]);

        float btnX[4], btnY[4];
        for (int i = 0; i < 4; i++) {
            btnX[i] = s.x(37 + i * 130);
            btnY[i] = s.y(280);
        }

        auto draw = [&]() {
            win.clear();
            drawBackground(win, s);

            for (int i = 0; i < 4; i++) {
                if (picRevealed[i] || slotDone[i])
                    drawPicture(win, app.fibelTex[indices[i]],
                                picX[i], picY, picW, picH);
                else
                    gs::drawFilledRect(win, picX[i], picY,
                                       picX[i] + picW, picY + picH,
                                       gs::colors::darkBrown);
                gs::drawRect(win, picX[i] - s.x(3), slotY - s.y(3),
                             picX[i] + picW + s.x(3), slotY + s.y(27),
                             gs::colors::red, s.x(3));
                gs::drawFilledRect(win, picX[i], slotY,
                                   picX[i] + picW, slotY + s.y(25), sf::Color::White);
                if (slotDone[i])
                    gs::drawText(win, f.sans, fMd, kFibel[indices[i]].word,
                                 slotCX[i] - gs::textWidth(f.sans, fMd, kFibel[indices[i]].word) / 2.f,
                                 slotY, gs::colors::black);
                if (app.helpOn && !slotDone[i])
                    gs::drawText(win, f.sans, fSm, kFibel[indices[i]].letterUpper,
                                 slotCX[i], slotY + s.y(1), gs::colors::red);
            }

            // Sound buttons
            for (int i = 0; i < 4; i++) {
                int bi = btnOrder[i];
                if (slotDone[bi]) continue;
                sf::Color bg = (i == selectedBtn) ? gs::colors::blue : sf::Color::White;
                sf::Color fg = (i == selectedBtn) ? gs::colors::creme : gs::colors::black;
                gs::drawFilledRect(win, btnX[i], btnY[i],
                                   btnX[i] + s.x(100), btnY[i] + s.y(30), bg);
                gs::drawRect(win, btnX[i], btnY[i],
                             btnX[i] + s.x(100), btnY[i] + s.y(30),
                             gs::colors::blue, s.x(2));
                gs::drawText(win, f.sans, fMd, kFibel[indices[bi]].letterUpper,
                             btnX[i] + s.x(5), btnY[i] + s.y(2), fg);
            }

            drawProgressColumn(app);
            gs::drawText(win, f.sans, fSm, "F1: Hilfe   F8/Esc: Ende   Return: fertig",
                         s.x(30), s.y(450), gs::colors::white);
            win.display();
        };
        draw();

        bool roundDone = false, quit = false;
        while (win.isOpen() && !roundDone && !quit) {
            while (auto event = win.pollEvent()) {
                if (event->is<sf::Event::Closed>()) { win.close(); return false; }
                if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::F8 ||
                        kp->code == sf::Keyboard::Key::Escape) { quit = true; break; }
                    if (kp->code == sf::Keyboard::Key::F1) { app.helpOn = !app.helpOn; draw(); }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        if (slotDone[0] && slotDone[1] && slotDone[2] && slotDone[3])
                            roundDone = true;
                    }
                }
                if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button != sf::Mouse::Button::Left) continue;
                    float mx = (float)mp->position.x, my = (float)mp->position.y;
                    // Select a sound button
                    for (int i = 0; i < 4; i++) {
                        if (slotDone[btnOrder[i]]) continue;
                        if (mx >= btnX[i] && mx <= btnX[i] + s.x(100) &&
                            my >= btnY[i] && my <= btnY[i] + s.y(30)) {
                            selectedBtn = i;
                            draw();
                        }
                    }
                    // Place on a picture
                    if (selectedBtn >= 0) {
                        for (int pi = 0; pi < 4; pi++) {
                            if (slotDone[pi]) continue;
                            if (mx >= picX[pi] && mx <= picX[pi] + picW &&
                                my >= picY && my <= slotY + s.y(27)) {
                                int bi = btnOrder[selectedBtn];
                                if (bi == pi) {
                                    slotDone[pi] = true;
                                    picRevealed[pi] = true;
                                    app.correctF += app.correctInc();
                                    app.correct1++;
                                    app.lastError = false;
                                } else {
                                    app.wrong++;
                                    app.lastError = true;
                                }
                                selectedBtn = -1;
                                draw();
                                break;
                            }
                        }
                    }
                }
            }
            gs::delayMs(16);
        }
        if (quit) return false;
    }
    return true;
}

// ─── Result display ─────────────────────────────────────────────────────────
static void showResult(App& app) {
    int64_t elapsed = (gs::timerMs() - app.startTime) / 1000;
    int mins = (int)(elapsed / 60), secs = (int)(elapsed % 60);
    std::string msg = app.playerName + ": " +
        std::to_string(app.correct1) + " von " +
        std::to_string(app.correct1 + app.wrong) +
        " Aufgaben in " + std::to_string(mins) + " min " +
        std::to_string(secs) + " s";
    gs::alert(*app.win, app.fonts, app.sc, msg, {"OK"},
              app.wrong < app.correct1 ? &app.grins : &app.heul);
}

// ─── Main ───────────────────────────────────────────────────────────────────
int main() {
    const unsigned W = 800, H = 600;
    sf::RenderWindow window(sf::VideoMode({W, H}),
                            u8"Lesen für Grundschulkinder");
    window.setFramerateLimit(30);

    App app;
    app.win = &window;
    if (!app.fonts.load()) return 1;
    app.sc.init(640, 480, W, H);
    app.loadResources();

    // Name entry
    app.playerName = gs::nameEntryDialog(window, app.fonts, app.sc, "liest");
    if (!window.isOpen()) return 0;
    app.isSingle = (app.playerName.find(" und ") == std::string::npos &&
                    app.playerName.find('+') == std::string::npos);

    // Main loop
    while (window.isOpen()) {
        if (!mainMenu(app)) break;
        if (!exerciseMenu(app)) continue;

        // Reset scores
        app.correctF = 0; app.correct1 = 0; app.wrong = 0;
        app.lastError = false;
        app.startTime = gs::timerMs();

        bool finished = false;
        int art = app.exerciseType;
        if (art <= 2)       finished = exerciseWords(app);
        else if (art <= 5)  finished = exerciseLetterMatch(app);
        else if (art <= 9)  finished = exerciseLetterClick(app);
        else                finished = exerciseLetterType(app);

        if (finished && (app.correct1 > 0 || app.wrong > 0))
            showResult(app);
    }
    return 0;
}