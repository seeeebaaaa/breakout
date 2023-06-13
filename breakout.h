#pragma once
#include "sound.hpp"

#include "breakout_config.h"
#include "gui.h"
#include "power_ups.hpp"
#include "textures.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <queue>
/**
 *
 * TODO:
 * - Add more power ups:
 *    -> ?
 * - Add boss battle[s]
 *    -> when won, respawn blocks
 * - Ball out of field->live lost sound
 */

struct ball;
class breakout;

struct color {
  float r, g, b;
  color(float r, float g, float b) : r(r), g(g), b(b) {}
  void use(cairo_t *cr) { cairo_set_source_rgb(cr, r, g, b); }
};

struct block {
  cairo_t *cr;
  int x, y, w, h;
  color col;
  block(int x, int y, int w, int h, color col, cairo_t *cr) : x(x), y(y), w(w), h(h), col(col), cr(cr) {}
  virtual void draw(cairo_t *cr, breakout &game);
  void draw_debug(cairo_t *cr);
  void on_collision(breakout &game);
  bool collides(int _x, int _y, float last_good_pos[2], float dir[2], float speed);
  bool hidden = false;
  int lives = BREAKOUT_BLOCK_MAX_LIVES;
  int invincible = false;
  std::vector<std::vector<float>> debug_points;
  std::vector<std::vector<float>> debug_lines;
};

struct ball {
  float x, y, rad = 15;
  float dx, dy, speed = .5;
  void position(float posx, float posy);
  void direction(float dirx, float diry, bool noise = false); // noise adds a bit of randomnes to the direction
  void use_random_start_dir(float scale_x, float scale_y);
  virtual void draw(cairo_t *cr);
  void step(float gamespeed);
  bool check_collision(breakout &game, int time_diff);
  float last_good_x, last_good_y;
  void *last_hit = nullptr;
  float spin_add = 0;
  void tick(int time_diff);
  int life_time = 1000;
  bool main = true;
  bool invincible = false;
  friend std::ostream &operator<<(std::ostream &os, const ball &b);
};

struct board {
  float x, y;
  float w, h, org_w;
  float speed = 0.7;
  color col;
  board(float x, float y, float w, float h) : x(x), y(y), w(w), org_w(w), h(h), col(1, 1, 1) {}
  virtual void draw(cairo_t *cr);
  virtual void move_left(float by);
  virtual void move_right(float by, int width);
  float velocity = 0;
  void tick(int time_diff);
  void on_collision(breakout &game) const;
  bool collides(int _x, int _y, float last_good_pos[2], float dir[2], float speed);
  std::vector<std::vector<float>> debug_points;
  std::vector<std::vector<float>> debug_lines;
  void draw_debug(cairo_t *cr);
};

class breakout : public windowed_app {
  enum game_state { INIT,
                    PLAY,
                    BOSSFIGHT,
                    WAIT,
                    GAMEOVER };
  game_state state = INIT;
  int cells_x = 0, cells_y = 0;
  float space = 0; // percent of block
  int max_space = 5;
  std::vector<block *> blocks;

  // There could be multiple boards and balls -- that's the reason why a std::vector is used here.
  // If you intend to use one ball and/or board in your app you can use balls/boards.front() in your implementation.

  // needs to be ball*, cause otherweise a lot of stuff can break
  std::vector<ball *> balls;
  std::vector<board> boards;
  int lives = 3;

  // SCORE SYSTEM
  float combo = 1; // default is one
  float score = 0;
  int ticks_since_last_combo_hit = 0;
  int combo_duration = 400; // ticks until reset of combo
  void tick_combo(int time_diff);
  int update_score_ticks = 0;
  int update_combo_ticks = 0;
  void draw_score();
  void draw_combo();

  // DEBUG
  void draw_debug_lines();
  void draw_debug_fps();
  int time_diff = 0;
  std::vector<int> last_frame_time;
  void draw_debug_state();

  void debug_keys();

  bool key_hold[256] = {0}; // mimics key_pressed
  bool key_tap[256] = {0};  // is true if key only tapped

  // INIT
  void draw_init();              // draw initial state
  void tick_init(int time_diff); // draw initial state
  float init_hold_score = 0;
  bool help_menu_open = false;
  int help_page = 0;
  float transition_score = 0;
  long ani_score = 0; // 0-10000

  // WAIT
  void prep_wait();
  void draw_wait();
  void tick_wait(int time_diff);
  void move_start_dir(float by);
  float start_angle = 0; // angle from center
  float indicator_length = 100;
  float indicator_color[3] = {0, .5, .5};
  float max_angle[2] = {-60, 60};

  // PLAY
  void prep_play();
  void draw_play();
  void tick_play(int time_diff);
  void restore_game();

  // boss fight
  void prep_fight();
  void tick_fight(int time_diff);
  void tick_fight_intro(int time_diff);
  void tick_fight_fight(int time_diff);
  void tick_fight_death(int time_diff);
  void draw_fight();
  void draw_fight_intro();
  void draw_fight_fight();
  void draw_fight_death();
  enum FIGHT_STAGES { INTRO, // intro sequene, no fighting yet
                      FIGHT, // fighting
                      DEATH  // boss is dead, animation.. go to WAIT
  };
  FIGHT_STAGES current_fight_stage = INTRO;
  long unsigned int anim_timer = 0;

  // powerups
  int interval_time = 200;
  int time_till_next_pu = interval_time * 2;

  /**
   * GAMEOVER:
   * all balles are used and the gained score is showed.
   */

  void prep_gameover(); // load scoreboard
  struct score_struct {
    std::string name;
    int score;
    score_struct(std::string name, int score) : name(name), score(score) {}
  };
  static bool score_compare(score_struct i, score_struct j);
  std::vector<score_struct> hold;
  int player_index = -1;
  bool name_entered = false;
  std::string player_name = "";
  void draw_gameover();
  void tick_gameover(int time_diff);
  long ticks_since_show = 0;
  long max_ticks = 5000;
  int score_ticks = 400;
  bool finished = false;
  float gameover_hold_score = 0;

public:
  breakout(int w, int h);
  ~breakout();
  void draw() override;
  void tick(int time_diff) override;
  int get_width() const { return width; }
  int get_height() const { return height; }
  std::vector<block *> &get_blocks() { return blocks; };
  std::vector<ball *> &get_balls() { return balls; };
  void add_ball(ball *b) { balls.push_back(b); }
  std::vector<board> &get_boards() { return boards; };
  cairo_t *get_cr() { return cr; }

  float game_speed = 0.010;

  // powerups
  powerup::power_up_container *pu_container = nullptr;
  // score
  void add_score(float amount);
  void add_combo(float amount);
  // lives
  void add_lives(int amount) { lives += amount; }
  // textures
  texture_hub tex_hub;
  // font
  FT_Library ft;
  FT_Face ft_face;
  cairo_font_face_t *pixel_font;

  // sounds
  sound::sound_cloud main_cloud;
};
