#include "common.h"
#include <iostream>
#include <thread>

namespace gs {

// ---------------------------------------------------------------------------
// dataDir – locate the Bild/ Texte/ directories relative to the executable
// ---------------------------------------------------------------------------
static std::filesystem::path gDataDir;

std::filesystem::path dataDir() {
    if (gDataDir.empty()) {
        // Try the typical locations: CWD, then parent of executable
        for (auto candidate : {".", ".."}) {
            auto p = std::filesystem::canonical(candidate);
            if (std::filesystem::exists(p / "Bild")) {
                gDataDir = p;
                return gDataDir;
            }
        }
        // Fallback: just use CWD
        gDataDir = std::filesystem::current_path();
    }
    return gDataDir;
}

// ---------------------------------------------------------------------------
// Fonts
// ---------------------------------------------------------------------------
static const char* findFont(const std::vector<const char*>& candidates) {
    for (auto c : candidates)
        if (std::filesystem::exists(c))
            return c;
    return candidates.front();
}

bool Fonts::load() {
    const char* sansPath = findFont({
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/opentype/urw-base35/NimbusSans-Regular.otf"
    });
    const char* serifPath = findFont({
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/opentype/urw-base35/NimbusRoman-Regular.otf"
    });
    const char* monoPath = findFont({
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/opentype/urw-base35/NimbusMonoPS-Regular.otf"
    });
    bool ok = true;
    if (!sans.openFromFile(sansPath))   { std::cerr << "Cannot load sans font\n";  ok = false; }
    if (!serif.openFromFile(serifPath)) { std::cerr << "Cannot load serif font\n"; ok = false; }
    if (!mono.openFromFile(monoPath))   { std::cerr << "Cannot load mono font\n";  ok = false; }
    return ok;
}

// ---------------------------------------------------------------------------
// ScaleFactors
// ---------------------------------------------------------------------------
void ScaleFactors::init(unsigned baseW, unsigned baseH,
                        unsigned actualW, unsigned actualH) {
    winW = actualW;
    winH = actualH;
    fx = static_cast<float>(actualW) / static_cast<float>(baseW);
    fy = static_cast<float>(actualH) / static_cast<float>(baseH);
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------
void drawFilledRect(sf::RenderTarget& rt, float x1, float y1,
                    float x2, float y2, sf::Color fill) {
    sf::RectangleShape r({x2 - x1, y2 - y1});
    r.setPosition({x1, y1});
    r.setFillColor(fill);
    r.setOutlineThickness(0);
    rt.draw(r);
}

void drawRect(sf::RenderTarget& rt, float x1, float y1,
              float x2, float y2, sf::Color outline, float thickness) {
    sf::RectangleShape r({x2 - x1 - thickness, y2 - y1 - thickness});
    r.setPosition({x1 + thickness / 2.f, y1 + thickness / 2.f});
    r.setFillColor(sf::Color::Transparent);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thickness);
    rt.draw(r);
}

void drawRoundedRect(sf::RenderTarget& rt, float x1, float y1,
                     float x2, float y2, sf::Color outline,
                     float thickness, sf::Color fill) {
    // Simple implementation: just use a regular rect (rounded corners are
    // cosmetic and not essential to functionality)
    sf::RectangleShape r({x2 - x1 - thickness, y2 - y1 - thickness});
    r.setPosition({x1 + thickness / 2.f, y1 + thickness / 2.f});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thickness);
    rt.draw(r);
}

void drawLine(sf::RenderTarget& rt, float x1, float y1,
              float x2, float y2, sf::Color color, float thickness) {
    sf::Vector2f dir(x2 - x1, y2 - y1);
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    sf::RectangleShape line({len, thickness});
    line.setPosition({x1, y1});
    line.setFillColor(color);
    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
    line.setRotation(sf::degrees(angle));
    rt.draw(line);
}

void drawText(sf::RenderTarget& rt, const sf::Font& font, unsigned size,
              const std::string& str, float x, float y, sf::Color color,
              bool bold) {
    sf::Text text(font, str, size);
    text.setPosition({x, y});
    text.setFillColor(color);
    if (bold) text.setStyle(sf::Text::Bold);
    rt.draw(text);
}

