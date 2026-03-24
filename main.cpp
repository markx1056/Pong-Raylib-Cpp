#include <iostream>
#include <raylib.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

// ─── Color Palette ───────────────────────────────────────────
Color COL_BG_DARK = { 8,   8,  20, 255 };    // Near black
Color COL_BG_LEFT = { 15,  10,  50, 255 };   // Deep violet
Color COL_BG_RIGHT = { 5,  25,  45, 255 };   // Deep teal-navy
Color COL_PADDLE = { 255, 255, 255, 255 };   // Pure white
Color COL_PADDLE_GLOW = { 180, 180, 255,  60 };   // Soft blue glow
Color COL_BALL = { 255, 220,  60, 255 };     // Gold
Color COL_BALL_GLOW = { 255, 180,   0,  50 };     // Orange glow
Color COL_NET = { 255, 255, 255,  30 };      // Dim white
Color COL_SCORE = { 255, 255, 255, 200 };
Color COL_ACCENT = { 100, 200, 255, 255 };   // Cyan accent
Color COL_GOLD = { 255, 215,   0, 255 };
Color COL_SILVER = { 192, 192, 192, 255 };
Color COL_BRONZE = { 205, 127,  50, 255 };
Color COL_BTN_NORMAL = { 30,  30,  60, 255 };
Color COL_BTN_HOVER = { 60,  60, 120, 255 };
Color COL_BTN_SELECTED = { 255, 220,  60, 255 };
Color COL_BTN_TEXT = { 200, 200, 255, 255 };
Color COL_BTN_TEXT_SEL = { 0,   0,   0, 255 };

// ─── Globals ─────────────────────────────────────────────────
int player_score = 0;
int cpu_score = 0;

struct HighScore { int player; int cpu; };
vector<HighScore> high_scores;
const int MAX_HIGH_SCORES = 5;

// WIN CONDITION
const int WINNING_SCORE = 10;

bool  is_flashing = false;
int   flash_frame_count = 0;
float time_elapsed = 0.0f;

// ADDED "game_over" to the state enum
enum game_state { menu, playing, game_over, high_scores_screen, exit_game };
game_state current_state = menu;

const int screen_width = 1280;
const int screen_height = 800;

// ─── High Score IO ───────────────────────────────────────────
void load_high_scores() {
    ifstream file("highscores.txt");
    if (file.is_open()) {
        high_scores.clear();
        int p, c;
        while (file >> p >> c)
            high_scores.push_back({ p, c });
        file.close();
    }
}

void save_high_scores() {
    ofstream file("highscores.txt");
    if (file.is_open()) {
        for (const auto& s : high_scores)
            file << s.player << " " << s.cpu << "\n";
        file.close();
    }
}

void add_high_score(int p, int c) {
    high_scores.push_back({ p, c });
    sort(high_scores.begin(), high_scores.end(), [](const HighScore& a, const HighScore& b) {
        return a.player != b.player ? a.player > b.player : a.cpu < b.cpu;
        });
    if ((int)high_scores.size() > MAX_HIGH_SCORES)
        high_scores.resize(MAX_HIGH_SCORES);
    save_high_scores();
}

bool is_high_score(int p, int c) {
    if ((int)high_scores.size() < MAX_HIGH_SCORES) return true;
    for (const auto& s : high_scores)
        if (p > s.player || (p == s.player && c < s.cpu)) return true;
    return false;
}

// ─── Draw Helpers ────────────────────────────────────────────

void DrawGlowCircle(float x, float y, float r, Color col, float intensity) {
    for (int i = 4; i >= 1; i--) {
        Color c = col;
        c.a = (unsigned char)(col.a * intensity / i);
        DrawCircle((int)x, (int)y, r + i * 5, c);
    }
    DrawCircle((int)x, (int)y, (int)r, col);
}

void DrawGlowRect(float x, float y, float w, float h, Color col) {
    for (int i = 3; i >= 1; i--) {
        Color c = col; c.a = 30 / i;
        DrawRectangleRounded({ x - i * 2, y - i * 2, w + i * 4, h + i * 4 }, 0.4f, 8, c);
    }
    DrawRectangleRounded({ x, y, w, h }, 0.4f, 8, col);
}

