#pragma once

#include "breakout.h"
#include "gui.h"
#include <vector>

namespace bossfight {
enum BOSSES {
  RANDOM,
  NAME1
};
class boss_parent {
  long unsigned int lifetime = 0;

public:
  virtual boss_parent(breakout *game);
  virtual void draw(cairo_t *cr) = 0;
  virtual void tick(int time_diff) = 0;
};

class phantom : boss_parent {
};

struct boss_container {
  boss_parent *hold;
  void summon_boss(BOSSES to_summon = RANDOM); // does nothign if boss already present
  void stop();                                 // stops and deletes current boss
};

} // namespace bossfight