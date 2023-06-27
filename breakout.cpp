#include "breakout.h"
#include "breakout_config.h"
#include "textures.hpp"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "sound.hpp"
#include <cairo-ft.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <tuple>
static cairo_t *cr = nullptr;

breakout::breakout(int w, int h)
    : windowed_app(w, h, "brick breaker"),
      cells_x(BREAKOUT_CELLS_X),
      cells_y(BREAKOUT_CELLS_Y),
      space(BREAKOUT_BORDER) {
  if (FT_Init_FreeType(&ft)) {
    std::cout << "ERROR in init" << std::endl;
  };
  if (auto error = FT_New_Face(ft, "assets/windows_command_prompt.ttf", 0, &ft_face); error) {
    std::cout << "ERROR in face gen" << error << std::endl;
  }
  pixel_font = cairo_ft_font_face_create_for_ft_face(ft_face, 0);

  main_cloud.set_sound("./assets/audio/pickupCoin.wav", 1);
  main_cloud.set_sound("./assets/audio/black_out.wav", 1);
  main_cloud.set_sound("./assets/audio/ui_right.wav", 2);
  main_cloud.set_sound("./assets/audio/ui_wrong.wav", 2);
  main_cloud.set_sound("./assets/audio/ball_release.wav", 1);
  main_cloud.set_sound("./assets/audio/ball_hit.wav", 10);
  main_cloud.set_sound("./assets/audio/block_destruction.wav", 5);
  main_cloud.set_sound("./assets/audio/fall_Down.wav", 1);
  main_cloud.set_sound("./assets/audio/bosses/phantom/theme.mp3", 1, true);
}

breakout::~breakout() {
  // delete blocks
  for (block *b : blocks) {
    delete b;
  }
  // delete powerups
  delete pu_container;
}

/**
 *
 * DEBUG
 *
 */

// draws crosshair
void breakout::draw_debug_lines() {
  cairo_save(cr);
  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, .6);
  cairo_set_line_width(cr, 1);
  cairo_move_to(cr, width / 2, 0);
  cairo_line_to(cr, width / 2, height);
  cairo_stroke(cr);

  cairo_move_to(cr, 0, height / 2);
  cairo_line_to(cr, width, height / 2);
  cairo_stroke(cr);
  cairo_restore(cr);
}

// draws stat in right lower corner
void breakout::draw_debug_state() {
  cairo_save(cr);

  cairo_move_to(cr, width, height);
  cairo_set_font_size(cr, 10);
  cairo_set_source_rgba(cr, 1, .3, .3, 1);
  cairo_rel_move_to(cr, -20, -10);
  if (state == INIT) {
    cairo_show_text(cr, "I");
  } else if (state == WAIT) {
    cairo_show_text(cr, "W");
  } else if (state == PLAY) {
    cairo_show_text(cr, "P");
  } else if (state == BOSSFIGHT) {
    cairo_show_text(cr, "F");
  } else if (state == GAMEOVER) {
    cairo_show_text(cr, "G");
  }
  cairo_stroke(cr);
  cairo_restore(cr);
}

void breakout::draw_debug_fps() {
  cairo_save(cr);

  int sum = 1;
  for (int i = 0; i < last_frame_time.size(); i++) {
    sum += last_frame_time[i];
  }
  draw_text_centered(cr, 20, height - 10, 10, std::to_string(1000 * last_frame_time.size() / sum) + "fps");

  cairo_restore(cr);
}

// prints keys to console
void breakout::debug_keys() {
  int i = 0;
  for (bool b : key_pressed) {
    if (b) {
      std::cout << "\\     " << i << ": \e[32;1mpressed\e[0m (" << char(i) << "); " << std::endl;
    }
    i++;
  }
}

/**
 *
 * DRAWS
 *
 */

void breakout::draw() {
  // ::cr = cr;
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
#ifdef BREAKOUT_DEBUG_LINES
  draw_debug_lines();
#endif
#ifdef BREAKOUT_DEBUG_STATE
  draw_debug_state();
#endif
#ifdef BREAKOUT_DEBUG_SHOW_FPS
  draw_debug_fps();
#endif
  if (state == INIT) {
    draw_init();
  } else if (state == WAIT) {
    draw_wait();
  } else if (state == PLAY) {
    draw_play();
  } else if (state == BOSSFIGHT) {
    draw_fight();
  } else if (state == GAMEOVER) {
    draw_gameover();
  }
}

