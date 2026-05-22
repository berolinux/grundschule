// Geburtstagsrechner – Birthday / date-span calculator
// Ported from GFA-Basic (Geburtstagsrechner.g32)
//
// Calculates the difference between two dates in days, hours, minutes,
// seconds and also in years/months/days.  Handles the Gregorian calendar
// reform of October 1582.

#include "common.h"
#include <ctime>
#include <iomanip>

struct DateInfo {
    int year = 1988, month = 2, day = 15;
    int hour = 0, minute = 0, second = 0;
};

static bool isLeapYear(int y) {
    if (y <= 0) return (y + 1) % 4 == 0;
    if (y % 400 == 0) return true;
    if (y % 100 == 0 && y > 1582) return false;
    return y % 4 == 0;
}

static int daysInMonth(int m, int y) {
    static const int dm[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y)) return 29;
    return dm[m];
}

static const char* weekdayName(int y, int m, int d) {
    // Zeller's congruence (Gregorian)
    static const char* names[] = {"Samstag","Sonntag","Montag","Dienstag",
                                  "Mittwoch","Donnerstag","Freitag"};
    if (y <= 0) return "---";
    if (m < 3) { m += 12; y--; }
    int h = (d + (13*(m+1))/5 + y + y/4 - y/100 + y/400) % 7;
    return names[h];
}

static long long daysBetween(const DateInfo& a, const DateInfo& b) {
    // Simplified: iterate months (like original)
    DateInfo from = a, to = b;
    bool negative = false;
    if (to.year < from.year ||
        (to.year == from.year && to.month < from.month) ||
        (to.year == from.year && to.month == from.month && to.day < from.day)) {
        std::swap(from, to);
        negative = true;
    }
    long long total = -from.day;
    int j = from.year, m = from.month;
    int endM = (j < to.year) ? 12 : to.month - 1;
    while (j <= to.year) {
        if (j != 0) {
            while (m <= endM) {
                int d = daysInMonth(m, j);
                if (j == 1582 && m == 10) d -= 10; // Gregorian reform
                total += d;
                m++;
            }
        }
        j++;
        if (j < to.year) { m = 1; endM = 12; }
        else if (j == to.year) { m = 1; endM = to.month - 1; }
    }
    total += to.day;
    if (to.year == 1582 && to.month == 10 && to.day > 14)
        total -= 10;
    if (to.year > 0 && from.year < 0)
        total -= daysInMonth(12, -1); // no year 0
    return negative ? -total : total;
}

// Input field
struct Field {
    std::string label;
    std::string value;
    sf::FloatRect bounds;
    bool focused = false;
};

