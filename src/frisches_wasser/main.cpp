// Frisches Wasser – Water supply physics educational program
// Ported from GFA-Basic (Frisches_Wasser.g32)
//
// An interactive lesson about water supply:
//   1. Can water flow uphill without a pump? (U-tube experiment)
//   2. Water in different-shaped pipes (communicating vessels)
//   3. Finding a good location for a water tank (Hochbehälter)
//   4. Finding a good location for a well (Brunnen)
//   5. Building and operating a pump
//   6. Complete water pipeline to houses
//
// The original used pixel-level animations at various resolutions (800-1600).
// This port uses the 1152×864 base images, scales to the window, and
// simplifies the animations while preserving the interactive educational flow.

#include "common.h"
#include <cmath>

// ── Textures ────────────────────────────────────────────────────────────────
struct WaterTextures {
    sf::Texture bg;       // hintergrund (landscape with houses)
    sf::Texture bg1;      // hintergrund1 (underground cross-section)
    // Valve textures (right-side and left-side)
    sf::Texture vr_o, vr_u, vr_a, vr_b, vr_c, vr_vo, vr_vu;
    sf::Texture vl_o, vl_u, vl_a, vl_b, vl_c, vl_vo, vl_vu;

    bool load() {
        bool ok = true;
        ok &= gs::loadTexture(bg,  "Bild/Wasserleitung/hintergrund1.jpg");
        ok &= gs::loadTexture(bg1, "Bild/Wasserleitung/hintergrund800.jpg");
        ok &= gs::loadTexture(vr_o,  "Bild/Wasserleitung/Ventil1o.jpg");
        ok &= gs::loadTexture(vr_u,  "Bild/Wasserleitung/Ventil1u.jpg");
        ok &= gs::loadTexture(vr_a,  "Bild/Wasserleitung/Ventil1a.jpg");
        ok &= gs::loadTexture(vr_b,  "Bild/Wasserleitung/Ventil1b.jpg");
        ok &= gs::loadTexture(vr_c,  "Bild/Wasserleitung/Ventil1c.jpg");
        ok &= gs::loadTexture(vr_vo, "Bild/Wasserleitung/Ventil1vo.jpg");
        ok &= gs::loadTexture(vr_vu, "Bild/Wasserleitung/Ventil1vu.jpg");
        ok &= gs::loadTexture(vl_o,  "Bild/Wasserleitung/Ventil2o.jpg");
        ok &= gs::loadTexture(vl_u,  "Bild/Wasserleitung/Ventil2u.jpg");
        ok &= gs::loadTexture(vl_a,  "Bild/Wasserleitung/Ventil2a.jpg");
        ok &= gs::loadTexture(vl_b,  "Bild/Wasserleitung/Ventil2b.jpg");
        ok &= gs::loadTexture(vl_c,  "Bild/Wasserleitung/Ventil2c.jpg");
        ok &= gs::loadTexture(vl_vo, "Bild/Wasserleitung/Ventil2vo.jpg");
        ok &= gs::loadTexture(vl_vu, "Bild/Wasserleitung/Ventil2vu.jpg");
        return ok;
    }
};

// ── Application state ───────────────────────────────────────────────────────
struct App {
    sf::RenderWindow* win = nullptr;
    gs::Fonts fonts;
    gs::ScaleFactors sc;
    WaterTextures tex;
    int task = 0;          // current task (0=title, 1..6)

    // Task 1+2: water amount selected from palette (0 = none)
    int waterAmount = 0;
    float waterLevel = 0.f; // animation progress 0..1

    // Task 3: water tank placement
    bool tankPlaced = false;
    sf::Vector2f tankPos;

    // Task 4: well placement
    bool wellPlaced = false;
    sf::Vector2f wellPos;

    // Task 5: pump
    bool pistonUp = false;
    bool pumpFilled = false;
    int pumpCycles = 0;

    // Task 6: complete
    bool finished = false;
};

// ── Draw helper: stretch texture to fill window ─────────────────────────────
static void drawBg(sf::RenderWindow& win, const sf::Texture& tex,
                   const gs::ScaleFactors& sc) {
    sf::Sprite sp(tex);
    sp.setScale({static_cast<float>(sc.winW) / tex.getSize().x,
                 static_cast<float>(sc.winH) / tex.getSize().y});
    win.draw(sp);
}

