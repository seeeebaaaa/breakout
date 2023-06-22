#include "util.h"
#include "breakout.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <string>

Intersection line_intersection(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2, float intersection[2]) {
  float denominator = (ax1 - ax2) * (by1 - by2) - (ay1 - ay2) * (bx1 - bx2);

  if (fabs(denominator) <= __FLT_EPSILON__) {
    // either parallel or coincident => check for one point
    // calculate distance of b1 to ab(x) if ~0 then infinite
    float distance = (fabs((ax2 - ax1) * (ay1 - by1) - (ax1 - bx1) * (ay2 - ay1))) / sqrtf((ax2 - ax1) * (ax2 - ax1) + (ay2 - ay1) * (ay2 - ay1));
    if (distance <= __FLT_EPSILON__)
      return INFINITE;
    return NONE;
  }
  // one intersection
  intersection[0] = ((ax1 * ay2 - ay1 * ax2) * (bx1 - bx2) - (ax1 - ax2) * (bx1 * by2 - by1 * bx2)) / denominator;
  intersection[1] = ((ax1 * ay2 - ay1 * ax2) * (by1 - by2) - (ay1 - ay2) * (bx1 * by2 - by1 * bx2)) / denominator;
  return ONE;
}

float distance_between_points(float x1, float y1, float x2, float y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float distance_between_line_and_point(float line_a[2], float line_b[2], float p[2]) {
  return fabs((line_b[0] - line_a[0]) * (line_a[1] - p[1]) - (line_a[0] - p[0]) * (line_b[1] - line_a[1])) / sqrt(powf(line_b[0] - line_a[0], 2) + powf(line_b[1] - line_a[1], 2));
}

/**
 * Returns radius of circle for animation
 */
float get_circle_radius(int time, int interval, float speed, float max_rad) {
  // scale time frame from 0 to pi
  float x = (float)(time % interval) / interval * M_PI;
  return fabs(sinf(x * speed) * max_rad);
}

void draw_text_centered(cairo_t *cr, int x, int y, int size, const std::string &text, cairo_font_face_t *font, ALIGNMENT align, bool stroke, bool underline) {
  if (font) {
    cairo_set_font_face(cr, font);
  } else {
    cairo_select_font_face(cr, "Purisa",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
  }
  cairo_set_font_size(cr, size);
  cairo_text_extents_t te;
  cairo_text_extents(cr, text.c_str(), &te);
  switch (align) {
  case CENTER:
    cairo_move_to(cr, x - te.x_advance / 2, y + te.y_advance / 2);
    break;
  case LEFT:
    cairo_move_to(cr, x, y + te.y_advance / 2);
    break;
  case RIGHT:
    cairo_move_to(cr, x - te.x_advance, y + te.y_advance / 2);
  default:
    break;
  }
  if (stroke) {
    cairo_show_text(cr, text.c_str());
    cairo_stroke(cr);
    if (underline) {
      switch (align) {
      case CENTER:
        cairo_move_to(cr, x - te.x_advance / 2, y + te.height / 2 - (size / 4));
        break;
      case LEFT:
        cairo_move_to(cr, x, y + te.height / 2 - (size / 4));
        break;
      case RIGHT:
        cairo_move_to(cr, x - te.x_advance, y + te.height / 2 - (size / 4));
        break;
      }
      cairo_rel_line_to(cr, te.x_advance, 0);
      cairo_stroke(cr);
    }
  } else {
    cairo_text_path(cr, text.c_str());
    if (underline) {
      switch (align) {
      case CENTER:
        cairo_move_to(cr, x - te.x_advance / 2, y + te.height / 2 - (size / 10));
        break;
      case LEFT:
        cairo_move_to(cr, x, y + te.height / 2 - (size / 10));
        break;
      case RIGHT:
        cairo_move_to(cr, x - te.x_advance, y + te.height / 2 - (size / 10));
        break;
      }
      cairo_rel_line_to(cr, te.x_advance, 0);
    }
  }
}