// draw initial game state [+ help menu]
void breakout::draw_init() {
  cairo_save(cr);
  // breakout pic
  float scale = .4;
  cairo_scale(cr, scale, scale);
  tex_hub.get_or_create_texel("title", "assets/title.png")->texel_set_source_surface(cr, width / 2, height * (3.0 / 8), scale, scale);
  cairo_paint(cr);
  cairo_scale(cr, 1 / scale, 1 / scale);

  // play text
  // make clip to fill with color for space indicator
  draw_text_centered(cr, width / 2, height * (3.0 / 4), 60, "Play", pixel_font, CENTER, false);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill_preserve(cr); // to fill with white on standart
  cairo_clip(cr);          // clip the text path
  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_rectangle(cr, width / 2 - 80, height * (3.0 / 4) - 60, 145 * init_hold_score, 120);
  cairo_fill(cr);
  cairo_reset_clip(cr);

  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_text_centered(cr, width / 2, height * (3.0 / 4) + 25, 15, "(hold space)", pixel_font);
  // help text
  draw_text_centered(cr, width / 2, height * (27.0 / 32), 40, "Help", pixel_font);
  draw_text_centered(cr, width / 2, height * (27.0 / 32) + 20, 15, "(press h)", pixel_font);

  if (help_menu_open) {
    // paint help menu
    // bg fade
    cairo_set_source_rgba(cr, 1, 1, 1, .3);
    cairo_paint(cr);
    // draw_text_centered(cr, width / 2, height / 2, 100, "HELP MENUA OPEN", pixel_font);
    // bg plane
    cairo_rectangle(cr, width / 4, height / 8, width / 2, height * (6.0 / 8));
    cairo_set_source_rgb(cr, 0, 0, .2);
    cairo_fill(cr);
    // title
    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_text_centered(cr, width / 2, height / 8 + 60, 60, "Help", pixel_font);
    // close
    draw_text_centered(cr, width * 3.0 / 4 - 55, height / 8 + 25, 15, "[esc to close]", pixel_font);
    int left_bound = width / 4 + 20;
    switch (help_page) {
    case 0: { // goal
      cairo_set_source_rgb(cr, 1, 1, 1);
      cairo_set_line_width(cr, 2);
      draw_text_centered(cr, left_bound, height / 8 + 100, 40, "Goal", pixel_font, LEFT, true, true);
      draw_text_centered(cr, left_bound, height / 8 + 130, 25, "Keep the ball from falling and", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 160, 25, "hit as many blocks as possible", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 190, 25, "until you run out of lives.", pixel_font, LEFT);
      // board navigation
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 100, 40, "Controls", pixel_font, LEFT, true, true);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 130, 25, "You can control the paddle with", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 160, 25, "'a' and 'd'.  The initial ball", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 190, 25, "direction can be adjusted with 'w'", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 220, 25, "and 's'.", pixel_font, LEFT);
      // example pic:
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 270, 25, "The board can give the ball a", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 300, 25, "drift, if you hit the ball while", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 330, 25, "the board is moving.  aThis can be", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 360, 25, "used to change the direction of the", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height * 2.0 / 8 + 30 + 390, 25, "ball.", pixel_font, LEFT);
      cairo_set_source_rgb(cr, 1, 1, 1);
      // page nav
      draw_text_centered(cr, width / 2, height * 7 / 8 - 20, 20, "--1-- ", pixel_font, CENTER);
      draw_text_centered(cr, width * 3 / 4 - 15, height * 7 / 8 - 20, 20, "[press d] >", pixel_font, RIGHT);
      break;
    }
    case 1: {
      cairo_set_source_rgb(cr, 1, 1, 1);
      cairo_set_line_width(cr, 2);
      // pus
      float scale;
      draw_text_centered(cr, left_bound, height / 8 + 100, 40, "Powerups", pixel_font, LEFT, true, true);
      draw_text_centered(cr, left_bound, height / 8 + 130, 25, "Will activate on contact with ball.", pixel_font, LEFT);
      // slow/speed-down/up
      draw_text_centered(cr, left_bound, height / 8 + 160, 25, "Slowdown/Speedup:", pixel_font, LEFT);
      scale = 0.7;
      cairo_scale(cr, scale, scale);
      tex_hub.get_or_create_texel("pu_slowdown", "assets/powerups/slowdown.png")->texel_set_source_surface(cr, width * 3 / 4 - 60, height / 8 + 155, scale, scale);
      cairo_paint(cr);
      tex_hub.get_or_create_texel("pu_speedup", "assets/powerups/speedup.png")->texel_set_source_surface(cr, width * 3 / 4 - 25, height / 8 + 155, scale, scale);
      cairo_paint(cr);
      cairo_scale(cr, 1 / scale, 1 / scale);
      cairo_set_source_rgb(cr, 1, 1, 1);
      // blachole
      draw_text_centered(cr, left_bound, height / 8 + 190, 25, "Blackhole:", pixel_font, LEFT);
      cairo_set_source_rgb(cr, 0, .5, 1);
      draw_circle(cr, width * 3 / 4 - 25, height / 8 + 180, get_circle_radius(ani_score, 1500, 1, 13));
      // doubleup
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 220, 25, "Doubleup:", pixel_font, LEFT);
      cairo_set_source_rgb(cr, 1, 1, 1);
      int _x = width * 3 / 4 - 25;
      int _y = height / 8 + 210;
      draw_circle(cr, _x + sinf(ani_score / 200.0) * 5, _y + cosf(ani_score / 200.0) * 5, 5);
      draw_circle(cr, _x + sinf(ani_score / 200.0 + M_PI / 2) * 5, _y + cosf(ani_score / 200.0 + M_PI / 2) * 5, 5, true);
      draw_circle(cr, _x + sinf(ani_score / 200.0 + M_PI) * 5, _y + cosf(ani_score / 200.0 + M_PI) * 5, 5);
      draw_circle(cr, _x + sinf(ani_score / 200.0 + M_PI * 1.5) * 5, _y + cosf(ani_score / 200.0 + M_PI * 1.5) * 5, 5, true);

      // score points
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 250, 25, "Scorepoints:", pixel_font, LEFT);
      scale = 0.7;
      cairo_scale(cr, scale, scale);
      tex_hub.get_or_create_texel("pu_score_plus", "assets/powerups/score_plus.png")->texel_set_source_surface(cr, width * 3 / 4 - 60, height / 8 + 240, scale, scale);
      cairo_paint(cr);
      tex_hub.get_or_create_texel("pu_score_minus", "assets/powerups/score_minus.png")->texel_set_source_surface(cr, width * 3 / 4 - 25, height / 8 + 240, scale, scale);
      cairo_paint(cr);
      cairo_scale(cr, 1 / scale, 1 / scale);

      // lightsout
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 280, 25, "Lights out:", pixel_font, LEFT);
      cairo_set_source_rgb(cr, 1, 1, 0);
      draw_circle(cr, _x, _y + 60, 13);
      draw_circle(cr, _x, _y + 60, 5, (int)(0.5 + get_circle_radius(ani_score, 500, 1, 1)));

      // invincible
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 310, 25, "Invincible:", pixel_font, LEFT);
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_circle(cr, _x, _y + 90, 7, true);
      cairo_set_source_rgb(cr, 66.0 / 255, 218.0 / 255, 245.0 / 255);
      int part_count = 5;
      float offset = ani_score * 0.001 * M_PI;
      for (int i = 0; i < part_count; i++) {
        cairo_arc(cr, _x, _y + 90, 12, ((2.0 * M_PI) / part_count) * i + offset, ((2.0 * M_PI) / part_count) * (i + .5) + offset);
        cairo_stroke(cr);
      }

      // gain life
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 340, 25, "Gain-a-life:", pixel_font, LEFT);
      scale = 0.7;
      cairo_scale(cr, scale, scale);
      tex_hub.get_or_create_texel("pu_gain_live", "assets/powerups/gain_live.png")->texel_set_source_surface(cr, width * 3 / 4 - 25, height / 8 + 330, scale, scale);
      cairo_paint(cr);
      cairo_scale(cr, 1 / scale, 1 / scale);

      // page nav
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, width / 4 + 15, height * 7 / 8 - 20, 20, "< [press a]", pixel_font, LEFT);
      draw_text_centered(cr, width / 2, height * 7 / 8 - 20, 20, "--2-- ", pixel_font, CENTER);
      draw_text_centered(cr, width * 3 / 4 - 15, height * 7 / 8 - 20, 20, "[press d] >", pixel_font, RIGHT);
      break;
    }
    case 2: {
      draw_text_centered(cr, left_bound, height / 8 + 100, 40, "Bossfight", pixel_font, LEFT, true, true);
      draw_text_centered(cr, left_bound, height / 8 + 130, 25, "Once you run out of blocks you will", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 160, 25, "have to fight a boss to keep going", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 190, 25, "and restore all blocks.", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 230, 25, "Phantom", pixel_font, LEFT, true, true);
      draw_text_centered(cr, left_bound, height / 8 + 260, 25, "The one and only current boss.", pixel_font, LEFT);
      draw_text_centered(cr, left_bound, height / 8 + 290, 25, "Act like youre scared!", pixel_font, LEFT);
      // boss pick
      float scale = 0.4;
      cairo_scale(cr, scale, scale);
      tex_hub.get_or_create_texel("boss-phantom_main", "./assets/bosses/phantom/main.png")->texel_set_source_surface(cr, width / 2, height * 5.0 / 8 + 5 - get_circle_radius(ani_score, 1500, 1, 10), scale, scale);
      cairo_paint(cr);
      cairo_scale(cr, 1 / scale, 1 / scale);
      cairo_set_source_rgb(cr, 1, 1, 1);
      draw_text_centered(cr, left_bound, height / 8 + 570, 25, "Music:  Digifunk by DivKid", pixel_font, LEFT);
      // page nav
      draw_text_centered(cr, width / 4 + 15, height * 7 / 8 - 20, 20, "< [press a]", pixel_font, LEFT);
      draw_text_centered(cr, width / 2, height * 7 / 8 - 20, 20, "--3-- ", pixel_font, CENTER);
      break;
    }
    default:
      std::cout << "\e[31;1mERROR INVALID HELP pAGE";
      break;
    }
  }

  // black fadeout
  cairo_set_source_rgba(cr, 0, 0, 0, transition_score);
  cairo_paint(cr);

  cairo_restore(cr);
}