void DrawNet() {
    int dash = 18, gap = 10;
    for (int y = 0; y < screen_height; y += dash + gap) {
        DrawRectangle(screen_width / 2 - 1, y, 3, dash, COL_NET);
    }
}

bool DrawButton(const char* text, float cx, float cy, float w, float h, bool selected) {
    float x = cx - w / 2, y = cy - h / 2;
    Color bg = selected ? COL_BTN_SELECTED : COL_BTN_NORMAL;
    Color tcol = selected ? COL_BTN_TEXT_SEL : COL_BTN_TEXT;

    if (selected) {
        Color glow = COL_BTN_SELECTED; glow.a = 40;
        DrawRectangleRounded({ x - 6, y - 6, w + 12, h + 12 }, 0.4f, 8, glow);
    }

    DrawRectangleRoundedLines({ x - 1, y - 1, w + 2, h + 2 }, 0.4f, 8, selected ? COL_BTN_SELECTED : Color{ 60,60,100,255 });
    DrawRectangleRounded({ x, y, w, h }, 0.4f, 8, bg);

    int fs = 28;
    int tw = MeasureText(text, fs);
    DrawText(text, (int)(cx - tw / 2), (int)(cy - fs / 2), fs, tcol);
    return selected;
}

// ─── Ball ────────────────────────────────────────────────────
class ball_class {
public:
    float pos_x, pos_y;
    float velocity_x, velocity_y;
    int   radius;
    float trail_x[8], trail_y[8];
    int   trail_idx = 0;

    void draw() {
        for (int i = 0; i < 8; i++) {
            int idx = (trail_idx - i - 1 + 8) % 8;
            if (trail_x[idx] == 0 && trail_y[idx] == 0) continue;
            Color tc = COL_BALL_GLOW;
            tc.a = (unsigned char)(40 - i * 5);
            DrawCircle((int)trail_x[idx], (int)trail_y[idx], radius - i, tc);
        }
        DrawGlowCircle(pos_x, pos_y, (float)radius, COL_BALL_GLOW, 1.0f);
        DrawCircle((int)pos_x, (int)pos_y, radius, COL_BALL);
        DrawCircle((int)(pos_x - radius * 0.3f), (int)(pos_y - radius * 0.3f), radius / 4, { 255,255,255,180 });
    }

    void update() {
        trail_x[trail_idx] = pos_x;
        trail_y[trail_idx] = pos_y;
        trail_idx = (trail_idx + 1) % 8;

        pos_x += velocity_x;
        pos_y += velocity_y;

        if (pos_y + radius >= screen_height || pos_y - radius <= 0)
            velocity_y *= -1;

        if (pos_x + radius >= screen_width) {
            cpu_score++;
            is_flashing = true; flash_frame_count = 8;
            reset();
        }
        if (pos_x - radius <= 0) {
            player_score++;
            is_flashing = true; flash_frame_count = 8;
            reset();
        }
    }

    void reset() {
        pos_x = screen_width / 2.0f;
        pos_y = screen_height / 2.0f;
        for (int i = 0; i < 8; i++) { trail_x[i] = 0; trail_y[i] = 0; }
        int dirs[2] = { -1, 1 };
        velocity_x = 7.5f * dirs[GetRandomValue(0, 1)];
        velocity_y = 7.5f * dirs[GetRandomValue(0, 1)];
    }
};

// ─── Paddle ──────────────────────────────────────────────────
class paddle_base {
protected:
    void clamp() {
        if (pos_y <= 0)                                   pos_y = 0;
        if (pos_y + height >= screen_height)      pos_y = (float)(screen_height - height);
    }
public:
    float pos_x, pos_y, width, height;
    float speed;
    float visual_y;

    void draw(bool left_side) {
        Color glow = COL_PADDLE_GLOW;
        DrawRectangleRounded({ pos_x - 4, visual_y - 4, width + 8, height + 8 }, 0.5f, 8, glow);
        DrawGlowRect(pos_x, visual_y, width, height, COL_PADDLE);
        DrawRectangleRounded({ pos_x + (left_side ? 3.0f : width - 7.0f), visual_y + 10, 4, height - 20 }, 1.0f, 4, { 255,255,255,60 });
    }