// ── Draw: blue water bar palette (for tasks 1 & 2) ─────────────────────────
static void drawWaterPalette(sf::RenderWindow& win, const gs::ScaleFactors& s,
                             const gs::Fonts& fonts, int selected) {
    unsigned fSm = static_cast<unsigned>(17 * s.fx);
    gs::drawText(win, fonts.sans, fSm, "wenig Wasser", s.x(175), s.y(43), gs::colors::blue);
    gs::drawText(win, fonts.sans, fSm, "viel Wasser",  s.x(880), s.y(43), gs::colors::blue);

    int steps = 45;
    float barX = s.x(175), barY = s.y(60), barW = s.x(17), barH = s.y(35);
    for (int i = 0; i < steps; i++) {
        int r = 185 - i * 3, g = 235 - i * 3, b = 255 - i * 3;
        if (r < 0) r = 0; if (g < 0) g = 0; if (b < 0) b = 0;
        sf::Color c(r, g, b);
        float x = barX + i * barW;
        gs::drawFilledRect(win, x, barY, x + barW, barY + barH, c);
        if (i == selected - 1)
            gs::drawRect(win, x, barY, x + barW, barY + barH, gs::colors::red, 2.f);
    }
}

// ── Draw: U-tube pipe (task 1) ──────────────────────────────────────────────
static void drawUTube(sf::RenderWindow& win, const gs::ScaleFactors& s,
                      float fillLevel) {
    // Pipe outline (U-shape)
    float cx = s.x(500), cy = s.y(152), rx = s.x(435), ry = s.y(520);
    // Dark blue outline
    sf::Color pipeOuter(0, 80, 160);
    sf::Color pipeInner(255, 255, 255);
    sf::Color water(100, 220, 255);

    // Draw a simplified U-pipe: two vertical sections connected at bottom
    float leftX = s.x(65), rightX = s.x(935);
    float pipeW = s.x(30);
    float topY = s.y(120), bottomY = s.y(700);

    // Outer pipe
    gs::drawFilledRect(win, leftX, topY, leftX + pipeW, bottomY, pipeOuter);
    gs::drawFilledRect(win, leftX, bottomY - pipeW, rightX + pipeW, bottomY, pipeOuter);
    gs::drawFilledRect(win, rightX, topY + s.y(200), rightX + pipeW, bottomY, pipeOuter);

    // Inner pipe (white)
    float inset = s.x(6);
    gs::drawFilledRect(win, leftX + inset, topY + inset,
                       leftX + pipeW - inset, bottomY - inset, pipeInner);
    gs::drawFilledRect(win, leftX + inset, bottomY - pipeW + inset,
                       rightX + pipeW - inset, bottomY - inset, pipeInner);
    gs::drawFilledRect(win, rightX + inset, topY + s.y(200) + inset,
                       rightX + pipeW - inset, bottomY - inset, pipeInner);

    // Water fill
    if (fillLevel > 0.f) {
        float maxFill = bottomY - topY - 2 * inset;
        float leftFill = maxFill * fillLevel;
        float rightFill = maxFill * std::min(fillLevel, 0.8f);

        // Water in bottom
        gs::drawFilledRect(win, leftX + inset, bottomY - pipeW + inset,
                           rightX + pipeW - inset, bottomY - inset, water);

        // Water in left pipe (higher)
        float leftWaterTop = bottomY - inset - leftFill;
        gs::drawFilledRect(win, leftX + inset, leftWaterTop,
                           leftX + pipeW - inset, bottomY - pipeW + inset, water);

        // Water in right pipe (same level as left)
        float rightWaterTop = bottomY - inset - rightFill;
        if (rightWaterTop < topY + s.y(200) + inset)
            rightWaterTop = topY + s.y(200) + inset;
        gs::drawFilledRect(win, rightX + inset, rightWaterTop,
                           rightX + pipeW - inset, bottomY - pipeW + inset, water);
    }
}