void drawTextCentered(sf::RenderTarget& rt, const sf::Font& font,
                      unsigned size, const std::string& str, float y,
                      sf::Color color, bool bold) {
    sf::Text text(font, str, size);
    float w = text.getLocalBounds().size.x;
    float winW = static_cast<float>(rt.getSize().x);
    text.setPosition({(winW - w) / 2.f, y});
    text.setFillColor(color);
    if (bold) text.setStyle(sf::Text::Bold);
    rt.draw(text);
}

float textWidth(const sf::Font& font, unsigned size, const std::string& str) {
    sf::Text text(font, str, size);
    return text.getLocalBounds().size.x;
}

// ---------------------------------------------------------------------------
// Texture / Image loading with path fixup
// ---------------------------------------------------------------------------
static std::filesystem::path fixPath(const std::string& rel) {
    auto base = dataDir();
    // Replace backslashes
    std::string fixed = rel;
    std::replace(fixed.begin(), fixed.end(), '\\', '/');

    auto full = base / fixed;
    if (std::filesystem::exists(full))
        return full;

    // Try case-insensitive search in the immediate directory
    auto parent = full.parent_path();
    auto filename = full.filename().string();
    std::string filenameLower = filename;
    std::transform(filenameLower.begin(), filenameLower.end(),
                   filenameLower.begin(), ::tolower);
    if (std::filesystem::exists(parent)) {
        for (auto& entry : std::filesystem::directory_iterator(parent)) {
            std::string name = entry.path().filename().string();
            std::string nameLower = name;
            std::transform(nameLower.begin(), nameLower.end(),
                           nameLower.begin(), ::tolower);
            if (nameLower == filenameLower)
                return entry.path();
        }
    }
    return full; // return as-is; will fail at load time with a clear error
}

bool loadTexture(sf::Texture& tex, const std::string& relativePath) {
    auto p = fixPath(relativePath);
    if (!tex.loadFromFile(p)) {
        std::cerr << "Failed to load texture: " << p << "\n";
        return false;
    }
    return true;
}

bool loadImage(sf::Image& img, const std::string& relativePath) {
    auto p = fixPath(relativePath);
    if (!img.loadFromFile(p)) {
        std::cerr << "Failed to load image: " << p << "\n";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Decorative frame
// ---------------------------------------------------------------------------
void drawFrame(sf::RenderTarget& rt, const ScaleFactors& s) {
    float w = static_cast<float>(s.winW);
    float h = static_cast<float>(s.winH);

    // Dark background
    drawFilledRect(rt, 0, 0, w, h, colors::veryDark);
    // Creme inner area
    drawFilledRect(rt, s.x(3), s.y(3), s.x(633), s.y(475), colors::creme);
    // Red border
    drawRect(rt, s.x(6), s.y(7), s.x(632), s.y(473), colors::red, s.x(6));
}

// ---------------------------------------------------------------------------
// Name entry
// ---------------------------------------------------------------------------
std::string nameEntryDialog(sf::RenderWindow& win, const Fonts& fonts,
                            const ScaleFactors& s, const std::string& verb) {
    std::string name;
    sf::String inputStr;

    auto draw = [&]() {
        win.clear();
        drawFrame(win, s);

        drawText(win, fonts.sans, static_cast<unsigned>(21 * s.fx),
                 "Gib bitte deinen Namen ein,",
                 s.x(30), s.y(300), colors::blue);
        drawText(win, fonts.sans, static_cast<unsigned>(21 * s.fx),
                 "danach die Fertig-Taste (Return)!",
                 s.x(30), s.y(320), colors::blue);

        // Show current name
        drawText(win, fonts.sans, static_cast<unsigned>(25 * s.fx),
                 name, s.x(30), s.y(360), colors::black);

        // Cursor
        float cx = s.x(30) + textWidth(fonts.sans, static_cast<unsigned>(25 * s.fx), name);
        drawRect(win, cx, s.y(362), cx + s.x(14), s.y(380), colors::red, 1.f);

        win.display();
    };

    draw();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                win.close();
                return name.empty() ? "???" : name;
            }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Enter) {
                    if (name.empty()) name = "???";
                    return name;
                }
                if (kp->code == sf::Keyboard::Key::Backspace && !name.empty()) {
                    name.pop_back();
                    draw();
                }
                if (kp->code == sf::Keyboard::Key::Escape) {
                    return name.empty() ? "???" : name;
                }
            }
            if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                char32_t ch = te->unicode;
                if (ch >= 32 && ch != 127 && name.size() < 32) {
                    // Accept letters, spaces, umlauts, +
                    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                        ch == ' ' || ch == '+' ||
                        ch == 0xC4 || ch == 0xE4 ||  // Ä ä
                        ch == 0xD6 || ch == 0xF6 ||  // Ö ö
                        ch == 0xDC || ch == 0xFC ||  // Ü ü
                        ch == 0xDF) {                 // ß
                        name += static_cast<char>(ch);
                        // Trim leading spaces
                        while (!name.empty() && name[0] == ' ')
                            name.erase(name.begin());
                        draw();
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return name.empty() ? "???" : name;
}