void breakout::draw_wait() {
  // draw board
  boards.front().draw(cr);
  // draw ball
  balls.front()->draw(cr);
  // draw blocks
  for (block *b : blocks) {
    if (!b->hidden) {
      b->draw(cr, *this);
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      b->draw_debug(cr);
#endif
    }
  }

  // draw direction of ball start
  cairo_save(cr);
  cairo_set_source_rgb(cr, indicator_color[0], indicator_color[1], indicator_color[2]);
  // main line
  cairo_move_to(cr, balls.front()->x, balls.front()->y);
  int x = balls.front()->x + sinf(start_angle / 180.0 * M_PI) * indicator_length;
  int y = balls.front()->y - cosf(start_angle / 180.0 * M_PI) * indicator_length;
  cairo_line_to(cr, x, y);
  cairo_stroke(cr);
  // // left arrow
  // cairo_move_to(cr, x, y);
  // int left_x = balls.front().x + sinf(start_angle / 180.0 * M_PI) * (indicator_length)-cosf(start_angle / 180.0 * M_PI) * (20);
  // int left_y = balls.front().y - cosf(start_angle / 180.0 * M_PI) * (indicator_length)-sinf(start_angle / 180.0 * M_PI) * (20);
  // cairo_line_to(cr, left_x, left_y);
  // cairo_stroke(cr);
  // // right arrow
  // cairo_move_to(cr, x, y);
  // int right_x = balls.front().x + sinf(start_angle / 180.0 * M_PI) * (indicator_length) + cosf(start_angle / 180.0 * M_PI) * (20);
  // int right_y = balls.front().y - cosf(start_angle / 180.0 * M_PI) * (indicator_length) + sinf(start_angle / 180.0 * M_PI) * (20);
  // cairo_line_to(cr, right_x, right_y);
  // cairo_stroke(cr);

  // draw lives

  cairo_set_source_rgba(cr, .1, .1, .1, .85);
  cairo_rectangle(cr, width / 4, height * 5.1 / 8, width / 2, height * 0.6 / 8);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_text_centered(cr, width / 2, height * 5.5 / 8, 30, "Lives left: " + std::to_string(lives), pixel_font);
  cairo_restore(cr);
}

void breakout::draw_play() {
  // draw board
  boards.front().draw(cr);
#ifdef BREAKOUT_DEBUG_DRAW_BOARD_COLLISION
  boards.front().draw_debug(cr);
#endif
  // draw balls
  for (ball *b : balls) {
    b->draw(cr);
  }
  // draw blocks
  for (block *b : blocks) {
    if (!b->hidden) {
      b->draw(cr, *this);
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      b->draw_debug(cr);
#endif
    }
  }

  // draw powerups
  pu_container->draw(cr);

  // draw scoring
  draw_score();
  draw_combo();
}

void breakout::draw_fight() {
  boss_cont->hold->draw(cr);
}

