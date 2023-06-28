#include "power_ups.hpp"
#include "breakout.h"
#include "gui.h"
#include "util.h"
#include <cassert>
#include <cmath>
#include <iostream>
/**
 * PARENT
 */

bool powerup::parent::collides(float lx, float ly, int x, int y) {
  // best way:
  // make line between lx/ly and x/y and get distance of this->x/this->y to that line
  // if that distance is less then collision_radius, then return true, otherwise false
  float a[2] = {lx, ly};
  float b[2] = {(float)x, (float)y};
  float p[2] = {(float)this->x, (float)this->y};
  float d = distance_between_line_and_point(a, b, p);
  return !active && d <= 20 && distance_between_points(lx, ly, this->x, this->y) <= distance_between_points(lx, ly, x, y) + collision_radius * 2;
}

/**
 * SLOWDOWN
 * Slows the ball a bit down for a couple of seconds
 */
powerup::slowdown::slowdown(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
}

void powerup::slowdown::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
    cairo_set_source_rgb(cr, 1, 0, 0);
    draw_circle(cr, x, y, ticks_since_activation * 15 * 3 / effect_time);
  } else {
    // cairo_set_source_rgb(cr, 0, 0, 1);
    // draw_circle(cr, x, y, 10);
    // cairo_stroke(cr);
    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    game->tex_hub.get_or_create_texel("pu_slowdown", "assets/powerups/slowdown.png")->texel_set_source_surface(cr, x, y, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
  }
  cairo_restore(cr);
}

void powerup::slowdown::activate(ball *b) {
  active = true;
  affected_ball = b;
  std::cout << "Changes seped from " << affected_ball->speed << " to ";
  affected_ball->speed *= slow_down_factor;
  game->main_cloud.play_sound("./assets/audio/power_ups/slowdown.wav", 1, 1, 0);
  std::cout << affected_ball->speed << std::endl;
}

void powerup::slowdown::deactivate() {
  active = false;

  affected_ball->speed *= 1 / slow_down_factor;

  mghhh_delete_me_daddy = true;
}

powerup::slowdown::~slowdown() {
  if (active) {
    deactivate();
  }
}

void powerup::slowdown::tick(int tick_offset) {
  if (active) {
    if (ticks_since_activation >= effect_time)
      deactivate();
    ticks_since_activation += tick_offset;
  }
}

/**
 * SPEEDUP
 * Speeds the ball a bit up for a couple of seconds
 */

powerup::speedup::speedup(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
}

void powerup::speedup::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
    cairo_set_source_rgb(cr, 0, 1, .3);
    draw_circle(cr, x, y, get_circle_radius(ticks_since_activation, effect_time / 4, 1, 30));
    // std::cout << (float)ticks_since_activation / effect_time * 100.0 << std::endl;
  } else {
    // cairo_set_source_rgb(cr, 0, 1, .3);
    // draw_circle(cr, x, y, 10);
    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    game->tex_hub.get_or_create_texel("pu_speedup", "assets/powerups/speedup.png")->texel_set_source_surface(cr, x, y, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
  }
  cairo_restore(cr);
}

void powerup::speedup::activate(ball *b) {
  active = true;
  affected_ball = b;
  affected_ball->speed *= speedup_factor;
  game->main_cloud.play_sound("./assets/audio/power_ups/speedup.wav", 1, 1, 0);
}

void powerup::speedup::deactivate() {
  active = false;

  affected_ball->speed *= 1 / speedup_factor;

  mghhh_delete_me_daddy = true;
}

powerup::speedup::~speedup() {
  if (active) {
    deactivate();
  }
}

void powerup::speedup::tick(int tick_offset) {
  if (active) {
    if (ticks_since_activation >= effect_time)
      deactivate();
    ticks_since_activation += tick_offset;
  }
}

/**
 * BLACK HOLE
 * Sucks in the ball and propels it in a random direction
 */

powerup::black_hole::black_hole(breakout *game) {
  std::cout << "NEW blackhole" << std::endl;
  this->game = game;
  active = false;
  x = 0;
  y = 0;
}