// ---------------------------------------------------------------------------
// Alert dialog
// ---------------------------------------------------------------------------
int alert(sf::RenderWindow& win, const Fonts& fonts, const ScaleFactors& s,
          const std::string& message,
          const std::vector<std::string>& buttons,
          const sf::Texture* smiley) {
    int n = static_cast<int>(buttons.size());
    unsigned fontSize = static_cast<unsigned>(21 * s.fx);

    auto draw = [&]() {
        // Semi-transparent overlay
        drawFilledRect(win, s.x(75), s.y(175), s.x(563), s.y(303), colors::darkBrown);
        drawRect(win, s.x(78), s.y(178), s.x(560), s.y(300), colors::red, s.fx);
        drawFilledRect(win, s.x(87), s.y(187), s.x(551), s.y(291), colors::creme);

        drawText(win, fonts.sans, fontSize, message,
                 s.x(99), s.y(210), colors::black);

        if (smiley) {
            sf::Sprite sp(*smiley);
            sp.setPosition({s.x(489), s.y(195)});
            sp.setScale({s.x(50) / static_cast<float>(smiley->getSize().x),
                         s.y(56) / static_cast<float>(smiley->getSize().y)});
            win.draw(sp);
        }

        float totalW = s.x(464 - 174);
        float btnW = totalW / n - s.x(10);
        float startX = s.x(174);
        for (int i = 0; i < n; i++) {
            float bx = startX + i * (btnW + s.x(10));
            drawFilledRect(win, bx, s.y(258), bx + btnW, s.y(280), colors::lightBlue);
            drawRect(win, bx, s.y(258), bx + btnW, s.y(280), colors::darkBrown, s.fx * 2);
            drawText(win, fonts.sans, fontSize, buttons[i],
                     bx + s.x(10), s.y(258), colors::black);
        }
        win.display();
    };

    draw();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return 0; }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mp->button == sf::Mouse::Button::Left) {
                    float mx = static_cast<float>(mp->position.x);
                    float my = static_cast<float>(mp->position.y);
                    if (my >= s.y(258) && my <= s.y(280)) {
                        float totalW2 = s.x(464 - 174);
                        float btnW2 = totalW2 / n - s.x(10);
                        float startX2 = s.x(174);
                        for (int i = 0; i < n; i++) {
                            float bx = startX2 + i * (btnW2 + s.x(10));
                            if (mx >= bx && mx <= bx + btnW2)
                                return i + 1;
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Progress bars
// ---------------------------------------------------------------------------
void drawProgressBars(sf::RenderTarget& rt, const ScaleFactors& s,
                      int correct, int wrong,
                      const sf::Texture* smileyHappy,
                      const sf::Texture* smileySad,
                      bool lastWasError) {
    if (correct > 0 && correct <= 36) {
        drawFilledRect(rt, s.x(570), s.y(460) - correct * s.y(4),
                       s.x(611), s.y(467), colors::blue);
    }
    if (wrong > 0 && wrong <= 36) {
        drawFilledRect(rt, s.x(512), s.y(467),
                       s.x(552), s.y(461) - wrong * s.y(4), colors::grey);
    }
    const sf::Texture* face = lastWasError ? smileySad : smileyHappy;
    if (face) {
        sf::Sprite sp(*face);
        sp.setPosition({s.x(536), s.y(256)});
        sp.setScale({s.x(50) / static_cast<float>(face->getSize().x),
                     s.y(56) / static_cast<float>(face->getSize().y)});
        rt.draw(sp);
    }
}

// ---------------------------------------------------------------------------
// Timer helpers
// ---------------------------------------------------------------------------
int64_t timerMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void delayMs(int64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---------------------------------------------------------------------------
// Grid menu
// ---------------------------------------------------------------------------
int gridMenu(sf::RenderWindow& win, const Fonts& fonts, const ScaleFactors& s,
             const std::string& title,
             const std::vector<std::string>& items,
             const std::string& playerName) {
    unsigned fontSmall = static_cast<unsigned>(16 * s.fx);
    unsigned fontMed   = static_cast<unsigned>(25 * s.fx);
    int selected = 0;
    int n = static_cast<int>(items.size());

    auto draw = [&]() {
        win.clear();
        // Background
        drawFilledRect(win, 0, 0, static_cast<float>(s.winW),
                       static_cast<float>(s.winH), colors::veryDark);
        drawFilledRect(win, s.x(3), s.y(3), s.x(633), s.y(475), colors::darkBrown);
        drawFilledRect(win, s.x(4), s.y(77), s.x(632), s.y(413), colors::creme);
        drawRect(win, s.x(6), s.y(7), s.x(632), s.y(473), colors::red, s.x(6));

        // Title
        drawText(win, fonts.sans, fontSmall, title,
                 s.x(60), s.y(20), colors::white);
        drawTextCentered(win, fonts.sans, fontSmall, playerName,
                         s.y(464), colors::white);

        // Items
        float lineH = s.y(26);
        float startY = s.y(76);
        for (int i = 0; i < n; i++) {
            float iy = startY + i * lineH;
            // Separator line
            drawLine(win, s.x(6), iy + lineH, s.x(631), iy + lineH,
                     colors::red, s.fx * 2);

            sf::Color fg = (i == selected) ? colors::creme : colors::blue;
            sf::Color bg = (i == selected) ? colors::blue : sf::Color::Transparent;
            if (bg != sf::Color::Transparent)
                drawFilledRect(win, s.x(12), iy, s.x(626), iy + lineH, bg);

            // Number
            if (i > 0) {
                std::string num = std::to_string(i) + ".";
                drawText(win, fonts.sans, fontMed, num,
                         s.x(45) - textWidth(fonts.sans, fontMed, num),
                         iy, colors::black);
            }
            drawText(win, fonts.sans, fontMed, items[i],
                     s.x(116), iy, fg);
        }

        // Instructions
        drawText(win, fonts.sans, fontSmall,
                 "linke Maustaste: Auswahl    rechte Maustaste: weiter    F8: Ende",
                 s.x(40), s.y(425), colors::white);

        win.display();
    };

    draw();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return 0; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape)
                    return 0;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float my = static_cast<float>(mp->position.y);
                float startY2 = s.y(76);
                float lineH2 = s.y(26);
                int idx = static_cast<int>((my - startY2) / lineH2);
                if (idx >= 0 && idx < n) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        selected = idx;
                        draw();
                    } else if (mp->button == sf::Mouse::Button::Right) {
                        return selected + 1;
                    }
                }
                if (mp->button == sf::Mouse::Button::Right)
                    return std::max(1, selected + 1);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Random
// ---------------------------------------------------------------------------
static std::mt19937& rng() {
    static std::mt19937 gen(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    return gen;
}

int randInt(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng());
}

} // namespace gs