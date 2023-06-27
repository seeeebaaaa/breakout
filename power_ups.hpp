#ifndef BREAKOUT_GAME_POWER_UP_H
#define BREAKOUT_GAME_POWER_UP_H
namespace powerup {
class parent;
struct power_up_container;
} // namespace powerup
#include "breakout.h"
class breakout;

#include "breakout.h"
struct ball;
#include "gui.h"
#include <vector>

namespace powerup {

class parent {
protected:
  bool active;
  breakout *game;
  float collision_radius = 15;
  int ticks_since_activation = 0;
  long lifetime = 0;
  ball *affected_ball;

public:
  int x, y; // center
  // virtual parent(breakout *game);
  virtual void draw(cairo_t *cr) = 0;
  virtual void activate(ball *b) = 0;
  virtual void deactivate() = 0;
  bool collides(float lx, float ly, int x, int y);
  virtual void tick(int tick_offset) = 0; // needs to be called every game tick. to e.g. process timings of the powerup
  // virtual ~parent();
  bool mghhh_delete_me_daddy = false;
  virtual ~parent(){};
};

struct slowdown : public parent {
  slowdown(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~slowdown() override;

  float slow_down_factor = 0.5;

  int effect_time = 2000 / slow_down_factor;
};

struct speedup : public parent {
  speedup(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~speedup() override;

  float speedup_factor = 1.8;

  int effect_time = 1000 * speedup_factor;
};

struct black_hole : public parent {
  black_hole(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~black_hole() override;
  enum STATE { IDLE,
               PULL,
               SPIN,
               SHOOT,
               SLOWDOWN };
  STATE state = IDLE;
  float pull_dir[2];
  int spin_time_max = 250;
  int spin_time = spin_time_max;
  float spin_angle;
  int indicator_length = 50;
  float original_speed;
  int max_tries = 60;
};

struct double_ball : public parent {
  double_ball(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~double_ball() override;

  float angle = 0;
};

struct lights_out : public parent {
  lights_out(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~lights_out() override;

  int effect_time = 2000;
};

struct score_add_or_sub : public parent {
  score_add_or_sub(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~score_add_or_sub() override;

  bool add = true;
  int score_diff = 2000 + 3000 * random_number();
};

struct invincible : public parent {
  invincible(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~invincible() override;

  int effect_time = 2000;
};

struct gain_life : public parent {
  gain_life(breakout *game);
  void draw(cairo_t *cr) override;
  void activate(ball *b) override;
  void deactivate() override;
  void tick(int tick_offset) override;
  ~gain_life() override;
};

//

parent *random_powerup(breakout *game);

struct power_up_housing {
  parent *pu;
  int x, y;
};

struct power_up_container {
  // init with gane, power up counts at once, and either nllptr [random] or powerup coords
  power_up_container(breakout *game, int max_powerups, int place_count, int nr_of_places = 0, int places[6][2] = nullptr); // nr of places should be first para of places, instead of the 6, but g++ complains

  void collision(ball *b); // goes through powerups;
  void new_random_powerup();
  void tick(int time_diff);
  void draw(cairo_t *cr);
  void deactivate_all();
  void delete_all();
  int interval_time = 500;
  int ticks_until_next_pu = interval_time * 2;

  ~power_up_container();

private:
  breakout *game;
  uint max_powerups;
  uint place_count;
  std::vector<power_up_housing> house;
};

} // namespace powerup
#endif