void powerup::black_hole::draw(cairo_t *cr) {
  cairo_save(cr);

  if (active) {
    cairo_set_source_rgb(cr, 0, 1, 1);
    draw_circle(cr, x, y, collision_radius);
    draw_circle(cr, x, y, collision_radius / 2);
    draw_circle(cr, x, y, collision_radius / 3);
    cairo_set_source_rgb(cr, 1, 1, 1);
    // angle indicator
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + (sinf(spin_angle) * indicator_length), y + (cosf(spin_angle) * indicator_length));
    cairo_stroke(cr);
  } else {
    cairo_set_source_rgb(cr, 0, .5, 1);
    draw_circle(cr, x, y, collision_radius);
  }

  cairo_restore(cr);
}

void powerup::black_hole::activate(ball *b) {
  active = true;
  state = PULL;
  affected_ball = b;
  affected_ball->dx = 0;
  affected_ball->dy = 0;
  spin_angle = random_number(0, 2) * M_PI;
  std::cout << "Activated!" << std::endl;
  game->main_cloud.play_sound("./assets/audio/power_ups/blackhole_pull.wav", 1, 1, 0);
}

void powerup::black_hole::deactivate() {
  active = false;
  mghhh_delete_me_daddy = true;
  affected_ball->speed = original_speed;
}

powerup::black_hole::~black_hole() {
  if (active) {
    deactivate();
  }
}

void powerup::black_hole::tick(int tick_offset) {
  lifetime += tick_offset;
  collision_radius = get_circle_radius(lifetime, 2000, 1, 20);
  if (state > IDLE) {
    if (state == PULL) {
      if (distance_between_points(x, y, affected_ball->x, affected_ball->y) <= 2 || max_tries <= 0) {
        // ball is pulled close enough
        state = SPIN;
        affected_ball->direction(0, 0);
        game->main_cloud.play_sound("./assets/audio/power_ups/blackhole_spin.wav", 1, 0.5, 0);
      } else {
        affected_ball->direction(x - affected_ball->x, y - affected_ball->y);
        affected_ball->dx *= 0.9;
        affected_ball->dy *= 0.9;
        affected_ball->step(1);
        max_tries--;
      }

    } else if (state == SPIN) {
      // spin ti desired direction
      spin_angle += ((float)spin_time / spin_time_max) * 0.1f * M_PI * (tick_offset * 0.1);
      // std::cout << "Angle: " << spin_angle << " (addition: " << (float)(1.0 / 10.0) * M_PI << ")" << std::endl;
      if (spin_time <= 0) {
        game->main_cloud.play_sound("./assets/audio/power_ups/blackhole_shoot.wav", 1, 1, 0);
        state = SHOOT;
      }
      spin_time -= tick_offset;
    } else if (state == SHOOT) {
      // shoot the ball

      affected_ball->direction(sinf(spin_angle), cosf(spin_angle));
      original_speed = affected_ball->speed;
      affected_ball->speed *= 2.2;

      state = SLOWDOWN;
    } else if (state == SLOWDOWN) {
      if (affected_ball->speed > original_speed) {
        affected_ball->speed *= powf(0.99, tick_offset / 10.0);
      } else
        deactivate();
    }
  }
}

/**
 * DOUBLE BALLS
 */

powerup::double_ball::double_ball(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
  collision_radius = 20;
}

void powerup::double_ball::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  // two circles spinning
  // draw_text_centered(cr, x, y, 10, "x2");
  int outer_radius = 10;
  int circle_radius = 10;
  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_circle(cr, x + sinf(angle) * (outer_radius + ticks_since_activation * (50.0 / 300)), y + cosf(angle) * (outer_radius + ticks_since_activation * (50.0 / 300)), circle_radius);
  draw_circle(cr, x + sinf(angle + M_PI / 2) * (outer_radius + ticks_since_activation * (50.0 / 300)), y + cosf(angle + M_PI / 2) * (outer_radius + ticks_since_activation * (50.0 / 300)), circle_radius, true);
  draw_circle(cr, x + sinf(angle + M_PI) * (outer_radius + ticks_since_activation * (50.0 / 300)), y + cosf(angle + M_PI) * (outer_radius + ticks_since_activation * (50.0 / 300)), circle_radius);
  draw_circle(cr, x + sinf(angle + M_PI * 1.5) * (outer_radius + ticks_since_activation * (50.0 / 300)), y + cosf(angle + M_PI * 1.5) * (outer_radius + ticks_since_activation * (50.0 / 300)), circle_radius, true);

  if (active) {
    draw_circle(cr, x, y, ticks_since_activation * (50.0 / 300));
  }

  cairo_restore(cr);
}

