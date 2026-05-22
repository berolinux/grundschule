// Kalender – Monthly calendar display
// Ported from GFA-Basic (Kalender.g32)
//
// Shows a monthly calendar page.  The user can browse months with arrow
// buttons.

#include "common.h"
#include <ctime>
#include <iomanip>

static bool isLeapYear(int y) {
    if (y % 400 == 0) return true;
    if (y % 100 == 0) return false;
    return y % 4 == 0;
}

static int daysInMonth(int m, int y) {
    static const int dm[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y)) return 29;
    return dm[m];
}

// Day-of-week: 0=Mon .. 6=Sun
static int dow(int y, int m, int d) {
    // Tomohiko Sakamoto's algorithm
    static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    if (m < 3) y--;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

static const char* monthName(int m) {
    static const char* names[] = {"","Januar","Februar","März","April",
        "Mai","Juni","Juli","August","September","Oktober","November","Dezember"};
    return names[m];
}

int main() {
    const unsigned W = 640, H = 520;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Kalender");
    window.setFramerateLimit(30);

    gs::Fonts fonts;
    if (!fonts.load()) return 1;

    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    int curMonth = tm->tm_mon + 1;
    int curYear = tm->tm_year + 1900;
    int todayD = tm->tm_mday, todayM = curMonth, todayY = curYear;

    auto draw = [&]() {
        window.clear(sf::Color(40, 20, 0));

        // Title
        std::string title = std::string(monthName(curMonth)) + "  " + std::to_string(curYear);
        gs::drawTextCentered(window, fonts.serif, 32, title, 20, gs::colors::creme, true);

        // Day-of-week headers
        const char* dayNames[] = {"Mo","Di","Mi","Do","Fr","Sa","So"};
        float cellW = 80.f, cellH = 50.f;
        float startX = 30.f, startY = 80.f;
        for (int i = 0; i < 7; i++) {
            float cx = startX + i * cellW;
            gs::drawFilledRect(window, cx, startY, cx + cellW - 2, startY + 28, gs::colors::darkBrown);
            sf::Color tc = (i >= 5) ? gs::colors::red : gs::colors::creme;
            gs::drawText(window, fonts.sans, 18, dayNames[i],
                         cx + (cellW - 30) / 2, startY + 4, tc, true);
        }

        // Days
        int dim = daysInMonth(curMonth, curYear);
        int wd = dow(curYear, curMonth, 1);  // 0=Mon
        float dy = startY + 34;
        int col = wd;
        for (int d = 1; d <= dim; d++) {
            float cx = startX + col * cellW;
            // Today highlight
            if (d == todayD && curMonth == todayM && curYear == todayY)
                gs::drawFilledRect(window, cx, dy, cx + cellW - 2, dy + cellH - 2,
                                   sf::Color(0, 60, 120));
            // Grid
            gs::drawRect(window, cx, dy, cx + cellW - 2, dy + cellH - 2,
                         gs::colors::darkBrown, 1.f);
            sf::Color dc = (col >= 5) ? gs::colors::red : gs::colors::creme;
            gs::drawText(window, fonts.sans, 22, std::to_string(d),
                         cx + 5, dy + 10, dc);
            col++;
            if (col > 6) { col = 0; dy += cellH; }
        }

        // Navigation buttons
        float btnY = H - 50.f;
        gs::drawFilledRect(window, 30, btnY, 140, btnY + 36, gs::colors::lightBlue);
        gs::drawRect(window, 30, btnY, 140, btnY + 36, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 20, "<  Zurück", 40, btnY + 6, gs::colors::black);

        gs::drawFilledRect(window, 260, btnY, 380, btnY + 36, gs::colors::lightBlue);
        gs::drawRect(window, 260, btnY, 380, btnY + 36, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 20, "Heute", 290, btnY + 6, gs::colors::black);

        gs::drawFilledRect(window, 500, btnY, 610, btnY + 36, gs::colors::lightBlue);
        gs::drawRect(window, 500, btnY, 610, btnY + 36, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 20, "Weiter  >", 510, btnY + 6, gs::colors::black);

        window.display();
    };

    draw();

    while (window.isOpen()) {
        bool needDraw = false;
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close(); break; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape ||
                    kp->code == sf::Keyboard::Key::F8)
                    { window.close(); break; }
                if (kp->code == sf::Keyboard::Key::Left) {
                    curMonth--; if (curMonth < 1) { curMonth = 12; curYear--; }
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Right) {
                    curMonth++; if (curMonth > 12) { curMonth = 1; curYear++; }
                    needDraw = true;
                }
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                float btnY2 = H - 50.f;
                if (my >= btnY2 && my <= btnY2 + 36) {
                    if (mx >= 30 && mx <= 140) {
                        curMonth--; if (curMonth < 1) { curMonth = 12; curYear--; }
                        needDraw = true;
                    }
                    else if (mx >= 260 && mx <= 380) {
                        curMonth = todayM; curYear = todayY; needDraw = true;
                    }
                    else if (mx >= 500 && mx <= 610) {
                        curMonth++; if (curMonth > 12) { curMonth = 1; curYear++; }
                        needDraw = true;
                    }
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return 0;
}