// ── Draw: five pipes (task 2) ───────────────────────────────────────────────
static void drawFivePipes(sf::RenderWindow& win, const gs::ScaleFactors& s,
                          float fillLevel) {
    sf::Color pipeC(0, 80, 160);
    sf::Color innerC(255, 255, 255);
    sf::Color water(100, 220, 255);
    float pipeW = s.x(12);
    float inset = s.x(3);
    float bottomY = s.y(620);

    struct PipeDef { float x; float topY; float width; const char* label; };
    PipeDef pipes[] = {
        {s.x(65),  s.y(300), pipeW,       "Rohr A"},
        {s.x(200), s.y(320), s.x(50),     "Rohr B"},
        {s.x(370), s.y(320), s.x(20),     "Rohr C"},
        {s.x(530), s.y(320), s.x(50),     "Rohr D"},
        {s.x(700), s.y(310), s.x(30),     "Rohr E"},
    };

    // Connect at bottom
    gs::drawFilledRect(win, s.x(55), bottomY, s.x(760), bottomY + pipeW, pipeC);
    gs::drawFilledRect(win, s.x(55) + inset, bottomY + inset,
                       s.x(760) - inset, bottomY + pipeW - inset, innerC);

    for (auto& p : pipes) {
        gs::drawFilledRect(win, p.x, p.topY, p.x + p.width, bottomY, pipeC);
        gs::drawFilledRect(win, p.x + inset, p.topY + inset,
                           p.x + p.width - inset, bottomY - inset, innerC);
    }

    // Water
    if (fillLevel > 0.f) {
        float waterH = (bottomY - s.y(300)) * fillLevel;
        float waterTop = bottomY - waterH;

        gs::drawFilledRect(win, s.x(55) + inset, bottomY + inset,
                           s.x(760) - inset, bottomY + pipeW - inset, water);
        for (auto& p : pipes) {
            float wt = std::max(waterTop, p.topY + inset);
            gs::drawFilledRect(win, p.x + inset, wt,
                               p.x + p.width - inset, bottomY - inset, water);
        }
    }

    // Labels
    unsigned fSm = static_cast<unsigned>(20 * s.fx);
    for (auto& p : pipes) {
        gs::drawText(win, gs::Fonts().sans, fSm, p.label,
                     p.x, bottomY + s.y(25), gs::colors::red);
    }
}

// ── Title screen ────────────────────────────────────────────────────────────
static bool titleScreen(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;

    drawBg(win, app.tex.bg, s);

    unsigned fTitle = static_cast<unsigned>(50 * s.fx);
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    gs::drawText(win, f.serif, fTitle, "Frisches Wasser für diese Häuser!",
                 s.x(40), s.y(80), gs::colors::red, true);

    gs::drawText(win, f.sans, fMed, "Hier sind deine Aufgaben:",
                 s.x(270), s.y(180), gs::colors::red);

    gs::drawText(win, f.sans, fMed,
                 "1. Kann Wasser bergauf fließen? (zwei Versuche)",
                 s.x(330), s.y(220), gs::colors::blue);
    gs::drawText(win, f.sans, fMed,
                 "2. Wasserhochbehälter und Brunnen",
                 s.x(330), s.y(260), gs::colors::blue);
    gs::drawText(win, f.sans, fMed,
                 "3. Bau einer Pumpe",
                 s.x(330), s.y(300), gs::colors::blue);
    gs::drawText(win, f.sans, fMed,
                 "4. Frisches Wasser für die Häuser",
                 s.x(330), s.y(340), gs::colors::blue);

    gs::drawText(win, f.sans, fSm,
                 "Hier siehst du tief in die Erde hinein,",
                 s.x(10), s.y(450), gs::colors::creme);
    gs::drawText(win, f.sans, fSm,
                 "    so als wäre sie mit einem riesigen Messer durchgeschnitten.",
                 s.x(10), s.y(480), gs::colors::creme);

    // "Weiter" button
    gs::drawRect(win, s.x(838), s.y(625), s.x(920), s.y(660),
                 gs::colors::creme, s.fx);
    gs::drawText(win, f.sans, fSm, "Weiter", s.x(850), s.y(630), gs::colors::creme);

    gs::drawText(win, f.sans, fSm, "Ende: Taste F8 / Esc",
                 s.x(60), s.y(630), gs::colors::creme);

    win.display();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                if (mx > s.x(838) && mx < s.x(920) && my > s.y(625) && my < s.y(660))
                    return true;
            }
        }
        gs::delayMs(16);
    }
    return false;
}