void powerup::double_ball::activate(ball *b) {
  active = true;
  int current_ball_count = game->get_balls().size();
  float ball_speed = b->speed * 0.9;
  for (int i = 0; i < current_ball_count; i++) {
    ball *new_ball = new ball;
    new_ball->x = x;
    new_ball->y = y;
    new_ball->direction(b->dx + .1 - 0.2 * random_number(), b->dy + .1 - .2 * random_number());
    new_ball->main = false;
    new_ball->life_time = 10000;
    new_ball->speed = ball_speed;
    std::cout << "New ball rot: [" << new_ball->dx << "," << new_ball->dy << "], speed: " << new_ball->speed << std::endl;
    game->add_ball(new_ball);
  }
  game->main_cloud.play_sound("./assets/audio/power_ups/doubleup.wav", 1, 1, 0);
}

void powerup::double_ball::deactivate() {
  active = false;
  mghhh_delete_me_daddy = true;
}

powerup::double_ball::~double_ball() {
  if (active) {
    deactivate();
  }
}

void powerup::double_ball::tick(int tick_offset) {
  angle += 0.001 * M_PI * tick_offset;
  if (ticks_since_activation >= 100)
    deactivate();
  ticks_since_activation += active * tick_offset;
}

/**
 * LIGHTSOUT
 * Makes only the balls visible
 */
powerup::lights_out::lights_out(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
}

void powerup::lights_out::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    for (ball *b : game->get_balls())
      cairo_arc(cr, b->x, b->y, 45, 0, 2 * M_PI);
    cairo_rectangle(cr, 0, 0, game->get_width(), game->get_height());
    cairo_clip(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
  } else {
    cairo_set_source_rgb(cr, 1, 1, 0);
    draw_circle(cr, x, y, 15);
    draw_circle(cr, x, y, 7, (int)(0.5 + get_circle_radius(lifetime, 50, 1, 1)));
  }
  cairo_restore(cr);
}

void powerup::lights_out::activate(ball *b) {
  active = true;
  affected_ball = b;
  std::cout << affected_ball->speed << std::endl;
  game->main_cloud.play_sound("./assets/audio/power_ups/lights_out.wav", 1, 1, 0);
}

void powerup::lights_out::deactivate() {
  active = false;
  mghhh_delete_me_daddy = true;
}

powerup::lights_out::~lights_out() {
  if (active) {
    deactivate();
  }
}

void powerup::lights_out::tick(int tick_offset) {
  if (active) {
    if (ticks_since_activation >= effect_time)
      deactivate();
    ticks_since_activation += tick_offset;
  }
  lifetime++;
}

/**
 * SCORE ADD OR SUB
 * Makes only the balls visible
 */
powerup::score_add_or_sub::score_add_or_sub(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
  add = random_number() >= 0.5;
  if (!add)
    score_diff *= -1;
}

void powerup::score_add_or_sub::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
    // draw score points
    if (add) {
      cairo_set_source_rgb(cr, 0, 1, 0);
    } else {
      cairo_set_source_rgb(cr, 1, 0, 0);
    }
    draw_text_centered(cr, x, y, 30 + get_circle_radius(ticks_since_activation, 50, 1, 5), (add ? "+" : "") + std::to_string(score_diff), game->pixel_font);
  } else {
    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    if (add)
      game->tex_hub.get_or_create_texel("pu_score_plus", "assets/powerups/score_plus.png")->texel_set_source_surface(cr, x, y, scale, scale);
    else
      game->tex_hub.get_or_create_texel("pu_score_minus", "assets/powerups/score_minus.png")->texel_set_source_surface(cr, x, y, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
  }
  cairo_restore(cr);
}

