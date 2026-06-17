#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>

const int WIDTH = 1200, HEIGHT = 800;
const int SEGMENTS = 40;
const float SEG_LEN = 12.f;
const float GRAVITY = 980.f;
const int ITERATIONS = 20;

struct Point {
    float x, y, px, py;
    bool pinned;
};

struct Rope {
    std::vector<Point> pts;
    sf::Color col;
    bool cut = false;

    void init(float sx, float sy, int n, float segLen, bool pinEnd = false) {
        pts.clear();
        for (int i = 0; i < n; i++) {
            pts.push_back({sx + i * segLen, sy, sx + i * segLen, sy, i == 0});
        }
        if (pinEnd) pts.back().pinned = true;
    }

    void update(float dt) {
        if (cut) return;
        for (auto& p : pts) {
            if (p.pinned) continue;
            float vx = p.x - p.px, vy = p.y - p.py;
            p.px = p.x; p.py = p.y;
            p.x += vx * 0.98f;
            p.y += vy * 0.98f + GRAVITY * dt * dt;
            if (p.y > HEIGHT - 10) { p.y = HEIGHT - 10; p.py = p.y + vy * 0.3f; }
            if (p.x < 0) { p.x = 0; } if (p.x > WIDTH) { p.x = WIDTH; }
        }
        for (int iter = 0; iter < ITERATIONS; iter++) {
            for (int i = 0; i < (int)pts.size() - 1; i++) {
                auto& a = pts[i]; auto& b = pts[i + 1];
                float dx = b.x - a.x, dy = b.y - a.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist < 0.001f) continue;
                float diff = (dist - SEG_LEN) / dist * 0.5f;
                float ox = dx * diff, oy = dy * diff;
                if (!a.pinned) { a.x += ox; a.y += oy; }
                if (!b.pinned) { b.x -= ox; b.y -= oy; }
            }
        }
    }

    void draw(sf::RenderWindow& win, float thick = 3.f) {
        if (pts.size() < 2) return;
        for (int i = 0; i < (int)pts.size() - 1; i++) {
            float dx = pts[i+1].x - pts[i].x;
            float dy = pts[i+1].y - pts[i].y;
            float len = std::sqrt(dx*dx + dy*dy);
            if (len < 0.001f) continue;
            sf::RectangleShape seg({len, thick});
            seg.setOrigin({0.f, thick/2.f});
            seg.setPosition({pts[i].x, pts[i].y});
            seg.setRotation(sf::degrees(std::atan2(dy, dx) * 180.f / 3.14159f));
            // Color based on tension
            float tension = len / SEG_LEN;
            uint8_t r = (uint8_t)std::min(255.f, col.r * tension);
            uint8_t g = (uint8_t)(col.g / tension);
            seg.setFillColor(sf::Color(r, g, col.b));
            win.draw(seg);
        }
        // Draw pin circles
        for (auto& p : pts) {
            if (p.pinned) {
                sf::CircleShape pin(6.f);
                pin.setOrigin({6.f, 6.f});
                pin.setPosition({p.x, p.y});
                pin.setFillColor(sf::Color(220, 220, 220));
                win.draw(pin);
            }
        }
    }

    // Returns index of nearest point within radius
    int nearest(float mx, float my, float radius = 20.f) {
        int best = -1; float bestD = radius * radius;
        for (int i = 0; i < (int)pts.size(); i++) {
            float dx = pts[i].x - mx, dy = pts[i].y - my;
            float d = dx*dx + dy*dy;
            if (d < bestD) { bestD = d; best = i; }
        }
        return best;
    }
};

