// Waehrung – Euro currency converter
// Ported from GFA-Basic (Waehrung.g32)
//
// The original was a form-based Windows application with text fields for each
// currency.  The SFML port recreates the same layout using drawn text fields.

#include "common.h"
#include <iomanip>

struct CurrencyField {
    std::string label;
    std::string abbrev;
    double rate;        // how many units per 1 Euro
    std::string value;
    sf::FloatRect bounds;
    bool focused = false;
};

static std::string fmtMoney(double v) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << v;
    return os.str();
}

int main() {
    const unsigned W = 560, H = 520;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Euro-Waehrungsrechner");
    window.setFramerateLimit(30);

    gs::Fonts fonts;
    if (!fonts.load()) return 1;

    std::vector<CurrencyField> fields = {
        {"Euro",          "\u20AC", 1.0,       "", {}, false},
        {"Finnland",      "Fmk",   5.94573,   "", {}, false},
        {"Niederlande",   "hfl",   2.20371,   "", {}, false},
        {"Belgien",       "bfr",   40.3399,   "", {}, false},
        {"Luxemburg",     "lfr",   40.3399,   "", {}, false},
        {"Frankreich",    "FF",    6.55957,   "", {}, false},
        {"Irland",        "IR\u00A3", 0.787564,"",{}, false},
        {"Spanien",       "Pta",   166.386,   "", {}, false},
        {"Portugal",      "Esc",   200.482,   "", {}, false},
        {"Italien",       "Lit",   1936.27,   "", {}, false},
        {"\u00D6sterreich","öS",   13.7603,   "", {}, false},
        {"Deutschland",   "DM",    1.95583,   "", {}, false},
    };

    unsigned fsize = 18;
    float labelW = 140.f, fieldW = 140.f, abbrW = 50.f;
    float startY = 60.f, lineH = 36.f;
    float fieldX = labelW + 20.f;

    for (size_t i = 0; i < fields.size(); i++) {
        fields[i].bounds = sf::FloatRect(
            {fieldX, startY + i * lineH},
            {fieldW, lineH - 4.f});
    }

    int focusIdx = 0;
    fields[0].focused = true;
    double euroVal = 0.0;

    auto recalc = [&](int from) {
        if (from == 0) {
            // Euro field changed
            try { euroVal = std::stod(fields[0].value); } catch (...) { euroVal = 0; }
        } else {
            try {
                double v = std::stod(fields[from].value);
                euroVal = v / fields[from].rate;
            } catch (...) { euroVal = 0; }
        }
        for (size_t i = 0; i < fields.size(); i++) {
            if (static_cast<int>(i) != from)
                fields[i].value = fmtMoney(euroVal * fields[i].rate);
        }
    };

    auto draw = [&]() {
        window.clear(sf::Color(60, 30, 0));

        // Title
        gs::drawText(window, fonts.serif, 26,
                     "Euro-Waehrungsrechner", 100.f, 10.f,
                     gs::colors::creme, true);

        for (size_t i = 0; i < fields.size(); i++) {
            float y = startY + i * lineH;
            // Label
            gs::drawText(window, fonts.sans, fsize,
                         fields[i].label, 10.f, y + 4.f,
                         gs::colors::creme);
            // Field background
            sf::Color bg = fields[i].focused ? sf::Color::White : sf::Color(230, 230, 230);
            gs::drawFilledRect(window, fieldX, y,
                               fieldX + fieldW, y + lineH - 4.f, bg);
            gs::drawRect(window, fieldX, y,
                         fieldX + fieldW, y + lineH - 4.f,
                         fields[i].focused ? gs::colors::blue : gs::colors::black, 1.f);
            // Value
            gs::drawText(window, fonts.mono, fsize - 2,
                         fields[i].value,
                         fieldX + 4.f, y + 4.f, gs::colors::black);
            // Abbreviation
            gs::drawText(window, fonts.sans, fsize - 2,
                         fields[i].abbrev,
                         fieldX + fieldW + 10.f, y + 4.f, gs::colors::creme);
        }

        // Instructions
        gs::drawText(window, fonts.sans, 14,
                     "Klicke ein Feld an und tippe einen Betrag. Return = Umrechnen.",
                     10.f, static_cast<float>(H) - 30.f, gs::colors::white);

        window.display();
    };

    draw();

    while (window.isOpen()) {
        bool needDraw = false;
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close(); break; }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                for (size_t i = 0; i < fields.size(); i++) {
                    auto& b = fields[i].bounds;
                    if (mx >= b.position.x && mx <= b.position.x + b.size.x &&
                        my >= b.position.y && my <= b.position.y + b.size.y) {
                        fields[focusIdx].focused = false;
                        focusIdx = static_cast<int>(i);
                        fields[focusIdx].focused = true;
                        fields[focusIdx].value.clear();
                        needDraw = true;
                    }
                }
            }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Enter) {
                    recalc(focusIdx);
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Backspace) {
                    auto& v = fields[focusIdx].value;
                    if (!v.empty()) v.pop_back();
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
                if (kp->code == sf::Keyboard::Key::Tab) {
                    fields[focusIdx].focused = false;
                    focusIdx = (focusIdx + 1) % static_cast<int>(fields.size());
                    fields[focusIdx].focused = true;
                    fields[focusIdx].value.clear();
                    needDraw = true;
                }
            }
            if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                char32_t ch = te->unicode;
                if ((ch >= '0' && ch <= '9') || ch == '.' || ch == ',') {
                    if (ch == ',') ch = '.';
                    fields[focusIdx].value += static_cast<char>(ch);
                    needDraw = true;
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return 0;
}