int main() {
    const unsigned W = 700, H = 520;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Geburtstagsrechner");
    window.setFramerateLimit(30);

    gs::Fonts fonts;
    if (!fonts.load()) return 1;

    // Get today's date
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    DateInfo today;
    today.year = tm->tm_year + 1900;
    today.month = tm->tm_mon + 1;
    today.day = tm->tm_mday;
    today.hour = tm->tm_hour;
    today.minute = tm->tm_min;
    today.second = tm->tm_sec;

    // Fields: row1 = start date, row2 = end date
    std::vector<Field> row1 = {
        {"Tag",   std::to_string(today.day),   {}, false},
        {"Monat", std::to_string(today.month), {}, false},
        {"Jahr",  std::to_string(today.year),  {}, false},
        {"Std",   "0", {}, false},
        {"Min",   "0", {}, false},
        {"Sek",   "0", {}, false},
    };
    std::vector<Field> row2 = {
        {"Tag",   std::to_string(today.day),   {}, false},
        {"Monat", std::to_string(today.month), {}, false},
        {"Jahr",  std::to_string(today.year),  {}, false},
        {"Std",   std::to_string(today.hour),  {}, false},
        {"Min",   std::to_string(today.minute),{}, false},
        {"Sek",   std::to_string(today.second),{}, false},
    };

    float y1 = 80, y2 = 140;
    float startX = 100, fW = 70, gap = 10;
    for (size_t i = 0; i < row1.size(); i++) {
        row1[i].bounds = {{startX + i * (fW + gap), y1}, {fW, 28}};
        row2[i].bounds = {{startX + i * (fW + gap), y2}, {fW, 28}};
    }

    int focusRow = 0, focusCol = 0;
    row1[0].focused = true;
    std::string result;
    std::string wday1, wday2;

    auto doCalc = [&]() {
        DateInfo a, b;
        try { a.day = std::stoi(row1[0].value); } catch (...) { a.day = 1; }
        try { a.month = std::stoi(row1[1].value); } catch (...) { a.month = 1; }
        try { a.year = std::stoi(row1[2].value); } catch (...) { a.year = 2000; }
        try { a.hour = std::stoi(row1[3].value); } catch (...) { a.hour = 0; }
        try { a.minute = std::stoi(row1[4].value); } catch (...) { a.minute = 0; }
        try { a.second = std::stoi(row1[5].value); } catch (...) { a.second = 0; }
        try { b.day = std::stoi(row2[0].value); } catch (...) { b.day = 1; }
        try { b.month = std::stoi(row2[1].value); } catch (...) { b.month = 1; }
        try { b.year = std::stoi(row2[2].value); } catch (...) { b.year = 2000; }
        try { b.hour = std::stoi(row2[3].value); } catch (...) { b.hour = 0; }
        try { b.minute = std::stoi(row2[4].value); } catch (...) { b.minute = 0; }
        try { b.second = std::stoi(row2[5].value); } catch (...) { b.second = 0; }

        wday1 = (a.year > 0) ? weekdayName(a.year, a.month, a.day) : "";
        wday2 = (b.year > 0) ? weekdayName(b.year, b.month, b.day) : "";

        long long days = daysBetween(a, b);
        long long totalSec = days * 86400LL +
                             (b.hour - a.hour) * 3600LL +
                             (b.minute - a.minute) * 60LL +
                             (b.second - a.second);

        long long absDays = std::abs(days);
        long long absH = std::abs(totalSec) / 3600;
        long long absMin = std::abs(totalSec) / 60;
        long long absSec = std::abs(totalSec);

        // Years/months/days breakdown
        int years = static_cast<int>(absDays / 365);
        int months = static_cast<int>(absDays / 30);
        int remDays = static_cast<int>(absDays % 30);

        std::ostringstream os;
        os << "Unterschied: " << absDays << " Tage";
        if (absH > 0) os << " = " << absH << " Stunden";
        if (absMin > 0) os << " = " << absMin << " Minuten";
        os << "\n";
        os << "Etwa " << years << " Jahre, " << (months - years * 12) << " Monate, " << remDays << " Tage";
        result = os.str();
    };

    auto draw = [&]() {
        window.clear(sf::Color(40, 20, 0));

        gs::drawText(window, fonts.serif, 28,
                     "Geburtstagsrechner", 180, 10, gs::colors::creme, true);

        // Row labels
        gs::drawText(window, fonts.sans, 18,
                     "Anfangsdatum:", 10, y1 + 4, gs::colors::white);
        gs::drawText(window, fonts.sans, 18,
                     "Enddatum:", 10, y2 + 4, gs::colors::white);

        auto drawRow = [&](std::vector<Field>& row) {
            for (auto& f : row) {
                sf::Color bg = f.focused ? sf::Color::White : sf::Color(220, 220, 220);
                gs::drawFilledRect(window, f.bounds.position.x, f.bounds.position.y,
                                   f.bounds.position.x + f.bounds.size.x,
                                   f.bounds.position.y + f.bounds.size.y, bg);
                gs::drawRect(window, f.bounds.position.x, f.bounds.position.y,
                             f.bounds.position.x + f.bounds.size.x,
                             f.bounds.position.y + f.bounds.size.y,
                             f.focused ? gs::colors::blue : gs::colors::black, 1.f);
                gs::drawText(window, fonts.mono, 16, f.value,
                             f.bounds.position.x + 4, f.bounds.position.y + 4,
                             gs::colors::black);
                gs::drawText(window, fonts.sans, 12, f.label,
                             f.bounds.position.x, f.bounds.position.y - 16,
                             gs::colors::creme);
            }
        };
        drawRow(row1);
        drawRow(row2);

        // Weekdays
        gs::drawText(window, fonts.sans, 16, wday1, startX, y1 + 34, gs::colors::white);
        gs::drawText(window, fonts.sans, 16, wday2, startX, y2 + 34, gs::colors::white);

        // Result
        if (!result.empty()) {
            // Split by newline
            std::istringstream iss(result);
            std::string line;
            float ry = 220;
            while (std::getline(iss, line)) {
                gs::drawText(window, fonts.sans, 20, line, 30, ry, gs::colors::creme);
                ry += 30;
            }
        }

        // Buttons
        gs::drawFilledRect(window, 30, H - 60, 150, H - 30, gs::colors::lightBlue);
        gs::drawRect(window, 30, H - 60, 150, H - 30, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 18, "Berechnen", 45, H - 58, gs::colors::black);

        gs::drawFilledRect(window, 180, H - 60, 280, H - 30, gs::colors::lightBlue);
        gs::drawRect(window, 180, H - 60, 280, H - 30, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 18, "Neu", 210, H - 58, gs::colors::black);

        gs::drawFilledRect(window, 310, H - 60, 410, H - 30, gs::colors::lightBlue);
        gs::drawRect(window, 310, H - 60, 410, H - 30, gs::colors::darkBrown, 2.f);
        gs::drawText(window, fonts.sans, 18, "Ende", 340, H - 58, gs::colors::black);

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

                // Check buttons
                if (my >= H - 60 && my <= H - 30) {
                    if (mx >= 30 && mx <= 150) { doCalc(); needDraw = true; }
                    else if (mx >= 180 && mx <= 280) {
                        for (auto& f : row1) f.value.clear();
                        for (auto& f : row2) f.value.clear();
                        result.clear(); wday1.clear(); wday2.clear();
                        needDraw = true;
                    }
                    else if (mx >= 310 && mx <= 410) { window.close(); break; }
                }

                // Check fields
                auto checkRow = [&](std::vector<Field>& row, int r) {
                    for (size_t i = 0; i < row.size(); i++) {
                        auto& b = row[i].bounds;
                        if (mx >= b.position.x && mx <= b.position.x + b.size.x &&
                            my >= b.position.y && my <= b.position.y + b.size.y) {
                            row1[focusCol].focused = false;
                            row2[focusCol].focused = false;
                            focusRow = r;
                            focusCol = static_cast<int>(i);
                            (r == 0 ? row1 : row2)[focusCol].focused = true;
                            needDraw = true;
                        }
                    }
                };
                checkRow(row1, 0);
                checkRow(row2, 1);
            }

            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) { window.close(); break; }
                if (kp->code == sf::Keyboard::Key::Enter) {
                    // Move to next field, or calculate if last
                    auto& row = (focusRow == 0) ? row1 : row2;
                    row[focusCol].focused = false;
                    focusCol++;
                    if (focusCol >= static_cast<int>(row.size())) {
                        focusCol = 0;
                        focusRow = 1 - focusRow;
                    }
                    (focusRow == 0 ? row1 : row2)[focusCol].focused = true;
                    doCalc();
                    needDraw = true;
                }
                if (kp->code == sf::Keyboard::Key::Backspace) {
                    auto& v = (focusRow == 0 ? row1 : row2)[focusCol].value;
                    if (!v.empty()) v.pop_back();
                    needDraw = true;
                }
            }

            if (auto* te = event->getIf<sf::Event::TextEntered>()) {
                char32_t ch = te->unicode;
                if ((ch >= '0' && ch <= '9') || ch == '-') {
                    (focusRow == 0 ? row1 : row2)[focusCol].value += static_cast<char>(ch);
                    needDraw = true;
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return 0;
}