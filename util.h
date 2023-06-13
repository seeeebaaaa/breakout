#ifndef UTIL_H
#define UTIL_H
#include "breakout.h"
enum Intersection { NONE,
                    ONE,
                    INFINITE };

/**
 * Intersection between two lines
 * Each line is described by two distinct points p1 and p2
 * Type of intersection is returned
 * Point of intersection is stored in intersection
 */
Intersection line_intersection(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2, float intersection[2]);

float distance_between_points(float x1, float y1, float x2, float y2);

float distance_between_line_and_point(float line_a[2], float line_b[2], float p[2]); // g(x): x=a+t*b

float get_circle_radius(int time, int interval, float speed, float max_rad);
enum ALIGNMENT { LEFT,
                 CENTER,
                 RIGHT };

void draw_text_centered(cairo_t *cr, int x, int y, int size, const std::string &text, cairo_font_face_t *font = nullptr, ALIGNMENT align = CENTER, bool stroke = true, bool underline = false); // if stroke is true, then fills text and uses caching, if false then only paths the text without caching

template <typename T>
void delete_element_in_vector(int i, std::vector<T> &a) {
  typename std::vector<T>::iterator q;
  q = a.begin();
  q += i;
  a.erase(q);
}
#endif