// ── Task 1: Can water flow uphill? ──────────────────────────────────────────
static bool task1(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    app.waterAmount = 0;
    app.waterLevel = 0.f;

    auto draw = [&]() {
        win.clear(sf::Color(244, 255, 251));
        drawUTube(win, s, app.waterLevel);
        drawWaterPalette(win, s, f, app.waterAmount);

        gs::drawText(win, f.sans, fMed,
                     "Kann Wasser ohne eine Pumpe bergauf fließen?",
                     s.x(175), s.y(155), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Wähle die Wassermenge oben in der Wasserleiste aus!",
                     s.x(175), s.y(195), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Das Wasser steht in beiden Hälften des Rohres gleich hoch.",
                     s.x(175), s.y(235), gs::colors::red);

        // Buttons
        gs::drawRect(win, s.x(770), s.y(605), s.x(915), s.y(635),
                     gs::colors::blue, s.fx);
        gs::drawText(win, f.sans, fSm, "Weiter", s.x(780), s.y(610), gs::colors::blue);

        gs::drawRect(win, s.x(770), s.y(645), s.x(915), s.y(675),
                     gs::colors::blue, s.fx);
        gs::drawText(win, f.sans, fSm, "Ich hab's verstanden!", s.x(780), s.y(650),
                     gs::colors::red);

        gs::drawText(win, f.sans, fSm, "F8 / Esc = Ende",
                     s.x(60), s.y(630), gs::colors::blue);
        win.display();
    };
    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);

                // Water palette click
                float barX = s.x(175), barY = s.y(60), barW = s.x(17), barH = s.y(35);
                if (my >= barY && my <= barY + barH &&
                    mx >= barX && mx < barX + 45 * barW) {
                    int idx = static_cast<int>((mx - barX) / barW) + 1;
                    app.waterAmount = std::clamp(idx, 1, 45);
                    // Animate water filling
                    app.waterLevel = static_cast<float>(app.waterAmount) / 45.f;
                    needDraw = true;
                }

                // "Weiter" or "Ich hab's verstanden"
                if (mx > s.x(770) && mx < s.x(915)) {
                    if (my > s.y(605) && my < s.y(635)) {
                        // Show explanation
                        gs::alert(win, f, s,
                                  "Das Wasser steht in beiden Hälften gleich hoch.",
                                  {"OK"});
                        needDraw = true;
                    }
                    if (my > s.y(645) && my < s.y(675))
                        return true;
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return false;
}

// ── Task 2: Five pipes (communicating vessels) ──────────────────────────────
static bool task2(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    app.waterAmount = 0;
    app.waterLevel = 0.f;

    auto draw = [&]() {
        win.clear(sf::Color(244, 255, 251));
        drawFivePipes(win, s, app.waterLevel);
        drawWaterPalette(win, s, f, app.waterAmount);

        gs::drawText(win, f.sans, fMed,
                     "Im zweiten Versuch kannst du Wasser in fünf verschiedene Rohre fließen lassen.",
                     s.x(175), s.y(145), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Das Wasser steht am Ende in allen Rohren gleich hoch!",
                     s.x(175), s.y(185), gs::colors::red);
        gs::drawText(win, f.sans, fSm,
                     "Auf die Breite oder Form des Rohres kommt es nicht an.",
                     s.x(175), s.y(210), gs::colors::blue);

        gs::drawRect(win, s.x(770), s.y(650), s.x(915), s.y(680),
                     gs::colors::blue, s.fx);
        gs::drawText(win, f.sans, fSm, "Ich hab's verstanden!", s.x(780), s.y(655),
                     gs::colors::red);
        gs::drawText(win, f.sans, fSm, "F8 / Esc = Ende",
                     s.x(60), s.y(660), gs::colors::blue);
        win.display();
    };
    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);

                float barX = s.x(175), barY = s.y(60), barW = s.x(17), barH = s.y(35);
                if (my >= barY && my <= barY + barH &&
                    mx >= barX && mx < barX + 45 * barW) {
                    int idx = static_cast<int>((mx - barX) / barW) + 1;
                    app.waterAmount = std::clamp(idx, 1, 45);
                    app.waterLevel = static_cast<float>(app.waterAmount) / 45.f;
                    needDraw = true;
                }

                if (mx > s.x(770) && mx < s.x(915) && my > s.y(650) && my < s.y(680))
                    return true;
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return false;
}