void breakout::draw_gameover() {
  // own score
  cairo_set_source_rgb(cr, 1, 0, 0);
  draw_circle(cr, width / 2, height / 3, 70 + (get_circle_radius(ticks_since_show, 180, 1, 10) - 5));
  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_text_centered(cr, width / 2, height / 3 - 90, 30, "Your score");
  cairo_set_source_rgb(cr, .5, .6, .7);
  int score_to_be_shown = (ticks_since_show <= score_ticks) ? powf((float)ticks_since_show / score_ticks, 2) * (float)score : score;
  int size = 28 + ((ticks_since_show <= score_ticks) ? (get_circle_radius(ticks_since_show, 18, 1, 6) - 3) : 0);
  draw_text_centered(cr, width / 2, height / 3 + 3, size, std::to_string(score_to_be_shown));

  // scorebaord
  cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
  cairo_rectangle(cr, width / 4, height / 2 - 10, width * .5, (height / 3) * 0.9);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 1, 1, 1);
  draw_text_centered(cr, width / 2, height / 2 + 30, 40, "Scoreboard", pixel_font, CENTER, true, true);

  int i = 0;
  for (score_struct s : hold) {
    // name
    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_text_centered(cr, width / 4 + 50, height / 2 + 70 + (40 * i), 30, s.name, pixel_font, LEFT, true, false);

    draw_text_centered(cr, width * 3.0 / 4 - 50, height / 2 + 70 + (40 * i), 30, std::to_string(s.score), pixel_font, RIGHT, true, false);
    i++;
  }

  // enter name
  if (player_index >= 0 && !name_entered) {
    int cent_x = width / 2, cent_y = height * .9;
    cairo_set_source_rgba(cr, .3, 0.3, .5, 0.8);
    cairo_rectangle(cr, cent_x - 140, cent_y - 150 / 2, 280, 120);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_text_centered(cr, cent_x, cent_y - 40, 40, "Enter your name:", pixel_font, CENTER);
    draw_text_centered(cr, cent_x, cent_y + 35, 15, "(press enter)", pixel_font, CENTER);
    cairo_set_source_rgba(cr, 0.3, 0.5, 0.3, 0.7);
    cairo_rectangle(cr, cent_x - 105, cent_y - 20, 210, 40);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    draw_text_centered(cr, cent_x, cent_y + 10, 40, player_name + (score_blink_ticks < 500 && player_name.size() < 10 ? "|" : ""), pixel_font, CENTER);
  }
}

void breakout::draw_score() {
  // backround
  cairo_set_source_rgba(cr, .1, .1, .1, .75);
  int rec[2] = {200, 71};
  cairo_rectangle(cr, width / 2 - rec[0] / 2, 0, rec[0], rec[1]);
  cairo_fill(cr);
  // score number
  cairo_set_source_rgba(cr, 1, .1, .1, .8);
  int score_size = 30 + get_circle_radius(update_score_ticks, 5, 1, 5);
  draw_text_centered(cr, width / 2, 33, score_size, "Score: " + std::to_string((int)score), pixel_font);
  // combo number
  int combo_size = 20 + get_circle_radius(update_combo_ticks, 5, 1, 5);
  cairo_set_source_rgba(cr, .1, 1, 1, .8);
  draw_text_centered(cr, width / 2, 55, combo_size, "Combo: x" + std::to_string((int)combo));
  // combo indicator
  // bg
  cairo_set_source_rgba(cr, 1, 1, 1, .75);
  cairo_rectangle(cr, (width / 2) - (rec[0] * 0.5 * 0.8), 60, rec[0] * 0.8, rec[1] * 0.1);
  cairo_fill(cr);
  // inner fill
  // calc color based on combo
  int current_color[3] = {0};
#define FUNC (log2f(combo) / log2f(40))
  // #define FUNC (combo * (1.0f / 40))
  switch ((int)(2 * FUNC)) {
  case 0:
    current_color[0] = (int)((FUNC)*256 * 2);
    current_color[1] = 255;
    break;
  case 1:
    current_color[0] = 255;
    current_color[1] = 255 - (int)((FUNC - 0.5) * 256 * 2);
    break;
  }
  // std::cout << "Color: \e[38;2;" << current_color[0] << ";" << current_color[1] << ";" << current_color[2] << "m" << current_color[0] << ", " << current_color[1] << ", " << current_color[2] << "\e[0m" << std::endl;

  cairo_set_source_rgba(cr, (float)current_color[0] / 255, (float)current_color[1] / 255, (float)current_color[2] / 255, 1);
  int w = rec[0] * 0.8 * (log2f(combo) / log2f(40));
  w = w > rec[0] * 0.8 ? rec[0] * 0.8 : w;
  int h = rec[1] * 0.08;
  int x = width / 2;
  int y = 60;
  cairo_rectangle(cr, x - (w / 2), y + (rec[1] * 0.1 - h) / 2, w, h);
  cairo_fill(cr);
}

void breakout::draw_combo() {
}

/**
 *
 * TICKS
 *
 */

void breakout::tick(int time_diff) {
#ifdef DEBUG_PRINT_KEYS
  debug_keys();
#endif

  // handle keying [a way to get only key taps and not holds]
  int ind = 0;
  for (bool key : key_pressed) {
    if (key && key_hold[ind]) {
      // key is hold down
      key_tap[ind] = false;
    } else if (key && !key_hold[ind]) {
      key_tap[ind] = true;
    } else if (!key && key_hold[ind]) {
      key_tap[ind] = false;
    }
    key_hold[ind] = key;
    ind++;
  }

  this->time_diff = time_diff;
  last_frame_time.push_back(time_diff);
  if (last_frame_time.size() == 30 + 1) {
    last_frame_time.erase(last_frame_time.begin());
  }

  if (state == INIT) {
    tick_init(time_diff);
  } else if (state == WAIT) {
    tick_wait(time_diff);
  } else if (state == PLAY) {
    tick_play(time_diff);
  } else if (state == BOSSFIGHT) {
    tick_fight(time_diff);
  } else if (state == GAMEOVER) {
    tick_gameover(time_diff);
  }
}

void breakout::tick_init(int time_diff) {
  float grow_rate = .001;
  if (key_pressed[' ']) {
    init_hold_score += time_diff * grow_rate;
  } else if (init_hold_score > 0 && transition_score == 0) {
    init_hold_score -= 2 * time_diff * grow_rate;
  }
  if (init_hold_score < 0)
    init_hold_score = 0;
  if (init_hold_score > 1) {
    transition_score += 0.00001;
  }
  if (transition_score > 0) {
    main_cloud.play_sound("./assets/audio/black_out.wav", 1, .8, 0);
    transition_score += time_diff * 0.0005;
  }
  if (transition_score > 1) {
    state = WAIT;
    prep_wait();
  }

  if (key_tap['h']) {
    if (!help_menu_open)
      main_cloud.play_sound("./assets/audio/ui_right.wav", 1, 1, 0);
    help_menu_open = true;
  }
  if (help_menu_open) {
    help_page += key_tap['d'];
    help_page -= key_tap['a'];
    if (help_page >= 0 && help_page <= 2 && (key_tap['d'] || key_tap['a']))
      main_cloud.play_sound("./assets/audio/ui_right.wav", 1, 1, 0);
    if (help_page < 0) {
      help_page = 0;
      main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
    }
    if (help_page > 2) {
      main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
      help_page = 2;
    }
  }
  if (key_pressed[ESC_KEY]) {
    if (help_menu_open)
      main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
    help_menu_open = false;
  }
  ani_score += time_diff;
  if (ani_score > 10000)
    ani_score = 0;
}