void powerup::score_add_or_sub::activate(ball *b) {
  active = true;
  affected_ball = b;
  game->add_score(score_diff);
  if (add)
    game->main_cloud.play_sound("./assets/audio/power_ups/score_add.wav", 1, 1, 0);
  else
    game->main_cloud.play_sound("./assets/audio/power_ups/score_minus.wav", 1, 1, 0);
}

void powerup::score_add_or_sub::deactivate() {
  active = false;
  mghhh_delete_me_daddy = true;
}

powerup::score_add_or_sub::~score_add_or_sub() {
  if (active) {
    deactivate();
  }
}

void powerup::score_add_or_sub::tick(int tick_offset) {
  if (active) {
    if (ticks_since_activation >= 200)
      deactivate();
    ticks_since_activation += tick_offset;
  }
  lifetime++;
}

/**
 * INVINCIBLE
 * Slows the ball a bit down for a couple of seconds
 */
powerup::invincible::invincible(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
  collision_radius = 15;
}

void powerup::invincible::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
  } else {
    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_circle(cr, x, y, 10, true);
    cairo_set_source_rgb(cr, 66.0 / 255, 218.0 / 255, 245.0 / 255);
    int part_count = 5;
    float offset = lifetime * 0.003 * M_PI;
    for (int i = 0; i < part_count; i++) {
      cairo_arc(cr, x, y, 15, ((2.0 * M_PI) / part_count) * i + offset, ((2.0 * M_PI) / part_count) * (i + .5) + offset);
      cairo_stroke(cr);
    }
  }
  cairo_restore(cr);
}

void powerup::invincible::activate(ball *b) {
  active = true;
  affected_ball = b;
  affected_ball->invincible = true;
  game->main_cloud.play_sound("./assets/audio/power_ups/invincible.wav", 1, 1, 0);
}

void powerup::invincible::deactivate() {
  active = false;

  affected_ball->invincible = false;

  mghhh_delete_me_daddy = true;
}

powerup::invincible::~invincible() {
  if (active) {
    deactivate();
  }
}

void powerup::invincible::tick(int tick_offset) {
  lifetime++;
  if (active) {
    affected_ball->invincible = true;
    if (ticks_since_activation >= effect_time)
      deactivate();
    ticks_since_activation += tick_offset;
  }
}

/**
 * GAIN LIFE
 * Slows the ball a bit down for a couple of seconds
 */
powerup::gain_life::gain_life(breakout *game) {
  this->game = game;
  active = false;
  x = 0;
  y = 0;
  collision_radius = 15;
}