// ── Task 3: Place the water tank (Hochbehälter) ─────────────────────────────
static bool task3(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    app.tankPlaced = false;

    auto draw = [&]() {
        win.clear();
        drawBg(win, app.tex.bg, s);

        gs::drawText(win, f.sans, fMed,
                     "Suche jetzt den Platz für einen Wasserbehälter,",
                     s.x(70), s.y(95), gs::colors::blue);
        gs::drawText(win, f.sans, fMed,
                     "von dem aus das Wasser alle Häuser ohne eine Pumpe erreichen kann.",
                     s.x(70), s.y(125), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Der Behälter soll zum größten Teil unter der Erdoberfläche liegen.",
                     s.x(70), s.y(165), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Klicke mit der Maus an eine dafür geeignete Stelle!",
                     s.x(70), s.y(195), gs::colors::blue);

        if (app.tankPlaced) {
            // Draw small water tank
            gs::drawFilledRect(win, app.tankPos.x - s.x(16), app.tankPos.y - s.y(8),
                               app.tankPos.x + s.x(16), app.tankPos.y + s.y(16),
                               sf::Color(80, 80, 80));
            gs::drawFilledRect(win, app.tankPos.x - s.x(13), app.tankPos.y - s.y(5),
                               app.tankPos.x + s.x(13), app.tankPos.y - s.y(1),
                               sf::Color::White);
            gs::drawFilledRect(win, app.tankPos.x - s.x(13), app.tankPos.y,
                               app.tankPos.x + s.x(4), app.tankPos.y + s.y(12),
                               sf::Color(100, 220, 255));
        }

        gs::drawText(win, f.sans, fSm, "F8 / Esc = Ende",
                     s.x(60), s.y(630), gs::colors::creme);
        gs::drawRect(win, s.x(838), s.y(625), s.x(920), s.y(660),
                     gs::colors::creme, s.fx);
        gs::drawText(win, f.sans, fSm, "Weiter", s.x(850), s.y(630), gs::colors::creme);
        win.display();
    };
    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);

                // "Weiter" button
                if (mx > s.x(838) && mx < s.x(920) &&
                    my > s.y(625) && my < s.y(660) && app.tankPlaced)
                    return true;

                // Click on landscape to place tank
                float normY = my / static_cast<float>(s.winH);
                if (normY < 0.25f) {
                    gs::alert(win, f, s,
                              "Soll der Wasserbehälter in der Luft schweben?",
                              {"Nochmal"});
                } else if (normY > 0.62f) {
                    gs::alert(win, f, s,
                              "Soll der Wasserbehälter tief in der Erde vergraben werden?",
                              {"Nochmal"});
                } else if (normY > 0.35f && normY < 0.45f) {
                    app.tankPlaced = true;
                    app.tankPos = {mx, my};
                    gs::alert(win, f, s,
                              "Das ist ein guter Platz für den Wasserbehälter!",
                              {"OK"});
                } else {
                    app.tankPlaced = true;
                    app.tankPos = {mx, my};
                    gs::alert(win, f, s,
                              "Der Platz ist möglich. Ein Platz zwischen Oberfläche und tiefer "
                              "Erde wäre besser.",
                              {"Nochmal"});
                }
                needDraw = true;
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return false;
}