void breakout::prep_wait() {
  float board_start_width = width * 0.15;
  boards.push_back(board(width / 2 - board_start_width / 2, height * .95, board_start_width, 20));
  for (int i = 0; i < 1; i++) {
    ball *b = new ball;
    b->position(width / 2, height * .95 - b->rad);
    b->direction(0, 1);
    balls.push_back(b);
  }
  std::cout << "Cells x: " << cells_x << ", Cells y: " << cells_y << std::endl;
  for (block *b : blocks) {
    std::cout << "del" << std::endl;
    delete b;
  }
  for (int i = 0; i < cells_y; i++) {
    int y = i * (height * .5) / cells_y; // .5 is max used space
    for (int j = 0; j < cells_x; j++) {
      int x = j * (width / cells_x);
      int w = (width / cells_x);
      int h = ((height * .5) / cells_y);
      int used_space_percentage_w = std::min(space * w, (float)max_space);
      int used_space_percentage_h = std::min(space * h, (float)max_space);
      block *b = new block(x + used_space_percentage_w, y + used_space_percentage_h, w - 2 * used_space_percentage_w, h - 2 * used_space_percentage_h, color((float)j / cells_x, (float)i / cells_y, 0.4), cr);
      std::cout << "New Block: [" << x << "," << y << "][" << w << "," << h << "]" << std::endl;
      blocks.push_back(b);
    }
  }

  // init powerups
  int nr_of_places = 6;
  int places[nr_of_places][2];

  for (int i = 0; i < nr_of_places; i++) {
    places[i][0] = width * (((i % 3) % 3) + 1) / 4;
    places[i][1] = height * (0.3 + (i / 3) * 0.4);
  }

  pu_container = new powerup::power_up_container(this, nr_of_places, nr_of_places, nr_of_places, places);
}

void breakout::tick_wait(int time_diff) {
  // move board
  if (key_pressed['a']) {
    boards.front().move_left(boards.front().speed * time_diff);
    for (ball *b : balls) {
      b->position(boards.front().x + boards.front().w / 2, boards.front().y - balls.front()->rad);
    }
  }
  if (key_pressed['d']) {
    boards.front().move_right(boards.front().speed * time_diff, width);
    for (ball *b : balls) {
      b->position(boards.front().x + boards.front().w / 2, boards.front().y - balls.front()->rad);
    }
  }

  // change start direction
  if (key_pressed['w']) {
    move_start_dir(-time_diff);
  }
  if (key_pressed['s']) {
    move_start_dir(time_diff);
  }
  boards.front().velocity = 0;
  // start game
  if (key_tap[' ']) {
    main_cloud.play_sound("./assets/audio/ball_release.wav", 1, 1, 0);
    state = PLAY;
    prep_play();
  }
}

void breakout::prep_play() {
  // for (ball &b : balls) {
  //   b.use_random_start_dir(10, 10);
  // }
}

void breakout::tick_play(int time_diff) {
  // handle input
  if (key_pressed['a']) {
    boards.front().move_left(boards.front().speed * time_diff);
  }
  if (key_pressed['d']) {
    boards.front().move_right(boards.front().speed * time_diff, width);
  }

  boards.front().tick(time_diff);

  // move balls and handle collision between ball and blocks,walls,board and powerups
  std::vector<int> to_be_deleted; // sorted list of delete indexes
  int start_size = balls.size();
  for (int i = 0; i < start_size; i++) {
    ball *b = balls[i];
    b->tick(time_diff);
    if (b->life_time <= 0) {
      std::cout << "\e[31;1mlife:\e[0m " << b->life_time << std::endl;
      to_be_deleted.push_back(i);
    }
    if (!b->check_collision(*this, time_diff)) {
      lives--;
      main_cloud.play_sound("./assets/audio/fall_Down.wav", 1, .8 + random_number(0, .4), 0);
      if (lives == 0) {
        prep_gameover();
        state = GAMEOVER;
      } else {
        restore_game();
        state = WAIT;
      }
      break;
    }
  }
  if (to_be_deleted.size())
    std::cout << "Ball Delete: " << std::endl;
  std::reverse(to_be_deleted.begin(), to_be_deleted.end());
  for (int ind : to_be_deleted) {
    std::cout << ind << " ";
    delete_element_in_vector(ind, balls);
  }

  /**
   *
   * POWERUP HANDLING
   *
   */

  pu_container->tick(time_diff);

  // scoring
  tick_combo(time_diff);

  // if all blocks are hidden=hit, then start bossfight
  bool at_least_one_alive = false;
  for (block *b : blocks) {
    at_least_one_alive |= !b->hidden;
  }
  if (!at_least_one_alive) {
    prep_fight();
    state = BOSSFIGHT;
  }
}

void breakout::prep_fight() {

  std::cout << "PREP FIGHT" << std::endl;
  boss_cont = new bossfight::boss_container();
  boss_cont->summon_boss(this);
}

void breakout::tick_fight(int time_diff) {
  boss_cont->hold->tick(time_diff);
  if (boss_cont->hold->finished) {
    std::cout << "Done!" << std::endl;
    restore_game();
    for (block *b : blocks) {
      b->hidden = false;
      b->lives = BREAKOUT_BLOCK_MAX_LIVES;
    }
    state = WAIT;
    if (lives <= 0) {
      prep_gameover();
      state = GAMEOVER;
    }
  }
}

bool breakout::score_compare(score_struct i, score_struct j) { return (i.score > j.score); }

void breakout::prep_gameover() {
  std::cout << "Gameover Prep" << std::endl;
  // open or create
  std::fstream file("scoreboard");
  std::string lines[5] = {""};

  int ind = 0;
  while (ind < 5 && getline(file, lines[ind++]))
    ;
  for (int i = 0; i < ind; i++) {
    std::stringstream str(lines[i]);
    std::string name;
    getline(str, name, ' ');
    std::cout << "n: " << name << std::endl;
    std::string score_str;
    getline(str, score_str);
    std::cout << "s: " << score_str << std::endl;
    hold.push_back(score_struct(name, std::stoi(score_str)));
  }
  for (score_struct s : hold) {
    std::cout << "Name: " << s.name << ", score: " << s.score << std::endl;
  }
  std::sort(hold.begin(), hold.end(), score_compare);
  if (score > (hold.end() - 1)->score) {
    player_index = 0;
    hold.push_back(score_struct("YOUR NAME", score));
    std::sort(hold.begin(), hold.end(), score_compare);
    for (score_struct s : hold) {
      if (s.name == "YOUR NAME")
        break;
      else
        player_index++;
    }
    hold.pop_back();
  }
}

