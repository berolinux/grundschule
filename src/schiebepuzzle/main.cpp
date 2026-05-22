// Schiebepuzzle – Sliding tile puzzle
// Ported from GFA-Basic (Schiebepuzzle.g32)
//
// A 5×4 grid of tiles from a loaded image.  One tile is removed (the last
// one) and tiles can be slid into the empty space.

#include "common.h"
#include <numeric>

static const int COLS = 5, ROWS = 4, TOTAL = COLS * ROWS;

int main() {
    const unsigned W = 800, H = 600;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Schiebepuzzle");
    window.setFramerateLimit(30);

    gs::Fonts fonts;
    if (!fonts.load()) return 1;
    gs::ScaleFactors sc;
    sc.init(640, 480, W, H);

    // Load puzzle image
    sf::Texture puzzleTex;
    if (!gs::loadTexture(puzzleTex, "Bild/Puzzle/Zahlen_bis_20.jpg")) return 1;
    sf::Texture grinsTex;
    gs::loadTexture(grinsTex, "Bild/Grins.bmp");

    // Split into tiles
    auto imgSize = puzzleTex.getSize();
    float tileW = static_cast<float>(imgSize.x) / COLS;
    float tileH = static_cast<float>(imgSize.y) / ROWS;

    // Tile positions: 0..19, tile 19 is "empty"
    std::vector<int> board(TOTAL);
    std::iota(board.begin(), board.end(), 0);
    int emptyPos = TOTAL - 1; // position of the empty slot

    // Shuffle by doing random valid moves
    for (int i = 0; i < 500; i++) {
        std::vector<int> neighbours;
        int er = emptyPos / COLS, ec = emptyPos % COLS;
        if (er > 0) neighbours.push_back(emptyPos - COLS);
        if (er < ROWS - 1) neighbours.push_back(emptyPos + COLS);
        if (ec > 0) neighbours.push_back(emptyPos - 1);
        if (ec < COLS - 1) neighbours.push_back(emptyPos + 1);
        int pick = neighbours[gs::randInt(0, static_cast<int>(neighbours.size()) - 1)];
        std::swap(board[emptyPos], board[pick]);
        emptyPos = pick;
    }

    // Drawing layout
    float gridX = sc.x(120), gridY = sc.y(30);
    float dispTW = sc.x(72), dispTH = sc.y(68);
    int moves = 0;
    bool solved = false;
    auto startTime = gs::timerMs();

    auto isSolved = [&]() {
        for (int i = 0; i < TOTAL; i++)
            if (board[i] != i) return false;
        return true;
    };

    auto draw = [&]() {
        window.clear(gs::colors::veryDark);

        // Draw frame
        gs::drawFilledRect(window, sc.x(3), sc.y(3), sc.x(633), sc.y(475), gs::colors::creme);
        gs::drawRect(window, sc.x(6), sc.y(7), sc.x(632), sc.y(473), gs::colors::red, sc.x(6));

        // Reference image (small)
        {
            sf::Sprite ref(puzzleTex);
            ref.setPosition({sc.x(10), sc.y(320)});
            float refScale = sc.x(100) / static_cast<float>(imgSize.x);
            ref.setScale({refScale, refScale});
            window.draw(ref);
        }

        // Puzzle grid
        gs::drawRect(window, gridX - 2, gridY - 2,
                     gridX + COLS * dispTW + 2, gridY + ROWS * dispTH + 2,
                     gs::colors::red, sc.x(3));

        for (int pos = 0; pos < TOTAL; pos++) {
            int tile = board[pos];
            int r = pos / COLS, c = pos % COLS;
            float dx = gridX + c * dispTW;
            float dy = gridY + r * dispTH;

            if (tile == TOTAL - 1 && !solved) {
                // Empty slot
                gs::drawFilledRect(window, dx, dy, dx + dispTW, dy + dispTH,
                                   gs::colors::darkBrown);
            } else {
                // Draw tile from texture
                int tr = tile / COLS, tc = tile % COLS;
                sf::Sprite sp(puzzleTex);
                sp.setTextureRect(sf::IntRect(
                    {static_cast<int>(tc * tileW), static_cast<int>(tr * tileH)},
                    {static_cast<int>(tileW), static_cast<int>(tileH)}));
                sp.setPosition({dx, dy});
                sp.setScale({dispTW / tileW, dispTH / tileH});
                window.draw(sp);
            }
            // Grid lines
            gs::drawRect(window, dx, dy, dx + dispTW, dy + dispTH,
                         sf::Color(30, 30, 120), 1.f);
        }

        // Status
        gs::drawText(window, fonts.sans, static_cast<unsigned>(sc.x(18)),
                     "Züge: " + std::to_string(moves),
                     sc.x(10), sc.y(420), gs::colors::white);

        int64_t elapsed = (gs::timerMs() - startTime) / 1000;
        gs::drawText(window, fonts.sans, static_cast<unsigned>(sc.x(18)),
                     "Zeit: " + std::to_string(elapsed / 60) + " min " +
                     std::to_string(elapsed % 60) + " s",
                     sc.x(10), sc.y(445), gs::colors::white);

        if (solved) {
            gs::drawText(window, fonts.serif, static_cast<unsigned>(sc.x(40)),
                         "Geschafft!", sc.x(200), sc.y(400), gs::colors::red, true);
            sf::Sprite gr(grinsTex);
            gr.setPosition({sc.x(480), sc.y(400)});
            float gsz = sc.x(60) / static_cast<float>(grinsTex.getSize().x);
            gr.setScale({gsz, gsz});
            window.draw(gr);
        }

        gs::drawText(window, fonts.sans, static_cast<unsigned>(sc.x(14)),
                     "Klicke auf ein Feld neben der Lücke!    F8 / Esc = Ende",
                     sc.x(120), sc.y(460), gs::colors::white);

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
                    window.close();
            }
            if (!solved) {
                if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    float mx = static_cast<float>(mp->position.x);
                    float my = static_cast<float>(mp->position.y);
                    if (mx >= gridX && my >= gridY) {
                        int cc = static_cast<int>((mx - gridX) / dispTW);
                        int cr = static_cast<int>((my - gridY) / dispTH);
                        if (cc >= 0 && cc < COLS && cr >= 0 && cr < ROWS) {
                            int clickPos = cr * COLS + cc;
                            // Check if adjacent to empty
                            int er = emptyPos / COLS, ec2 = emptyPos % COLS;
                            if ((cc == ec2 && std::abs(cr - er) == 1) ||
                                (cr == er && std::abs(cc - ec2) == 1)) {
                                std::swap(board[clickPos], board[emptyPos]);
                                emptyPos = clickPos;
                                moves++;
                                if (isSolved()) solved = true;
                                needDraw = true;
                            }
                        }
                    }
                }
            }
        }
        if (needDraw) draw();
        gs::delayMs(16);
    }
    return 0;
}