// ── Task 4: Place the well (Brunnen) ────────────────────────────────────────
static bool task4(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    app.wellPlaced = false;

    auto draw = [&]() {
        win.clear();
        drawBg(win, app.tex.bg, s);

        // Draw water tank from previous task
        if (app.tankPlaced) {
            gs::drawFilledRect(win, app.tankPos.x - s.x(16), app.tankPos.y - s.y(8),
                               app.tankPos.x + s.x(16), app.tankPos.y + s.y(16),
                               sf::Color(80, 80, 80));
        }

        gs::drawText(win, f.sans, fMed,
                     "Suche jetzt den besten Platz für einen Brunnen.",
                     s.x(70), s.y(120), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Dafür soll ein Loch von der Erdoberfläche bis zum Grundwasser gebohrt werden.",
                     s.x(70), s.y(155), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Klicke an eine Stelle, von der aus das Grundwasser leicht zu erreichen ist!",
                     s.x(70), s.y(185), gs::colors::blue);

        // Underground labels
        gs::drawText(win, f.sans, fSm,
                     "Diese Erdschicht kann sich mit Regenwasser voll saugen wie ein Schwamm.",
                     s.x(10), s.y(460), gs::colors::creme);
        gs::drawText(win, f.sans, static_cast<unsigned>(35 * s.fx),
                     "Grundwasser", s.x(400), s.y(580), sf::Color(60, 230, 255));

        if (app.wellPlaced) {
            // Draw well shaft
            gs::drawFilledRect(win, app.wellPos.x - s.x(4), app.wellPos.y,
                               app.wellPos.x + s.x(4), app.wellPos.y + s.y(120),
                               sf::Color::White);
            gs::drawRect(win, app.wellPos.x - s.x(5), app.wellPos.y,
                         app.wellPos.x + s.x(5), app.wellPos.y + s.y(120),
                         sf::Color(100, 30, 30), s.fx);
            // Water at bottom
            gs::drawFilledRect(win, app.wellPos.x - s.x(3), app.wellPos.y + s.y(80),
                               app.wellPos.x + s.x(3), app.wellPos.y + s.y(120),
                               sf::Color(100, 220, 255));
        }

        gs::drawText(win, f.sans, fSm, "F8 / Esc = Ende",
                     s.x(60), s.y(630), gs::colors::creme);
        gs::drawRect(win, s.x(838), s.y(625), s.x(920), s.y(660),
                     gs::colors::creme, s.fx);
        gs::drawText(win, f.sans, fSm, "Weiter", s.x(850), s.y(630), gs::colors::creme);
        win.display();
    };
    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);

                if (mx > s.x(838) && mx < s.x(920) &&
                    my > s.y(625) && my < s.y(660) && app.wellPlaced)
                    return true;

                float normY = my / static_cast<float>(s.winH);
                if (normY > 0.55f && normY < 0.65f) {
                    app.wellPlaced = true;
                    app.wellPos = {mx, my};
                    gs::alert(win, f, s,
                              "Du hast eine gute Stelle gefunden! Der Brunnen kann gebohrt werden.",
                              {"OK"});
                } else if (normY < 0.4f) {
                    gs::alert(win, f, s,
                              "Diese Stelle ist nicht geeignet. Lies die Aufgabe noch einmal durch!",
                              {"Nochmal"});
                } else {
                    app.wellPlaced = true;
                    app.wellPos = {mx, my};
                    gs::alert(win, f, s,
                              "Der Brunnen müsste tiefer als notwendig gebohrt werden. Suche eine "
                              "bessere Stelle!",
                              {"Nochmal"});
                }
                needDraw = true;
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return false;
}