void breakout::tick_gameover(int time_diff) {
  // animation handling
  if (!finished) {
    ticks_since_show++;
    if (ticks_since_show >= max_ticks) {
      finished = true;
    }
  }

  // if score good, enter name
  if (player_index >= 0 && !name_entered) {
    score_blink_ticks -= time_diff;
    if (score_blink_ticks <= 0)
      score_blink_ticks = 1000;

    // pressed enter write name to file
    if (key_pressed[13]) {
      if (player_name.size() > 0) {
        hold[player_index].name = player_name;
        name_entered = true;
        std::cout << "name is: " << player_name << std::endl;
        // save names to file
        std::ofstream file("scoreboard");
        for (score_struct s : hold)
          file << s;
        file.close();
      } else
        main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
    }
    // check for key press and add to string otherwise
    int i = 0;
    for (bool b : key_tap) {
      if (b) {

        if (i == 8) { // backspace
          if (player_name.size() > 0)
            player_name.pop_back();
          else
            main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
        } else if (player_name.size() == 10) {
          // too long
          main_cloud.play_sound("./assets/audio/ui_wrong.wav", 1, 1, 0);
          std::cout << "name too long" << std::endl;
        } else if ((i >= 'a' && i <= 'z') || ((i >= 'A' && i <= 'Z') || (i >= '0' && i <= '9')) || (i == '_')) {
          main_cloud.play_sound("./assets/audio/ui_right.wav", 1, 1, 0);
          player_name += char(i);
        }
      }
      i++;
    }
  }

  // esc to close
  if (key_pressed[ESC_KEY]) {
    quit_gui();
  }
}

void breakout::restore_game() {
  // wash blocks
  // commented out caue boss fights dont need resets anymore
  // for (block *b : blocks) {
  //   b->hidden = false;
  //   b->lives = BREAKOUT_BLOCK_MAX_LIVES;
  // }
  // tidy balls
  std::vector<int> ind;
  int i = 0;
  for (ball *b : balls) {
    b->position(width / 2, height * .95 - b->rad);
    b->spin_add = 0;
    b->direction(0, 1);
    if (!b->main) {
      ind.push_back(i);
    }
    i++;
  }
  std::reverse(ind.begin(), ind.end());
  for (int i : ind)
    delete_element_in_vector(i, balls);
  // refresh angle
  start_angle = 0;
  // order boards
  for (board &b : boards) {
    b.x = width / 2 - (width * 0.15) / 2;
    b.y = height * .95;
  }
  // clean powerups
  pu_container->delete_all();
  // launder timer
  pu_container->ticks_until_next_pu = pu_container->interval_time * 2;
}

void breakout::move_start_dir(float by) {
  start_angle += by * 0.1;
  if (start_angle < max_angle[0]) {
    start_angle = max_angle[0];
  }
  if (start_angle > max_angle[1]) {
    start_angle = max_angle[1];
  }
  // calculate direction of ball
  for (ball *b : balls) {
    int x = sinf(start_angle / 180.0 * M_PI) * indicator_length;
    int y = cosf(start_angle / 180.0 * M_PI) * indicator_length;
    b->direction(x, -y);
  }
}

// score handling
void breakout::tick_combo(int time_diff) {
  if (ticks_since_last_combo_hit > combo_duration) {
    combo = 1;
  } else {
    ticks_since_last_combo_hit += time_diff;
  }
  if (update_score_ticks > 0)
    update_score_ticks--;
  if (update_combo_ticks > 0)
    update_combo_ticks--;
}

void breakout::add_score(float amount) {
  score += combo * amount;
  if (score < 0)
    score = 0;
  update_score_ticks = 15;
}

void breakout::add_combo(float amount) {
  ticks_since_last_combo_hit = 0;
  combo += amount;
  update_combo_ticks = 15;
}

/**
 *
 * * BALL
 *
 */

void ball::tick(int time_diff) {
  step(time_diff);
  if (!main)
    life_time -= time_diff;
  spin_add -= 0.01;
  if (spin_add < 0)
    spin_add = 0;
}

void ball::position(float posx, float posy) {
  x = posx;
  y = posy;
}

void ball::direction(float dirx, float diry, bool noise) {
  // vector normed to |v|=1
  if (noise) {
    dirx += (random_number(0, 1) - 1) / 100;
    diry += (random_number(0, 1) - 1) / 100;
  }
  // board spin
  dirx += spin_add * 1.5;

  dx = dirx != 0 ? dirx / sqrt(dirx * dirx + diry * diry) : 0;
  dy = diry != 0 ? diry / sqrt(dirx * dirx + diry * diry) : 0;
  // std::cout << "Set Dir to: [" << dx << ";" << dy << "]" << std::endl;
}

void ball::draw(cairo_t *cr) {
  cairo_save(cr);
  if (!invincible) {
    cairo_set_source_rgba(cr, 1, 1, main ? 1 : 0.8, main ? 1 : 0.7);
    cairo_arc(cr, x, y, rad, 0, 2 * M_PI);
    cairo_fill(cr);
  } else {
    cairo_set_source_rgba(cr, 1, 1, main ? 1 : 0.8, main ? 1 : 0.7);
    draw_circle(cr, x, y, rad, true);
    cairo_set_source_rgb(cr, 66.0 / 255, 218.0 / 255, 245.0 / 255);
    int part_count = 5;
    // float offset = lifetime * 0.003 * M_PI;
    for (int i = 0; i < part_count; i++) {
      cairo_arc(cr, x, y, rad + 5, ((2.0 * M_PI) / part_count) * i, ((2.0 * M_PI) / part_count) * (i + .5));
      cairo_stroke(cr);
    }
  }
  cairo_restore(cr);
}