    void update_visual() {
        visual_y += (pos_y - visual_y) * 0.25f;
    }
};

class player_paddle_class : public paddle_base {
public:
    void update() {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))   pos_y -= speed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) pos_y += speed;
        clamp();
        update_visual();
    }
};

class cpu_paddle_class : public paddle_base {
public:
    void update(float ball_y) {
        float center = pos_y + height / 2;
        if (center > ball_y) pos_y -= speed;
        else                 pos_y += speed;
        clamp();
        update_visual();
    }
};

// ─── Game Objects ────────────────────────────────────────────
player_paddle_class player_paddle;
cpu_paddle_class    cpu_paddle_obj;
ball_class          ball;

const char* menu_items[] = { "Start Game", "High Scores", "Exit" };
const int   total_menu = 3;
int         selected_menu = 0;

// ─── Draw Background ─────────────────────────────────────────
void DrawBackground(float t) {
    DrawRectangle(0, 0, screen_width / 2, screen_height, COL_BG_LEFT);
    DrawRectangle(screen_width / 2, 0, screen_width / 2, screen_height, COL_BG_RIGHT);

    float pulse = 0.5f + 0.5f * sinf(t * 1.2f);
    Color gl = { 80, 40, 180, (unsigned char)(18 + (int)(pulse * 12)) };
    DrawCircle(screen_width / 4, screen_height / 2, 320, gl);
    Color gr = { 20, 80, 160, (unsigned char)(18 + (int)(pulse * 12)) };
    DrawCircle(3 * screen_width / 4, screen_height / 2, 320, gr);

    DrawCircleLines(screen_width / 2, screen_height / 2, 150, { 255,255,255,18 });
    DrawCircleLines(screen_width / 2, screen_height / 2, 148, { 255,255,255,  8 });

    DrawNet();
}

// ─── Draw Score ──────────────────────────────────────────────
void DrawScore(int score, int cx, int cy) {
    const char* s = TextFormat("%d", score);
    int fs = 90;
    int tw = MeasureText(s, fs);
    DrawText(s, cx - tw / 2 + 3, cy + 3, fs, { 0,0,0,80 });
    DrawText(s, cx - tw / 2, cy, fs, COL_SCORE);
}