// ── Task 5: Build and operate a pump ────────────────────────────────────────
static bool task5(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    app.pistonUp = false;
    app.pumpFilled = false;
    app.pumpCycles = 0;
    float tankFill = 0.f;

    auto draw = [&]() {
        win.clear(sf::Color(244, 255, 251));

        // Water tank (top left)
        gs::drawFilledRect(win, s.x(40), s.y(40), s.x(260), s.y(180),
                           sf::Color(0, 80, 160));
        gs::drawFilledRect(win, s.x(50), s.y(40), s.x(250), s.y(170),
                           sf::Color::Black);
        gs::drawFilledRect(win, s.x(54), s.y(40), s.x(246), s.y(166),
                           sf::Color::White);
        // Water in tank
        if (tankFill > 0.f) {
            float waterTop = s.y(166) - tankFill * s.y(120);
            gs::drawFilledRect(win, s.x(54), waterTop, s.x(246), s.y(166),
                               sf::Color(100, 220, 255));
        }
        gs::drawText(win, f.sans, fSm, "Wasserhochbehälter",
                     s.x(75), s.y(190), gs::colors::red);

        // Cylinder (center)
        float cylX = s.x(460), cylY = s.y(210), cylW = s.x(65), cylH = s.y(180);
        gs::drawFilledRect(win, cylX, cylY, cylX + cylW, cylY + cylH,
                           sf::Color(0, 80, 160));
        gs::drawFilledRect(win, cylX + s.x(4), cylY + s.y(4),
                           cylX + cylW - s.x(4), cylY + cylH - s.y(4),
                           sf::Color::White);

        // Piston
        float pistonY = app.pistonUp ? cylY + s.y(10) : cylY + cylH - s.y(30);
        gs::drawFilledRect(win, cylX + s.x(4), pistonY,
                           cylX + cylW - s.x(4), pistonY + s.y(16),
                           sf::Color(160, 50, 50));
        // Piston rod
        gs::drawFilledRect(win, cylX + s.x(26), s.y(180),
                           cylX + s.x(32), pistonY,
                           sf::Color(130, 130, 130));

        // Water in cylinder
        if (app.pumpFilled) {
            float waterTop = app.pistonUp ? pistonY + s.y(16) : pistonY + s.y(16);
            gs::drawFilledRect(win, cylX + s.x(4), waterTop,
                               cylX + cylW - s.x(4), cylY + cylH - s.y(4),
                               sf::Color(100, 220, 255));
        }

        // Well (bottom right)
        gs::drawFilledRect(win, s.x(380), s.y(560), s.x(600), s.y(700),
                           sf::Color(0, 80, 160));
        gs::drawFilledRect(win, s.x(390), s.y(560), s.x(590), s.y(700),
                           sf::Color(100, 220, 255));
        gs::drawText(win, f.sans, fSm, "Brunnen", s.x(610), s.y(600), gs::colors::red);

        // Pump arrows
        sf::Color arrowC = gs::colors::red;
        // Up arrow
        gs::drawFilledRect(win, cylX + cylW + s.x(10), cylY + s.y(20),
                           cylX + cylW + s.x(30), cylY + s.y(50), arrowC);
        gs::drawText(win, f.sans, fMed, "▲",
                     cylX + cylW + s.x(12), cylY + s.y(18), arrowC);
        // Down arrow
        gs::drawFilledRect(win, cylX + cylW + s.x(10), cylY + s.y(70),
                           cylX + cylW + s.x(30), cylY + s.y(100), arrowC);
        gs::drawText(win, f.sans, fMed, "▼",
                     cylX + cylW + s.x(12), cylY + s.y(68), arrowC);

        // Pipes connecting parts (simplified)
        gs::drawLine(win, cylX + s.x(30), cylY + cylH,
                     cylX + s.x(30), s.y(560), sf::Color(0, 80, 160), s.x(10));
        gs::drawLine(win, cylX + s.x(30), s.y(170),
                     s.x(246), s.y(170), sf::Color(0, 80, 160), s.x(10));

        // Instructions
        gs::drawText(win, f.sans, fMed,
                     "Deine Aufgabe: Schaffe mit der Pumpe Wasser",
                     s.x(560), s.y(30), gs::colors::blue);
        gs::drawText(win, f.sans, fMed,
                     "aus dem Brunnen in den Hochbehälter!",
                     s.x(560), s.y(60), gs::colors::blue);
        gs::drawText(win, f.sans, fSm,
                     "Klicke auf die roten Pfeile zum Bewegen des Kolbens.",
                     s.x(560), s.y(100), gs::colors::blue);

        std::string status = "Pumpzyklen: " + std::to_string(app.pumpCycles);
        gs::drawText(win, f.sans, fSm, status, s.x(40), s.y(720), gs::colors::blue);

        gs::drawText(win, f.sans, fSm, "F8 / Esc = Ende",
                     s.x(60), s.y(750), gs::colors::blue);
        gs::drawRect(win, s.x(838), s.y(745), s.x(920), s.y(775),
                     gs::colors::blue, s.fx);
        gs::drawText(win, f.sans, fSm, "Weiter", s.x(850), s.y(750), gs::colors::blue);

        if (tankFill >= 1.0f) {
            gs::drawText(win, f.serif, static_cast<unsigned>(40 * s.fx),
                         "Sehr gut: Der Hochbehälter ist gefüllt!",
                         s.x(150), s.y(420), gs::colors::red, true);
        }

        win.display();
    };
    draw();

    while (win.isOpen()) {
        bool needDraw = false;
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);

                float cylX2 = s.x(460), cylW2 = s.x(65), cylY2 = s.y(210);

                // Up arrow
                if (mx > cylX2 + cylW2 + s.x(10) && mx < cylX2 + cylW2 + s.x(30) &&
                    my > cylY2 + s.y(20) && my < cylY2 + s.y(50)) {
                    if (!app.pistonUp) {
                        app.pistonUp = true;
                        app.pumpFilled = true;
                        needDraw = true;
                    }
                }
                // Down arrow
                if (mx > cylX2 + cylW2 + s.x(10) && mx < cylX2 + cylW2 + s.x(30) &&
                    my > cylY2 + s.y(70) && my < cylY2 + s.y(100)) {
                    if (app.pistonUp) {
                        app.pistonUp = false;
                        if (app.pumpFilled) {
                            app.pumpCycles++;
                            tankFill = std::min(1.0f, tankFill + 0.15f);
                            app.pumpFilled = false;
                        }
                        needDraw = true;
                    }
                }

                // "Weiter" button
                if (mx > s.x(838) && mx < s.x(920) &&
                    my > s.y(745) && my < s.y(775) && tankFill >= 0.5f)
                    return true;
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return false;
}