void ball::use_random_start_dir(float scale_x, float scale_y) {
  direction(-random_number() * scale_x, -random_number() * scale_y);
}

void ball::step(float gamespeed) {
  // std::cout << "step" << std::endl;
  last_good_x = x;
  last_good_y = y;
  x += dx * gamespeed * speed;
  y += dy * gamespeed * speed;
}

// returns true if bounced from walls or sth; false when ball left area
bool ball::check_collision(breakout &game, int time_diff) {
  // wall collision
  if (x >= game.get_width()) {
    direction(-dx, dy);
    position(last_good_x, last_good_y);
    last_hit = nullptr;
    game.main_cloud.play_sound("./assets/audio/ball_hit.wav", 1, 1, 0);
  }
  if (x <= 0) {
    direction(-dx, dy);
    position(last_good_x, last_good_y);
    last_hit = nullptr;
    game.main_cloud.play_sound("./assets/audio/ball_hit.wav", 1, 1, 0);
  }
  if (y <= 0) {
    direction(dx, -dy);
    position(last_good_x, last_good_y);
    last_hit = nullptr;
    game.main_cloud.play_sound("./assets/audio/ball_hit.wav", 1, 1, 0);
  }
  if (y >= game.get_height()) {
    // direction(dx, -dy);
    // position(last_good_x, last_good_y);
    last_hit = nullptr;
    if (main && !invincible)
      return false;
    else if (!invincible) {
      life_time = -1;
    } else {
      direction(dx, -dy);
      position(last_good_x, last_good_y);
      last_hit = nullptr;
    }
  }

  float dir[2] = {dx, dy};
  float last[2] = {last_good_x,
                   last_good_y};

  // board collision
  for (board &b : game.get_boards()) {
    if (b.collides(x, y, last, dir, speed)) {
      b.on_collision(game);
      std::cout << b.velocity << std::endl;
      spin_add = b.velocity;
      direction(dx * dir[0], dy * dir[1], true);
      position(last_good_x, last_good_y);
      step(1);
      last_hit = nullptr;
      game.main_cloud.play_sound("./assets/audio/ball_hit.wav", 1, 1, 0);
    }
  }

  // block collisions
  dir[0] = dx;
  dir[1] = dy;
  for (block *b : game.get_blocks()) {
    if (!b->hidden && b->collides(x, y, last, dir, speed) && last_hit != b) {
      // std::cout << "Last good: " << last_good_x << "," << last_good_y << std::endl;
      b->on_collision(game);
      direction(dx * dir[0], dy * dir[1]);
      position(last_good_x, last_good_y);
      step(1);
      last_hit = b;
      if (b->lives)
        game.main_cloud.play_sound("./assets/audio/ball_hit.wav", 1, 1, 0);
      else
        game.main_cloud.play_sound("./assets/audio/block_destruction.wav", 1, 1, 0);
      break;
    }
  }

  // powerup collision
  game.pu_container->collision(this);

  // boss collision
  if (game.boss_cont && game.boss_cont->hold) {
    game.boss_cont->hold->collision(this);
  }

  return true;
}

std::ostream &operator<<(std::ostream &os, const ball &b) {
  os << "\e[34;1mBall\e[0m" << std::endl
     << "  x=" << b.x << ", y=" << b.y << ", r=" << b.rad << ", "
     << "  dx=" << b.dx << ", dy=" << b.dy << ", speed=" << b.speed << ", "
     << "  lx=" << b.last_good_x << ", ly=" << b.last_good_y << ", "
     << "  lifetime=" << b.life_time << ", main=" << (b.main ? "true" : "false") << std::endl;
  return os;
}

/**
 *
 * BOARD
 *
 */

void board::draw(cairo_t *cr) {
  cairo_save(cr);

  col.use(cr);
  cairo_move_to(cr, x, y);
  cairo_rectangle(cr, x, y, w, h);

  cairo_fill(cr);

  cairo_restore(cr);
}

void board::move_left(float by) {
  x -= by;
  if (x < 0)
    x = 0;
  else
    velocity -= by * 0.01;

  if (velocity > 1)
    velocity = 1;
  else if (velocity < -1)
    velocity = -1;
}

void board::move_right(float by, int width) {
  x += by;
  if (x + w > width)
    x = width - w;
  else
    velocity += by * 0.01;

  if (velocity > 1)
    velocity = 1;
  else if (velocity < -1)
    velocity = -1;
}

#ifdef BREAKOUT_DEBUG_DRAW_BOARD_COLLISION
void board::draw_debug(cairo_t *cr) {
  cairo_save(cr);
  for (std::vector<float> f : debug_lines) {
    cairo_set_source_rgb(cr, f[0], f[1], f[2]);
    cairo_move_to(cr, f[3], f[4]);
    cairo_line_to(cr, f[5], f[6]);
    cairo_stroke(cr);
  }
  for (std::vector<float> f : debug_points) {
    cairo_set_source_rgb(cr, f[0], f[1], f[2]);
    draw_circle(cr, f[3], f[4], (f[0] == 12 && f[1] == 13 && f[2] == 11) ? f[5] : 15);
  }
  cairo_restore(cr);
  debug_points.clear();
  debug_lines.clear();
}
#endif

// _x, _y => point to check collision; dir is direction change to moving entity
bool board::collides(int _x, int _y, float last_good_pos[2], float dir[2], float speed) {
  // std::cout << "_y: " << _y << ", y: " << y << "; _x: " << _x << "; x: " << x << "; wx: " << w + x << "; dy: " << dir[1];
  if (_y >= y && _y <= y + 1.3 * h && _x >= x - 5 && _x <= x + w + 5 && dir[1] >= 0) {
    dir[0] = 1;
    dir[1] = -1;
    return true;
  }
  dir[0] = 1;
  dir[1] = 1;
  return false;
}
// remove block,
void board::on_collision(breakout &game) const {
  // lives--;
  // hidden = lives != 0 ? false : true;
  // game.add_score((lives == 0) ? 20 : 10);
  // game.add_combo((lives == 0) ? 2 : 1);
}

void board::tick(int time_diff) {
  if (velocity < 0)
    velocity += 0.05;
  else if (velocity > 0)
    velocity -= 0.05;
}

/**
 *
 * BLOCKS
 *
 */