int main() {
    sf::RenderWindow window(
        sf::VideoMode({(unsigned)WIDTH, (unsigned)HEIGHT}),
        "Rope Simulation | LMB: Drag | RMB: Cut | Space: Reset | W: Wind | G: Gravity toggle"
    );
    window.setFramerateLimit(60);
    srand(42);

    std::vector<Rope> ropes;

    auto resetRopes = [&]() {
        ropes.clear();
        // Rope 1 — horizontal hanging
        Rope r1; r1.col = sf::Color(220, 180, 100);
        r1.init(200, 100, SEGMENTS, SEG_LEN, false);
        ropes.push_back(r1);

        // Rope 2 — pinned both ends
        Rope r2; r2.col = sf::Color(100, 200, 150);
        r2.init(400, 80, SEGMENTS, SEG_LEN, true);
        ropes.push_back(r2);

        // Rope 3 — diagonal
        Rope r3; r3.col = sf::Color(180, 100, 220);
        r3.init(700, 60, 30, SEG_LEN, false);
        ropes.push_back(r3);

        // Rope 4 — short pendulum style
        Rope r4; r4.col = sf::Color(255, 120, 80);
        r4.init(1000, 50, 20, SEG_LEN, false);
        ropes.push_back(r4);
    };

    resetRopes();

    int dragRope = -1, dragPt = -1;
    bool windOn = false;
    float windForce = 0.f;
    float gravity = GRAVITY;
    bool lowGrav = false;

    sf::Font font;
    bool hf = font.openFromFile("C:/Windows/Fonts/arial.ttf");
    sf::Text hud(font);
    hud.setCharacterSize(14);
    hud.setFillColor(sf::Color::White);
    hud.setOutlineColor(sf::Color::Black);
    hud.setOutlineThickness(1.5f);
    hud.setPosition({8.f, 8.f});

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = std::min(clock.restart().asSeconds(), 0.02f);
        auto mp = sf::Mouse::getPosition(window);

        windForce = windOn ? 150.f * std::sin(clock.getElapsedTime().asSeconds() * 0.8f) : 0.f;

        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();
            if (auto* k = ev->getIf<sf::Event::KeyPressed>()) {
                if (k->code == sf::Keyboard::Key::Escape) window.close();
                if (k->code == sf::Keyboard::Key::Space) resetRopes();
                if (k->code == sf::Keyboard::Key::W) windOn = !windOn;
                if (k->code == sf::Keyboard::Key::G) {
                    lowGrav = !lowGrav;
                }
                // Pin/unpin nearest point with P
                if (k->code == sf::Keyboard::Key::P) {
                    for (auto& r : ropes) {
                        int idx = r.nearest((float)mp.x, (float)mp.y, 30.f);
                        if (idx >= 0) { r.pts[idx].pinned = !r.pts[idx].pinned; break; }
                    }
                }
            }
            if (auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    for (int ri = 0; ri < (int)ropes.size(); ri++) {
                        int idx = ropes[ri].nearest((float)mp.x, (float)mp.y, 25.f);
                        if (idx >= 0) { dragRope = ri; dragPt = idx; break; }
                    }
                }
                if (mb->button == sf::Mouse::Button::Right) {
                    // Cut rope at nearest segment
                    for (auto& r : ropes) {
                        for (int i = 1; i < (int)r.pts.size() - 1; i++) {
                            float mx2 = (r.pts[i].x + r.pts[i-1].x) * 0.5f;
                            float my2 = (r.pts[i].y + r.pts[i-1].y) * 0.5f;
                            float dx = mx2 - mp.x, dy = my2 - mp.y;
                            if (dx*dx + dy*dy < 400.f) {
                                // Split: unpin from here
                                for (int j = i; j < (int)r.pts.size(); j++)
                                    r.pts[j].pinned = false;
                                goto cutDone;
                            }
                        }
                    }
                    cutDone:;
                }
            }
            if (auto* mb = ev->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Left) { dragRope = -1; dragPt = -1; }
            }
        }

        // Drag
        if (dragRope >= 0 && dragPt >= 0) {
            ropes[dragRope].pts[dragPt].x = (float)mp.x;
            ropes[dragRope].pts[dragPt].y = (float)mp.y;
            ropes[dragRope].pts[dragPt].px = (float)mp.x;
            ropes[dragRope].pts[dragPt].py = (float)mp.y;
        }

        float grav = lowGrav ? GRAVITY * 0.15f : GRAVITY;

        // Apply wind
        for (auto& r : ropes) {
            for (auto& p : r.pts) {
                if (!p.pinned) p.x += windForce * dt;
            }
            r.update(dt);
            // Override gravity constant — patch via direct force
            if (lowGrav) {
                for (auto& p : r.pts) {
                    if (!p.pinned) {
                        float vy = p.y - p.py;
                        p.y -= vy * 0.4f; // reduce accumulated gravity
                    }
                }
            }
        }

        window.clear(sf::Color(15, 18, 28));

        // Draw background grid
        sf::VertexArray grid(sf::PrimitiveType::Lines);
        for (int x = 0; x < WIDTH; x += 60) {
            grid.append(sf::Vertex{sf::Vector2f((float)x, 0), sf::Color(25,30,45)});
            grid.append(sf::Vertex{sf::Vector2f((float)x, (float)HEIGHT), sf::Color(25,30,45)});
        }
        for (int y = 0; y < HEIGHT; y += 60) {
            grid.append(sf::Vertex{sf::Vector2f(0, (float)y), sf::Color(25,30,45)});
            grid.append(sf::Vertex{sf::Vector2f((float)WIDTH, (float)y), sf::Color(25,30,45)});
        }
        window.draw(grid);

        for (auto& r : ropes) r.draw(window);

        // Wind indicator
        if (windOn) {
            sf::RectangleShape windBar({std::abs(windForce) * 0.5f, 8.f});
            windBar.setPosition({(float)WIDTH/2.f, (float)HEIGHT - 30.f});
            windBar.setFillColor(sf::Color(100, 180, 255, 150));
            window.draw(windBar);
        }

        if (hf) {
            std::string info = "LMB: Drag rope  |  RMB: Cut rope  |  P: Pin/Unpin  |  W: Wind(" +
                std::string(windOn?"ON":"OFF") + ")  |  G: LowGrav(" +
                std::string(lowGrav?"ON":"OFF") + ")  |  Space: Reset";
            hud.setString(info);
            window.draw(hud);
        }

        window.display();
    }
    return 0;
}