// ── Task 6: Water pipeline complete ─────────────────────────────────────────
static bool task6(App& app) {
    auto& win = *app.win;
    auto& s = app.sc;
    auto& f = app.fonts;
    unsigned fTitle = static_cast<unsigned>(40 * s.fx);
    unsigned fMed = static_cast<unsigned>(22 * s.fx);
    unsigned fSm = static_cast<unsigned>(17 * s.fx);

    win.clear();
    drawBg(win, app.tex.bg, s);

    gs::drawText(win, f.serif, fTitle,
                 "Frisches Wasser für alle Häuser!",
                 s.x(120), s.y(80), gs::colors::blue, true);
    gs::drawText(win, f.sans, fMed,
                 "Sehr gut! Du hast alle Aufgaben gelöst.",
                 s.x(150), s.y(180), gs::colors::red);
    gs::drawText(win, f.sans, fMed,
                 "Die Wasserleitung versorgt jetzt alle Häuser mit frischem Wasser.",
                 s.x(100), s.y(220), gs::colors::blue);
    gs::drawText(win, f.sans, fSm,
                 "Das Wasser wird aus einem Brunnen gepumpt, in dem Hochbehälter gespeichert",
                 s.x(100), s.y(280), gs::colors::blue);
    gs::drawText(win, f.sans, fSm,
                 "und fließt dann durch die Leitungen zu den Häusern.",
                 s.x(100), s.y(310), gs::colors::blue);

    // Draw water pipes on landscape
    gs::drawLine(win, s.x(60), s.y(290), s.x(60), s.y(500),
                 sf::Color(100, 220, 255), s.x(6));
    gs::drawLine(win, s.x(60), s.y(500), s.x(900), s.y(500),
                 sf::Color(100, 220, 255), s.x(6));

    gs::drawRect(win, s.x(400), s.y(600), s.x(550), s.y(660),
                 gs::colors::creme, s.fx * 2);
    gs::drawText(win, f.sans, fMed, "Ende / Neustart", s.x(410), s.y(615), gs::colors::creme);

    win.display();

    while (win.isOpen()) {
        while (auto event = win.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { win.close(); return false; }
            if (auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F8 ||
                    kp->code == sf::Keyboard::Key::Escape) return false;
            }
            if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                float mx = static_cast<float>(mp->position.x);
                float my = static_cast<float>(mp->position.y);
                if (mx > s.x(400) && mx < s.x(550) && my > s.y(600) && my < s.y(660))
                    return true;
            }
        }
        gs::delayMs(16);
    }
    return false;
}

// ── Main ────────────────────────────────────────────────────────────────────
int main() {
    const unsigned W = 1000, H = 750;
    sf::RenderWindow window(sf::VideoMode({W, H}),
                            "Frisches Wasser für diese Häuser!");
    window.setFramerateLimit(30);

    App app;
    app.win = &window;
    if (!app.fonts.load()) return 1;
    app.sc.init(1152, 864, W, H);
    app.tex.load();

    while (window.isOpen()) {
        if (!titleScreen(app)) break;
        if (!task1(app)) break;
        if (!task2(app)) break;
        if (!task3(app)) break;
        if (!task4(app)) break;
        if (!task5(app)) break;
        if (!task6(app)) break;
        // Loop back to title
    }
    return 0;
}