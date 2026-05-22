#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <functional>

namespace gs {

// ---------------------------------------------------------------------------
// Resource path helpers
// ---------------------------------------------------------------------------
// Returns the directory that contains the running executable's companion
// data (Bild/, Texte/, …).  All asset loading goes through this so that
// the programs work no matter where they are started from.
std::filesystem::path dataDir();

// ---------------------------------------------------------------------------
// Font management – replaces GFA-Basic's Font … To schriftN%
// ---------------------------------------------------------------------------
// The original programs used Arial, Times New Roman, Courier New and
// Wingdings.  On Linux we substitute DejaVu / Nimbus equivalents.
// A single Fonts object is shared per application.
struct Fonts {
    sf::Font sans;       // ≈ Arial
    sf::Font serif;      // ≈ Times New Roman
    sf::Font mono;       // ≈ Courier New
    bool load();         // returns false if a required font is missing
};

// ---------------------------------------------------------------------------
// Scaled coordinate system
// ---------------------------------------------------------------------------
// GFA-Basic programs designed for 640×480 used fx = _X/640, fy = _Y/480.
// We keep the same approach: the virtual canvas is 640×480 (or 1152×864
// for Frisches_Wasser) and we compute a global scale factor.
struct ScaleFactors {
    float fx = 1.f;
    float fy = 1.f;
    unsigned winW = 640;
    unsigned winH = 480;
    void init(unsigned baseW, unsigned baseH, unsigned actualW, unsigned actualH);
    float x(float v) const { return v * fx; }
    float y(float v) const { return v * fy; }
};

// ---------------------------------------------------------------------------
// Color constants used throughout the programs
// ---------------------------------------------------------------------------
namespace colors {
    inline const sf::Color creme     {255, 255, 198};
    inline const sf::Color darkBrown {100, 0, 0};
    inline const sf::Color veryDark  {75, 0, 0};
    inline const sf::Color red       {255, 0, 0};
    inline const sf::Color blue      {0, 0, 235};
    inline const sf::Color white     {255, 255, 255};
    inline const sf::Color black     {0, 0, 0};
    inline const sf::Color lightBlue {190, 225, 255};
    inline const sf::Color cyan      {0, 190, 255};
    inline const sf::Color green     {0, 255, 0};
    inline const sf::Color grey      {160, 160, 160};
    inline const sf::Color bgDark    {30, 0, 0};
}

// ---------------------------------------------------------------------------
// Drawing helpers – map GFA-Basic primitives to SFML
// ---------------------------------------------------------------------------
void drawFilledRect(sf::RenderTarget& rt, float x1, float y1,
                    float x2, float y2, sf::Color fill);
void drawRect(sf::RenderTarget& rt, float x1, float y1,
              float x2, float y2, sf::Color outline, float thickness = 1.f);
void drawRoundedRect(sf::RenderTarget& rt, float x1, float y1,
                     float x2, float y2, sf::Color outline, float thickness,
                     sf::Color fill = sf::Color::Transparent);
void drawLine(sf::RenderTarget& rt, float x1, float y1,
              float x2, float y2, sf::Color color, float thickness = 1.f);
void drawText(sf::RenderTarget& rt, const sf::Font& font, unsigned size,
              const std::string& str, float x, float y, sf::Color color,
              bool bold = false);
// Center text horizontally in window
void drawTextCentered(sf::RenderTarget& rt, const sf::Font& font, unsigned size,
                      const std::string& str, float y, sf::Color color,
                      bool bold = false);
float textWidth(const sf::Font& font, unsigned size, const std::string& str);

// ---------------------------------------------------------------------------
// Texture loading – handles path separators / capitalisation mismatches
// ---------------------------------------------------------------------------
bool loadTexture(sf::Texture& tex, const std::string& relativePath);
bool loadImage(sf::Image& img, const std::string& relativePath);

// ---------------------------------------------------------------------------
// Decorative frame used by most apps (the red border + creme interior)
// ---------------------------------------------------------------------------
void drawFrame(sf::RenderTarget& rt, const ScaleFactors& s);

// ---------------------------------------------------------------------------
// Name-entry dialog (common to Rechnen, Schreiben, Lesen, Schiebepuzzle)
// ---------------------------------------------------------------------------
std::string nameEntryDialog(sf::RenderWindow& win, const Fonts& fonts,
                            const ScaleFactors& s, const std::string& verb);

// ---------------------------------------------------------------------------
// Alert / confirmation dialogs
// ---------------------------------------------------------------------------
// Returns 1-based index of the button pressed.
int alert(sf::RenderWindow& win, const Fonts& fonts, const ScaleFactors& s,
          const std::string& message,
          const std::vector<std::string>& buttons,
          const sf::Texture* smiley = nullptr);

// ---------------------------------------------------------------------------
// Progress bars ("Säulen") for correct/wrong answers
// ---------------------------------------------------------------------------
void drawProgressBars(sf::RenderTarget& rt, const ScaleFactors& s,
                      int correct, int wrong,
                      const sf::Texture* smileyHappy,
                      const sf::Texture* smileySad,
                      bool lastWasError);

// ---------------------------------------------------------------------------
// Timer helper (milliseconds since epoch, like oTimer in GFA-Basic)
// ---------------------------------------------------------------------------
int64_t timerMs();
void delayMs(int64_t ms);

// ---------------------------------------------------------------------------
// Menu system used by Rechnen, Schreiben, Lesen
// ---------------------------------------------------------------------------
struct MenuItem {
    std::string label;
    sf::FloatRect bounds;   // in window coordinates
};

// Draw a grid-based selection menu and return the 1-based index picked
// when the user right-clicks (0 = F8/escape pressed).
int gridMenu(sf::RenderWindow& win, const Fonts& fonts, const ScaleFactors& s,
             const std::string& title,
             const std::vector<std::string>& items,
             const std::string& playerName);

// ---------------------------------------------------------------------------
// Random number generation
// ---------------------------------------------------------------------------
int randInt(int lo, int hi);  // inclusive on both ends

} // namespace gs