void powerup::gain_life::draw(cairo_t *cr) {
  // draw the powerup
  cairo_save(cr);
  if (active) {
    draw_text_centered(cr, x, y, 30 + get_circle_radius(ticks_since_activation, 50, 1, 5), "+life", game->pixel_font);
  } else {
    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    game->tex_hub.get_or_create_texel("pu_gain_live", "assets/powerups/gain_live.png")->texel_set_source_surface(cr, x, y, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
  }
  cairo_restore(cr);
}

void powerup::gain_life::activate(ball *b) {
  active = true;
  game->add_lives(1);
  game->main_cloud.play_sound("./assets/audio/power_ups/gain_life.wav", 1, 1, 0);
}

void powerup::gain_life::deactivate() {
  active = false;

  mghhh_delete_me_daddy = true;
}

powerup::gain_life::~gain_life() {
  if (active) {
    deactivate();
  }
}

void powerup::gain_life::tick(int tick_offset) {
  if (active) {
    if (ticks_since_activation >= 250)
      deactivate();
    ticks_since_activation += tick_offset;
  }
  lifetime++;
}

/**
 *
 * RANDOMIZER
 *
 */

powerup::parent *powerup::random_powerup(breakout *game) {
  int number_of_available_powerups = 8;
  int rand = random_number(0, number_of_available_powerups - .01);

  switch (rand) {
  case 0:
    return new slowdown(game);
  case 1:
    return new speedup(game);
  case 2:
    return new black_hole(game);
  case 3:
    return new double_ball(game);
  case 4:
    return new lights_out(game);
  case 5:
    return new score_add_or_sub(game);
  case 6:
    return new invincible(game);
  case 7:
    return new gain_life(game);
  default:
    std::cout << "\e[34;1maaa DEFAULTING NOT GOOD (" << rand << ")\e[0m" << std::endl;
    return new slowdown(game);
    break;
  }
  return nullptr;
}

/**
 * POWER UP CONTAINER
 */

powerup::power_up_container::power_up_container(breakout *game, int max_powerups, int place_count, int nr_of_places, int places[6][2]) {
  this->game = game;
  this->max_powerups = max_powerups;
  this->place_count = place_count;
  assert(max_powerups <= place_count && "Max Number of Powerups at once exceeds powerup capacity!");
  assert(nr_of_places == 0 || nr_of_places >= place_count);
  assert(nr_of_places >= 0 && place_count >= 0 && max_powerups >= 0 && "really?");
  for (int i = 0; i < nr_of_places; i++) {
    power_up_housing new_house;
    new_house.pu = nullptr;
    new_house.x = nr_of_places ? places[i][0] : 0; // random instead of 0
    new_house.y = nr_of_places ? places[i][1] : 0; // random instead of 0
    house.push_back(new_house);
    std::cout << "House: [" << new_house.x << "," << new_house.y << "]" << std::endl;
  }

  // load powerup sounds
  game->main_cloud.set_sound("./assets/audio/power_ups/blackhole_pull.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/blackhole_spin.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/blackhole_shoot.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/doubleup.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/gain_life.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/invincible.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/lights_out.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/score_add.wav", 2);
  game->main_cloud.set_sound("./assets/audio/power_ups/score_minus.wav", 2);
  game->main_cloud.set_sound("./assets/audio/power_ups/slowdown.wav", 3);
  game->main_cloud.set_sound("./assets/audio/power_ups/speedup.wav", 3);
}

void powerup::power_up_container::collision(ball *b) {
  for (power_up_housing &h : house) {
    if (h.pu) {
      if (h.pu->collides(b->last_good_x, b->last_good_y, b->x, b->y)) {
        h.pu->activate(b);
        std::cout << "Activated!" << std::endl;
      }
    }
  }
}

void powerup::power_up_container::new_random_powerup() {
  int esc = true, rand = 0;
  while (esc) {
    rand = random_number(0.0f, (float)place_count);
    esc = (bool)house[rand].pu;
  }
  house[rand].pu = powerup::random_powerup(game);
  house[rand].pu->x = house[rand].x;
  house[rand].pu->y = house[rand].y;
  std::cout << "\e[31;1mCreated with at [" << house[rand].pu->x << ";" << house[rand].pu->y << "]\e[0m" << std::endl;
}

void powerup::power_up_container::tick(int time_diff) {
  bool free_space = false;
  for (power_up_housing &h : house) {
    free_space |= !(bool)h.pu;
  }
  if (ticks_until_next_pu <= 0) {
    ticks_until_next_pu = interval_time * (1.5 + random_number());
    std::cout << "Next Powerup in " << ticks_until_next_pu << " ticks!" << std::endl;
    // new powerup
    if (free_space) {
      new_random_powerup();
    }
  }
  ticks_until_next_pu -= free_space;
  for (power_up_housing &h : house) {
    if (h.pu) {
      h.pu->tick(time_diff);
      if (h.pu->mghhh_delete_me_daddy) {
        parent *hold = h.pu;
        delete hold;
        h.pu = nullptr;
      }
    }
  }
}

void powerup::power_up_container::draw(cairo_t *cr) {
  for (power_up_housing &h : house) {
    if (h.pu) {
      h.pu->draw(cr);
    }
  }
}

void powerup::power_up_container::deactivate_all() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  for (power_up_housing &h : house) {
    if (h.pu) {
      h.pu->deactivate();
    }
  }
}

void powerup::power_up_container::delete_all() {
  std::cout << "Container contains " << house.size() << " housings!" << std::endl;
  for (power_up_housing &h : house) {
    std::cout << "Deleteing " << h.pu << std::endl;
    parent *hold = h.pu;
    delete hold;
    h.pu = nullptr;
  }
}

powerup::power_up_container::~power_up_container() {
  delete_all();
}