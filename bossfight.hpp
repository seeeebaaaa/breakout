#pragma once
namespace bossfight {
struct boss_container;
}

#include "breakout.h"
class breakout;
struct ball;
#include "gui.h"
#include "textures.hpp"
#include <iostream>
#include <vector>

namespace bossfight {
enum BOSSES {
  PHANTOM,
  RANDOM // has to be last element in list
};
enum STAGE {
  PREP_INTRO,
  INTRO,
  WAIT, // start wait for space
  FIGHT,
  DEATH // of boss
};
class boss_parent {
protected:
  long unsigned int lifetime = 0; // ticks since init [needs to be ticked with tick()]
  breakout *game = nullptr;
  STAGE stage = PREP_INTRO;

  virtual void prep_intro() = 0;
  virtual void draw_intro(cairo_t *cr) = 0;
  virtual void tick_intro(int time_diff) = 0;
  virtual void draw_wait(cairo_t *cr) = 0;
  virtual void tick_wait(int time_diff) = 0;
  virtual void draw_fight(cairo_t *cr) = 0;
  virtual void tick_fight(int time_diff) = 0;
  virtual void draw_death(cairo_t *cr) = 0;
  virtual void tick_death(int time_diff) = 0;
  bool play_death_once = false;

  int timeline_eval(std::vector<int> &timeline, int lifetime);      // returns index of timeline, based on ticks [where in timelien are we]
  int timeline_get_ticks(std::vector<int> &timeline, int lifetime); // returns nubmer of ticks since current timeline event started

  float x, y;
  float scale;

  int max_health = 1000;
  int health = max_health;

  int ticks_since_last_ability = 1000;
  int current_ability_lifetime = 0;
  int current_ability = -1;

  bool hit = false;
  float hitbox[2] = {0, 0}; // radius of hitbox

  float alpha_boss = 1;

public:
  // virtual boss_parent(breakout *game);
  void draw(cairo_t *cr);
  void tick(int time_diff);
  virtual void collision(ball *b) = 0;

  bool finished = false; // when everything is done and ready for destruction
};

class phantom : public boss_parent {
  void prep_intro() override;
  int rand_intro[2] = {0, 0};
  void draw_intro(cairo_t *cr) override;
  bool played_warp_1_1 = false;
  bool played_warp_1_2 = false;
  bool played_warp_2_1 = false;
  bool played_warp_2_2 = false;
  bool played_warp_3_1 = false;
  bool played_warp_4_1 = false;
  bool played_warp_4_2 = false;
  void tick_intro(int time_diff) override;
  void draw_wait(cairo_t *cr) override;
  void tick_wait(int time_diff) override;
  void draw_fight(cairo_t *cr) override;
  void tick_fight(int time_diff) override;
  void draw_death(cairo_t *cr) override;
  void tick_death(int time_diff) override;
  float org_y = 0;

  void collision(ball *b) override;

  float scale = 0.4;
  float hitbox[2] = {55, 85}; // radius
  struct cloud {
    int x, y;
    float scale;
    float alpha;
    cloud(int x, int y, float scale, float alpha) : x(x), y(y), scale(scale), alpha(alpha) {}
  };

  // cloud
  int cloud_max = 1000;
  std::vector<cloud> clouds;

  // warp
  int warp_pos[2] = {0, 0};
  bool played_once = false;

  // focus
  bool ball_in_focus = 0;
  float col[3] = {0, 0, 0};
  float org_speed = 0;
  float multiplier = 0;
  int multi_steps = 0;
  float dir[2] = {0, 0};

  bool lit_eyes = false;

public:
  explicit phantom(breakout *game);
  ~phantom();
};

struct boss_container {
  boss_parent *hold = nullptr;
  texture_hub tex_hub;
  void summon_boss(breakout *game, BOSSES to_summon = RANDOM); // does nothign if boss already present
  void stop();                                                 // stops and deletes current boss
};

} // namespace bossfight