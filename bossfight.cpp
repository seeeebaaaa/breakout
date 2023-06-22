#include "bossfight.hpp"
#include "breakout.h"
#include "breakout_config.h"
#include "textures.hpp"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
namespace bossfight {
/**
 * BOSS PARENT
 */

void boss_parent::draw(cairo_t *cr) {
  switch (stage) {
  case INTRO:
    draw_intro(cr);
    break;
  case WAIT:
    draw_wait(cr);
    break;
  case FIGHT:
    draw_fight(cr);
    break;
  case DEATH:
    draw_death(cr);
    break;
  }
}

void boss_parent::tick(int time_diff) {
  lifetime += time_diff;
  switch (stage) {
  case PREP_INTRO:
    prep_intro();
    break;
  case INTRO:
    tick_intro(time_diff);
    break;
  case WAIT:
    tick_wait(time_diff);
    break;
  case FIGHT:
    tick_fight(time_diff);
    break;
  case DEATH:
    tick_death(time_diff);
    break;
  }
}

int boss_parent::timeline_eval(std::vector<int> &timeline, int lifetime) {
  int last_index = -1;
  for (int time_i : timeline) {
    if (lifetime < time_i) {
      return last_index;
    }
    last_index++;
  }
  return last_index;
}

int boss_parent::timeline_get_ticks(std::vector<int> &timeline, int lifetime) {
  return lifetime - timeline[timeline_eval(timeline, lifetime)];
}

/**
 * BOSS UTILITY
 */

void boss_container::summon_boss(breakout *game, BOSSES to_summon) {
  if (hold == nullptr) {
    if (to_summon == RANDOM) {
      to_summon = static_cast<BOSSES>(random_number(0, RANDOM));
    }
    std::cout << "Boss: " << to_summon << std::endl;
    // specific boss or set random
    switch (to_summon) {
    case PHANTOM:
      hold = new phantom(game);
      break;
    default:
      throw std::runtime_error(std::string("Unknown boss name: ") + std::to_string(to_summon));
      break;
    }
  }
}

/**
 * PHANTOM:
 * Intro:
 * - Warps multiple times
 * - Says oneliner
 *
 * Combat:
 * - Normal ball movement
 * - Hit phantom to damage
 *
 * Abilities:
 * - Darken screen
 * - appear and disappear [warp]
 * - Focus ball -> laser -> One of following effects:
 *    - Speedzp ball
 *    - Throw ball
 */
phantom::phantom(breakout *game) {
  this->game = game;
  game->main_cloud.set_sound("./assets/audio/bosses/phantom/hit.wav", 3, true);
  game->main_cloud.set_sound("./assets/audio/bosses/phantom/cloud.wav", 20, true);
  game->main_cloud.set_sound("./assets/audio/bosses/phantom/ball_dir_change.wav", 10, true);
  game->main_cloud.set_sound("./assets/audio/bosses/phantom/appear.wav", 3, true);
  game->main_cloud.set_sound("./assets/audio/bosses/phantom/disappear.wav", 3, true);
}

void phantom::draw_intro(cairo_t *cr) {
  std::vector<int> timeline = {
      0,     // wait
      500,   // first apearence
      2500,  // wait
      3000,  // warp
      5000,  // wait
      5500,  // final warp to screen center
      6500,  // wait
      7000,  // start of oneliner
      11000, // diaspear
      11500, // warp back
      12000  // start fight
  };
  // std::cout << "timeline: " << timeline_eval(timeline, lifetime) << std::endl;

  switch (timeline_eval(timeline, lifetime)) {
  case 0:
    // wait
    break;
  case 1: { // first apperaence
    float scale = timeline_get_ticks(timeline, lifetime) >= 1500 ? ((1000 - (timeline_get_ticks(timeline, lifetime) - 1000)) / 1000.0) : 0.5;
    if (!played_warp_1_1 && timeline_get_ticks(timeline, lifetime) < 1500) {
      played_warp_1_1 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/appear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    } else if (!played_warp_1_2 && timeline_get_ticks(timeline, lifetime) >= 1500) {
      played_warp_1_2 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/disappear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, rand_intro[0], game->get_height() + 50 - (timeline_get_ticks(timeline, lifetime) / 50), scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 2:
    // wait
    break;
  case 3: { // warp
    if (!played_warp_2_1 && timeline_get_ticks(timeline, lifetime) < 1500) {
      played_warp_2_1 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/appear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    } else if (!played_warp_2_2 && timeline_get_ticks(timeline, lifetime) >= 1500) {
      played_warp_2_2 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/disappear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    float scale = timeline_get_ticks(timeline, lifetime) >= 1500 ? ((1000 - (timeline_get_ticks(timeline, lifetime) - 1000)) / 1000.0) : 0.5;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, rand_intro[1], game->get_height() + 60 - (timeline_get_ticks(timeline, lifetime) / 50), scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 4: { // wait
    break;
  }
  case 5: { // final warp
    if (!played_warp_3_1 && timeline_get_ticks(timeline, lifetime) < 1000) {
      played_warp_3_1 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/appear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    float scale = (1 - ((1000 - (timeline_get_ticks(timeline, lifetime))) / 1000.0)) * 0.7;
    std::cout << scale << std::endl;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 6: // wait
  {
    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 7: { // start of oneliner
            // music
    game->main_cloud.play_sound("./assets/audio/bosses/phantom/theme.mp3", 0.7, 1, 0);

    float scale = 0.7;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main_eye_lit", "./assets/bosses/phantom/main_eye_lit.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);

    // text box
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, game->get_width() / 4.0, game->get_height() * 2.3 / 3.0 - 25, game->get_width() * 2 / 4.0, game->get_height() / 3.0 * 0.5);
    std::cout << game->get_width() / 3.0 << ", " << game->get_height() / 3.0 << std::endl;
    cairo_fill(cr);

    // text
    std::string text1 = "Watch out! My shadows";
    std::string text2 = "are coming for you..";

    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_text_centered(cr, game->get_width() / 2.0, game->get_height() * 2.4 / 3, 40, text1, game->pixel_font);
    draw_text_centered(cr, game->get_width() / 2.0, game->get_height() * 2.4 / 3 + 50, 40, text2, game->pixel_font);

    break;
  }
  case 8: { // diasppear
    if (!played_warp_4_1 && timeline_get_ticks(timeline, lifetime) < 500) {
      played_warp_4_1 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/disappear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    float scale = (((500 - (timeline_get_ticks(timeline, lifetime))) / 500.0)) * 0.7;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 9: { // reappear
    if (!played_warp_4_2 && timeline_get_ticks(timeline, lifetime) < 500) {
      played_warp_4_2 = true;
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/appear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    float scale = (1 - ((500 - (timeline_get_ticks(timeline, lifetime))) / 500.0)) * 0.4;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3 + 30, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  case 10: { // wait/start fight
    float scale = 0.4;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, game->get_width() / 2, game->get_height() / 3 + 30, scale, scale);
    cairo_paint(cr);
    cairo_scale(cr, 1 / scale, 1 / scale);
    break;
  }
  default:
    throw std::runtime_error(std::string("TIMELINE: Event") + std::to_string(timeline_eval(timeline, lifetime)) + " not present!");
  }
}

void phantom::draw_wait(cairo_t *cr) {
  for (ball *b : game->get_balls()) {
    b->draw(cr);
  }
  game->get_boards().front().draw(cr);

  float scale = 0.4;
  cairo_scale(cr, scale, scale);
  game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, x, y + (5 - get_circle_radius(lifetime, 1500, 1, 10)), scale, scale);
  cairo_paint(cr);
  cairo_scale(cr, 1 / scale, 1 / scale);

  // draw lives
  cairo_save(cr);
  cairo_set_source_rgba(cr, .1, .1, .1, .85);
  cairo_rectangle(cr, game->get_width() / 4, game->get_height() * 5.1 / 8, game->get_width() / 2, game->get_height() * 0.6 / 8);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_text_centered(cr, game->get_width() / 2, game->get_height() * 5.5 / 8, 30, "Lives left: " + std::to_string(game->get_lives()), game->pixel_font);
  cairo_restore(cr);
}

void phantom::draw_fight(cairo_t *cr) {
  for (ball *b : game->get_balls()) {
    b->draw(cr);
  }
  game->get_boards().front().draw(cr);

  // ability focus
  if (ball_in_focus) {
    cairo_save(cr);
    cairo_set_source_rgba(cr, col[0], col[1], col[2], 0.8);
    cairo_move_to(cr, x, y);
    cairo_set_line_width(cr, 5);
    for (ball *b : game->get_balls()) {
      cairo_line_to(cr, b->x, b->y);
      cairo_stroke(cr);
    }
    cairo_restore(cr);
  }

  float scale = 0.4;
  cairo_scale(cr, scale, scale);
  if (lit_eyes)
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main_eye_lit", "./assets/bosses/phantom/main_eye_lit.png")->texel_set_source_surface(cr, x, y + (5 - get_circle_radius(lifetime, 1500, 1, 10)), scale, scale);
  else
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, x, y + (5 - get_circle_radius(lifetime, 1500, 1, 10)), scale, scale);
  cairo_paint_with_alpha(cr, alpha_boss);
  cairo_scale(cr, 1 / scale, 1 / scale);

// debug
#ifdef BREAKOUT_BOSS_DRAW_COLLISION
  cairo_save(cr);
  cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
  cairo_translate(cr, x, y);
  cairo_scale(cr, hitbox[0] / 2, hitbox[1] / 2);
  cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
  cairo_stroke(cr);
  cairo_restore(cr);
#endif

  // abilities: cloud

  for (cloud &c : clouds) {
    float scale = c.scale;
    cairo_scale(cr, scale, scale);
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_cloud", "./assets/bosses/phantom/cloud.png")->texel_set_source_surface(cr, c.x, c.y, scale, scale);
    cairo_paint_with_alpha(cr, c.alpha);
    cairo_scale(cr, 1 / scale, 1 / scale);
  }

  // health bar last element so always visible
  int pos_x = game->get_width() / 2.0;
  int pos_y = 20;
  int length = game->get_width() * 0.6;
  int height = 10;
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, pos_x - length / 2, pos_y - height / 2, length, height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_rectangle(cr, pos_x - length / 2, pos_y - height / 2, std::clamp(health * length / max_health, 0, length), height);
  cairo_fill(cr);
}

void phantom::draw_death(cairo_t *cr) {
  float scale = 0.4;
  cairo_scale(cr, scale, scale);
  if (lit_eyes)
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main_eye_lit", "./assets/bosses/phantom/main_eye_lit.png")->texel_set_source_surface(cr, x, y + (5 - get_circle_radius(lifetime, 1500, 1, 10)), scale, scale);
  else
    game->boss_cont->tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, x, y + (5 - get_circle_radius(lifetime, 1500, 1, 10)), scale, scale);
  cairo_paint_with_alpha(cr, alpha_boss);
  cairo_scale(cr, 1 / scale, 1 / scale);
}

void phantom::prep_intro() {
  std::cout << "Prepped for bossfight!" << std::endl;
  std::vector<int> ind;
  int i = 0;
  for (ball *b : game->get_balls()) {
    b->position(game->get_width() / 2, game->get_height() * .95 - b->rad);
    b->spin_add = 0;
    b->direction(0, -1);
    if (!b->main) {
      ind.push_back(i);
    }
    i++;
  }
  std::reverse(ind.begin(), ind.end());
  for (int i : ind)
    delete_element_in_vector(i, game->get_balls());

  // order boards
  for (board &b : game->get_boards()) {
    b.x = game->get_width() / 2 - (game->get_width() * 0.15) / 2;
    b.y = game->get_height() * .95;
  }
  // clean powerups
  game->pu_container->delete_all();
  // launder timer
  game->pu_container->ticks_until_next_pu = game->pu_container->interval_time * 2;

  x = game->get_width() / 2.0;
  y = game->get_height() / 3 + 30;

  rand_intro[0] = random_number(game->get_width() * 0.1, game->get_width() * 0.9);
  rand_intro[1] = random_number(game->get_width() * 0.1, game->get_width() * 0.9);

  stage = INTRO;
}

void phantom::tick_intro(int time_diff) {
  if (lifetime >= 13000) {
    lifetime = 0;
    stage = WAIT;
  }
}

void phantom::tick_wait(int time_diff) {
  if (game->get_key_pressed()[' ']) {
    lifetime = 0;
    stage = FIGHT;
  }
  // board movement
  if (game->get_key_pressed()['a']) {
    game->get_boards().front().move_left(game->get_boards().front().speed * time_diff);
    for (ball *b : game->get_balls()) {
      b->position(game->get_boards().front().x + game->get_boards().front().w / 2, game->get_boards().front().y - game->get_balls().front()->rad);
    }
  }
  if (game->get_key_pressed()['d']) {
    game->get_boards().front().move_right(game->get_boards().front().speed * time_diff, game->get_width());
    for (ball *b : game->get_balls()) {
      b->position(game->get_boards().front().x + game->get_boards().front().w / 2, game->get_boards().front().y - game->get_balls().front()->rad);
    }
  }

  game->get_boards().front().tick(time_diff);
}

void phantom::tick_fight(int time_diff) {
  // temp
  if (game->get_key_pressed()['p']) {
    lifetime = 0;
    stage = DEATH;
  }

  // board movement
  if (game->get_key_pressed()['a']) {
    game->get_boards().front().move_left(game->get_boards().front().speed * time_diff);
  }
  if (game->get_key_pressed()['d']) {
    game->get_boards().front().move_right(game->get_boards().front().speed * time_diff, game->get_width());
  }

  game->get_boards().front().tick(time_diff);

  // ball ticks & collision
  // move balls and handle collision between ball and blocks,walls,board and powerups
  std::vector<int> to_be_deleted; // sorted list of delete indexes
  int start_size = game->get_balls().size();
  for (int i = 0; i < start_size; i++) {
    ball *b = game->get_balls()[i];
    b->tick(time_diff);
    if (b->life_time <= 0) {
      std::cout << "\e[31;1mlife:\e[0m " << b->life_time << std::endl;
      to_be_deleted.push_back(i);
    }
    if (!b->check_collision(*game, time_diff)) {
      game->add_lives(-1);
      game->main_cloud.play_sound("./assets/audio/fall_Down.wav", 1, .8 + random_number(0, .4), 0);
      if (game->get_lives() == 0) {
      } else {
        // restore game
        std::vector<int> ind;
        int i = 0;
        for (ball *b : game->get_balls()) {
          b->position(game->get_width() / 2, game->get_height() * .95 - b->rad);
          b->spin_add = 0;
          b->direction(0, 1);
          if (!b->main) {
            ind.push_back(i);
          }
          i++;
        }
        std::reverse(ind.begin(), ind.end());
        for (int i : ind)
          delete_element_in_vector(i, game->get_balls());
        // order boards
        for (board &b : game->get_boards()) {
          b.x = game->get_width() / 2 - (game->get_width() * 0.15) / 2;
          b.y = game->get_height() * .95;
        }
        stage = WAIT;
      }
      break;
    }
  }
  if (to_be_deleted.size())
    std::cout << "Ball Delete: " << std::endl;
  std::reverse(to_be_deleted.begin(), to_be_deleted.end());
  for (int ind : to_be_deleted) {
    std::cout << ind << " ";
    delete_element_in_vector(ind, game->get_balls());
  }

  // abilitys
  ticks_since_last_ability -= time_diff;
  if (ticks_since_last_ability <= 0) {
    std::cout << "Newa ability choosen" << std::endl;
    ticks_since_last_ability = 7000 + random_number(0, 6) * 1000;
    current_ability = random_number(0, 4 - .01);

    // try to play music again so it never stops
    game->main_cloud.play_sound("./assets/audio/bosses/phantom/theme.mp3", .7, 1, 0);
  }
  switch (current_ability) {
  case 0: // darken screen
  {
    // cloud buildup
    if (current_ability_lifetime < cloud_max && current_ability_lifetime % 10 < 2) {
      int x = random_number(0, game->get_width());
      int y = random_number(0, game->get_height());
      float scale = random_number(1, 1.9);
      float alpha = 1;
      clouds.push_back(cloud(x, y, scale, alpha));
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/cloud.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
      std::cout << "0-buildup" << std::endl;
      lit_eyes = true;
    }
    // cloud vanish
    if (current_ability_lifetime > cloud_max * 5 && current_ability_lifetime < cloud_max * 6 && current_ability_lifetime % 10 < 2) {
      if (clouds.size() > 0)
        clouds.erase(clouds.end() - 1);
      std::cout << "0-vanish" << std::endl;
    }
    if (current_ability_lifetime >= cloud_max * 6) {
      std::cout << "0-del" << std::endl;
      current_ability_lifetime = 0;
      current_ability = -1;
      lit_eyes = false;
      clouds.clear();
    }
    break;
  }
  case 1: // warp
    if (current_ability_lifetime == 0) {
      // init
      warp_pos[0] = random_number(game->get_width() * 0.2, game->get_width() * 0.8);
      warp_pos[1] = random_number(game->get_height() * 0.2, game->get_height() * 0.8);
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/appear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
      played_once = true;
      lit_eyes = true;
    }
    if (current_ability_lifetime <= 1000) {
      alpha_boss = 1 - current_ability_lifetime / 1000.0;
    }
    if (current_ability_lifetime > 1000 && current_ability_lifetime <= 3000) {
      // move away cause hutbox
      x = game->get_width() * 2;
      y = game->get_height() * 2;
    }
    if (current_ability_lifetime > 3000) {
      x = warp_pos[0];
      y = warp_pos[1];
      alpha_boss = (current_ability_lifetime - 3000) / 500.0;
      if (!played_once) {
        game->main_cloud.play_sound("./assets/audio/bosses/phantom/disappear.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
        played_once = true;
      }
    }
    if (current_ability_lifetime > 3500) {
      alpha_boss = 1;
      current_ability_lifetime = 0;
      current_ability = -1;
      lit_eyes = false;
    }

    break;
  case 2: // focus ball - speed
    if (current_ability_lifetime == 0) {
      // init
      multiplier = random_number(2, 3.5);
      org_speed = game->get_balls().front()->speed;
      col[0] = 18.0 / 255;
      col[1] = 97.0 / 255;
      col[2] = 92.0 / 255;
      ball_in_focus = true;
      lit_eyes = true;
    }
    if (current_ability_lifetime <= 2000) {
      for (ball *b : game->get_balls()) {
        b->speed = org_speed * multiplier;
      }
    }
    if (current_ability_lifetime > 2000) {
      ball_in_focus = false;
      for (ball *b : game->get_balls()) {
        b->speed = org_speed;
      }
      current_ability = -1;
      current_ability_lifetime = 0;
      lit_eyes = false;
    }
    break;
  case 3: // focus ball - throw/locks
    if (current_ability_lifetime == 0) {
      // init
      org_speed = game->get_balls().front()->speed;
      col[0] = 97.0 / 255;
      col[1] = 18.0 / 255;
      col[2] = 79.0 / 255;
      ball_in_focus = true;
      lit_eyes = true;
    }
    if (multi_steps <= 0) {
      multiplier = random_number(1, 1.5);
      dir[0] = random_number(-1, 1);
      dir[1] = random_number(-1, 1);
      for (ball *b : game->get_balls()) {
        b->direction(dir[0], dir[1]);
      }
      multi_steps += 250 + (25 - random_number(0, 25));
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/ball_dir_change.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
    if (current_ability_lifetime <= 2000) {
      for (ball *b : game->get_balls()) {
        b->speed = org_speed * multiplier;
      }
    }
    if (current_ability_lifetime > 2000) {
      ball_in_focus = false;
      for (ball *b : game->get_balls()) {
        b->speed = org_speed;
      }
      current_ability = -1;
      current_ability_lifetime = 0;
      lit_eyes = false;
    }
    multi_steps -= time_diff;
    break;
  default:
    // no ab
    break;
  }
  if (current_ability != -1)
    current_ability_lifetime += time_diff;

  // helth
  if (health <= 0 && current_ability == -1) {
    lifetime = 0;
    stage = DEATH;
  }
  // lives
  if (game->get_lives() <= 0) {
    game->main_cloud.stop_sound("./assets/audio/bosses/phantom/theme.mp3");
    game->main_cloud.play_sound("./assets/audio/bosses/phantom/cloud.wav", 1, 0.7, 0);
    finished = true;
  }
}

void phantom::tick_death(int time_diff) {
  if (lifetime == 0) {
    // init
    lit_eyes = true;
  }
  if (!play_death_once) {
    play_death_once = true;
    game->main_cloud.stop_sound("./assets/audio/bosses/phantom/theme.mp3");
    game->main_cloud.play_sound("./assets/audio/bosses/phantom/cloud.wav", 1, 0.7, 0);
  }
  // lazy and not good but idc tbh
  if (lifetime > 0 && lifetime <= 1000) {
    lit_eyes = true;
  }
  if (lifetime > 1000 && lifetime <= 1100) {
    lit_eyes = false;
  }
  if (lifetime > 1100 && lifetime <= 1500) {
    lit_eyes = true;
  }
  if (lifetime > 1500 && lifetime <= 1600) {
    lit_eyes = false;
  }
  if (lifetime > 1600 && lifetime <= 1900) {
    lit_eyes = true;
  }
  if (lifetime > 1900 && lifetime <= 1950) {
    lit_eyes = false;
  }
  if (lifetime > 1950 && lifetime <= 2150) {
    lit_eyes = true;
  }
  if (lifetime > 2150 && lifetime <= 3000) {
    lit_eyes = false;
    org_y = y;
  }
  // sink down
  if (lifetime > 3500 && lifetime <= 7500) {
    std::cout << "y: " << y << ", org_y: " << org_y << ", pow: " << powf(1.00015, lifetime - 3500) << ", x: " << powf(game->get_height() / org_y, 1.0 / (7500 - 3500)) << std::endl;
    y = org_y * powf(powf((game->get_height() + 200) / org_y, 1.0 / (7500 - 3500)), lifetime - 3500);
  }

  if (lifetime >= 8000) {
    lifetime = 0;
    finished = true;
  }
}

void phantom::collision(ball *b) {
  float r_x = hitbox[0];
  float r_y = hitbox[1];

  if ((powf(b->x - x, 2)) / powf(r_x, 2) + (powf(b->y - y, 2)) / powf(r_y, 2) <= 1) {
    // on ellipse, hit boss
    if (!hit) {
      hit = true;
      health -= 50 + (5 - random_number(0, 10));
      game->add_score(50 + (5 - random_number(0, 10)));
      game->main_cloud.play_sound("./assets/audio/bosses/phantom/hit.wav", 1, 1 + (0.2 - random_number(0, 0.4)), 0);
    }
  } else {
    // not in hitbox, make hittable again
    hit = false;
  }
}

phantom::~phantom() {}

} // namespace bossfight