void block::draw(cairo_t *cr, breakout &game) {
  cairo_save(cr);
  float live_multiplier = lives * 1.0 / BREAKOUT_BLOCK_MAX_LIVES;
  cairo_set_source_rgb(cr, col.r * live_multiplier, col.g * live_multiplier, col.b * live_multiplier);
  cairo_rectangle(cr, x, y, w, h);
  cairo_fill(cr);

  float scale_x = (w)*1.0 / cairo_image_surface_get_width(game.tex_hub.get_or_create_texel("block_crack_2", "assets/cracks/2.png")->img);
  float scale_y = (h)*1.0 / cairo_image_surface_get_height(game.tex_hub.get_or_create_texel("block_crack_2", "assets/cracks/2.png")->img);
  cairo_scale(cr, scale_x, scale_y);
  switch (lives) {
  case 1:
    game.tex_hub.get_texel("block_crack_2")->texel_set_source_surface(cr, x, y, scale_x, scale_y, false);
    cairo_paint(cr);
    break;
  case 2:
    game.tex_hub.get_or_create_texel("block_crack_1", "assets/cracks/1.png")->texel_set_source_surface(cr, x, y, scale_x, scale_y, false);
    cairo_paint(cr);
    break;
  default:
    break;
  }
  cairo_scale(cr, 1 / scale_x, 1 / scale_y);

#ifdef BREAKOUT_DEBUG_SHOW_LIVES
  // lives
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_select_font_face(cr, "Purisa",
                         CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 15);
  cairo_text_extents_t te;
  char lives_text[1];
  lives_text[0] = '0' + lives;
  cairo_text_extents(cr, lives_text, &te);
  cairo_move_to(cr, (x + w / 2) - te.x_advance / 2, y + h / 2);
  cairo_show_text(cr, lives_text);
  cairo_stroke(cr);
#endif
  cairo_restore(cr);
}

#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
void block::draw_debug(cairo_t *cr) {
  cairo_save(cr);
  for (std::vector<float> f : debug_lines) {
    cairo_set_source_rgb(cr, f[0], f[1], f[2]);
    cairo_move_to(cr, f[3], f[4]);
    cairo_line_to(cr, f[5], f[6]);
    cairo_stroke(cr);
  }
  for (std::vector<float> f : debug_points) {
    cairo_set_source_rgb(cr, f[0], f[1], f[2]);
    draw_circle(cr, f[3], f[4], (f[0] == 12 && f[1] == 13 && f[2] == 11) ? f[5] : 15);
  }
  cairo_restore(cr);
  debug_points.clear();
  debug_lines.clear();
}
#endif
// _x, _y => point to check collision; dir is direction change to moving entity
bool block::collides(int _x, int _y, float last_good_pos[2], float dir[2], float speed) {
  // new code with line interception:
  // points
  float lx = last_good_pos[0];
  float ly = last_good_pos[1];

  std::vector<std::vector<float>> hits; // 2d points of hits
  float hit[2] = {0};
  Intersection res = NONE;
  // bottom
  res = line_intersection(lx, ly, _x, _y, x, y + h, x + w, y + h, hit);
  if (res) {
    if (distance_between_points(hit[0], hit[1], lx, ly) <= distance_between_points(lx, ly, _x, _y) && (hit[0] >= x && hit[0] <= x + w)) {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      std::cout << "Hit BOT at: [" << hit[0] << "," << hit[1] << "]" << std::endl;
#endif
      hits.push_back({hit[0], hit[1]});
      dir[0] = 1;
      dir[1] = -1;
    } else {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({1, 1, 0, hit[0], hit[1]});
      debug_lines.push_back({1, 1, 0, hit[0], hit[1], lx, ly});
#endif
    }
  }
  // top
  res = line_intersection(lx, ly, _x, _y, x, y, x + w, y, hit);
  if (res) {
    if (distance_between_points(hit[0], hit[1], lx, ly) <= distance_between_points(lx, ly, _x, _y) && (hit[0] >= x && hit[0] <= x + w)) {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({1, 0, 0, hit[0], hit[1]});
      std::cout << "Hit TOP at: [" << hit[0] << "," << hit[1] << "]" << std::endl;
#endif
      hits.push_back({hit[0], hit[1]});
      dir[0] = 1;
      dir[1] = -1;
    } else {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({1, .5, 0, hit[0], hit[1]});
      debug_lines.push_back({1, .5, 0, hit[0], hit[1], lx, ly});
#endif
    }
  }
  // left
  res = line_intersection(lx, ly, _x, _y, x, y, x, y + h, hit);
  if (res) {
    if (distance_between_points(hit[0], hit[1], lx, ly) <= distance_between_points(lx, ly, _x, _y) && (hit[1] >= y && hit[1] <= y + h)) {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({1, 0, 0, hit[0], hit[1]});
      std::cout << "Hit LEFT at: [" << hit[0] << "," << hit[1] << "]" << std::endl;
#endif
      hits.push_back({hit[0], hit[1]});
      dir[0] = -1;
      dir[1] = 1;
    } else {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({.5, 1, 0, hit[0], hit[1]});
      debug_lines.push_back({.5, 1, 0, hit[0], hit[1], lx, ly});
#endif
    }
  }
  // right
  res = line_intersection(lx, ly, _x, _y, x + w, y, x + w, y + h, hit);
  if (res) {
    if (distance_between_points(hit[0], hit[1], lx, ly) <= distance_between_points(lx, ly, _x, _y) && (hit[1] >= y && hit[1] <= y + h)) {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({1, 0, 0, hit[0], hit[1]});
      std::cout << "Hit at: [" << hit[0] << "," << hit[1] << "]" << std::endl;
#endif
      hits.push_back({hit[0], hit[1]});
      dir[0] = -1;
      dir[1] = 1;
    } else {
#ifdef BREAKOUT_DEBUG_DRAW_BLOCK_COLLISION
      debug_points.push_back({.5, .5, .5, hit[0], hit[1]});
      debug_lines.push_back({.5, .5, .5, hit[0], hit[1], lx, ly});
#endif
    }
  }
  return hits.size();
}

// remove block,
void block::on_collision(breakout &game) {
  lives--;
  hidden = lives != 0 ? false : true;
  game.add_score((lives == 0) ? 20 : 10);
  game.add_combo((lives == 0) ? 2 : 1);
}