// ─── Main ────────────────────────────────────────────────────
int main() {
    InitWindow(screen_width, screen_height, "Pong — Two Player Edition");
    SetTargetFPS(60);

    // FIX 1: Prevent ESC from instantly closing the window
    SetExitKey(KEY_NULL);

    load_high_scores();

    // Ball
    ball.radius = 14;
    ball.reset();

    // Player paddle (right side)
    player_paddle.width = 18;
    player_paddle.height = 110;
    player_paddle.pos_x = screen_width - 40.0f;
    player_paddle.pos_y = screen_height / 2.0f - 55;
    player_paddle.visual_y = player_paddle.pos_y;
    player_paddle.speed = 7;

    // CPU paddle (left side)
    cpu_paddle_obj.width = 18;
    cpu_paddle_obj.height = 110;
    cpu_paddle_obj.pos_x = 22.0f;
    cpu_paddle_obj.pos_y = screen_height / 2.0f - 55;
    cpu_paddle_obj.visual_y = cpu_paddle_obj.pos_y;
    cpu_paddle_obj.speed = 6;

    while (!WindowShouldClose() && current_state != exit_game) {
        float dt = GetFrameTime();
        time_elapsed += dt;

        BeginDrawing();

        // ── MENU ──────────────────────────────────────────────
        if (current_state == menu) {
            DrawBackground(time_elapsed);

            // Title
            const char* title = "PONG";
            int tfs = 130, tw = MeasureText(title, tfs);
            DrawText(title, screen_width / 2 - tw / 2 + 4, 84, tfs, { 0,0,0,100 });
            DrawText(title, screen_width / 2 - tw / 2, 80, tfs, COL_GOLD);

            const char* sub = "VS Computer EDITION";
            int sfs = 22, sw = MeasureText(sub, sfs);
            DrawText(sub, screen_width / 2 - sw / 2, 220, sfs, { 200,200,255,160 });

            DrawRectangle(screen_width / 2 - 140, 252, 280, 2, { 100,100,200,80 });

            const char* menu_win_text = TextFormat("FIRST TO 10 WINS", WINNING_SCORE);
            DrawText(menu_win_text, screen_width / 2 - MeasureText(menu_win_text, 20) / 2, 270, 20, { 150, 150, 200, 180 });

            DrawRectangle(screen_width / 2 - 140, 252 + 50, 280, 2, { 100,100,200,80 });

            for (int i = 0; i < total_menu; i++) {
                float cy = 340.0f + i * 90.0f;
                DrawButton(menu_items[i], screen_width / 2.0f, cy + 30 , 320, 58, i == selected_menu);
            }


            DrawText("W / S  or  UP / DOWN  to navigate     ENTER to select",
                screen_width / 2 - MeasureText("W / S  or  UP / DOWN  to navigate     ENTER to select", 18) / 2,
                screen_height - 40, 18, { 150,150,200,120 });

            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                selected_menu = (selected_menu - 1 + total_menu) % total_menu;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                selected_menu = (selected_menu + 1) % total_menu;

            if (IsKeyPressed(KEY_ENTER)) {
                if (selected_menu == 0) {
                    player_score = 0; cpu_score = 0;
                    ball.reset();
                    current_state = playing;
                }
                else if (selected_menu == 1) {
                    current_state = high_scores_screen;
                }
                else {
                    current_state = exit_game;
                }
            }
        }

        // ── HIGH SCORES ───────────────────────────────────────
        else if (current_state == high_scores_screen) {
            DrawBackground(time_elapsed);

            const char* hs_title = "HIGH SCORES";
            int htw = MeasureText(hs_title, 60);
            DrawText(hs_title, screen_width / 2 - htw / 2 + 3, 63, 60, { 0,0,0,100 });
            DrawText(hs_title, screen_width / 2 - htw / 2, 60, 60, COL_GOLD);

            DrawRectangle(screen_width / 2 - 180, 132, 360, 2, { 200,180,60,120 });

            int hx1 = 340, hx2 = 580, hx3 = 820;
            DrawText("RANK", hx1, 165, 26, COL_ACCENT);
            DrawText("PLAYER", hx2, 165, 26, COL_ACCENT);
            DrawText("CPU", hx3, 165, 26, COL_ACCENT);
            DrawRectangle(300, 198, 680, 1, { 100,100,200,80 });

            if (high_scores.empty()) {
                const char* msg = "No records yet — play a game!";
                DrawText(msg, screen_width / 2 - MeasureText(msg, 28) / 2, 360, 28, { 180,180,255,180 });
            }
            else {
                for (int i = 0; i < (int)high_scores.size(); i++) {
                    int y = 218 + i * 68;
                    Color row_bg = { 255,255,255, (unsigned char)(i % 2 == 0 ? 8 : 4) };
                    DrawRectangleRounded({ 295, (float)y - 6, 690, 56 }, 0.2f, 6, row_bg);

                    Color rc = (i == 0) ? COL_GOLD : (i == 1) ? COL_SILVER : (i == 2) ? COL_BRONZE : Color{ 180,180,180,255 };
                    const char* medals[] = { "#1", "#2", "#3", "#4", "#5" };
                    DrawText(medals[i], hx1, y + 8, 30, rc);

                    DrawText(TextFormat("%d", high_scores[i].player), hx2, y + 8, 30, COL_PADDLE);
                    DrawText(TextFormat("%d", high_scores[i].cpu), hx3, y + 8, 30, { 255,160,60,255 });
                }
            }

            DrawButton("Back to Menu", screen_width / 2.0f, 700, 280, 54, true);
            const char* hint = "Press ESC or ENTER to return to menu";
            DrawText(hint, screen_width / 2 - MeasureText(hint, 18) / 2, screen_height - 36, 18, { 150,150,200,120 });

            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
                current_state = menu;
        }

        // ── PLAYING ───────────────────────────────────────────
        else if (current_state == playing) {
            if (is_flashing && flash_frame_count > 0) {
                ClearBackground({ 40, 40, 80, 255 });
                flash_frame_count--;
            }
            else {
                is_flashing = false;
                DrawBackground(time_elapsed);
            }

            // Update
            ball.update();
            player_paddle.update();
            cpu_paddle_obj.update(ball.pos_y);

            // Collision
            Rectangle prect = { player_paddle.pos_x, player_paddle.pos_y, player_paddle.width, player_paddle.height };
            Rectangle crect = { cpu_paddle_obj.pos_x, cpu_paddle_obj.pos_y, cpu_paddle_obj.width, cpu_paddle_obj.height };

            if (CheckCollisionCircleRec({ ball.pos_x, ball.pos_y }, (float)ball.radius, prect) ||
                CheckCollisionCircleRec({ ball.pos_x, ball.pos_y }, (float)ball.radius, crect)) {
                ball.velocity_x *= -1.05f;
                if (ball.velocity_x > 15) ball.velocity_x = 15;
                if (ball.velocity_x < -15) ball.velocity_x = -15;
            }

            // Draw
            ball.draw();
            cpu_paddle_obj.draw(true);
            player_paddle.draw(false);

            DrawScore(cpu_score, screen_width / 4, 30);
            DrawScore(player_score, 3 * screen_width / 4, 30);

            DrawText("CPU", screen_width / 4 - MeasureText("CPU", 16) / 2, screen_height - 30, 16, { 200,200,255,100 });
            DrawText("PLAYER", 3 * screen_width / 4 - MeasureText("PLAYER", 16) / 2, screen_height - 30, 16, { 200,200,255,100 });
            DrawText("ESC = Menu", screen_width / 2 - MeasureText("ESC = Menu", 16) / 2, screen_height - 30, 16, { 200,200,255,60 });

            // FIX 2: Win Condition Check
            if (player_score >= WINNING_SCORE || cpu_score >= WINNING_SCORE) {
                if (is_high_score(player_score, cpu_score)) {
                    add_high_score(player_score, cpu_score);
                }
                current_state = game_over;
            }

            // Quit midway
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (is_high_score(player_score, cpu_score) && (player_score > 0 || cpu_score > 0))
                    add_high_score(player_score, cpu_score);
                current_state = menu;
            }
        }

        // ── GAME OVER ─────────────────────────────────────────
        else if (current_state == game_over) {
            DrawBackground(time_elapsed);

            const char* result_text = (player_score >= WINNING_SCORE) ? "YOU WIN!" : "CPU WINS!";
            Color result_col = (player_score >= WINNING_SCORE) ? COL_GOLD : Color{ 255, 100, 100, 255 };

            int fs = 100;
            int tw = MeasureText(result_text, fs);
            DrawText(result_text, screen_width / 2 - tw / 2 + 4, 154, fs, { 0,0,0,100 });
            DrawText(result_text, screen_width / 2 - tw / 2, 150, fs, result_col);

            DrawText("FINAL SCORE", screen_width / 2 - MeasureText("FINAL SCORE", 30) / 2, 300, 30, COL_ACCENT);
            DrawScore(cpu_score, screen_width / 2 - 120, 360);
            DrawText("-", screen_width / 2 - MeasureText("-", 90) / 2, 360, 90, COL_SCORE);
            DrawScore(player_score, screen_width / 2 + 120, 360);

            DrawButton("Return to Menu", screen_width / 2.0f, 650, 320, 58, true);
            const char* hint = "Press ENTER or ESC to return";
            DrawText(hint, screen_width / 2 - MeasureText(hint, 18) / 2, 720, 18, { 150,150,200,120 });

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                current_state = menu;
            }
        }

        EndDrawing();
    }

    // FIX 3: Catch unsaved scores if the player closes the window via the "X" button
    if (current_state == playing && (player_score > 0 || cpu_score > 0)) {
        if (is_high_score(player_score, cpu_score)) {
            add_high_score(player_score, cpu_score);
        }
    }

    CloseWindow();
    return 0;
}
