#pragma once

#include <cairo.h>
#include <gtk/gtk.h>
#include <string>

class windowed_app {
protected:
  GtkWidget *window = nullptr;
  int width, height;
  cairo_t *cr = nullptr;
  bool key_pressed[256];
  enum { ESC_KEY = 033 };

public:
  virtual void tick(int time_diff) = 0;
  virtual void draw() = 0;
  virtual void key_down(char c) { key_pressed[c] = true; }
  virtual void key_up(char c) { key_pressed[c] = false; }
  windowed_app(int w, int h, const std::string &name);
  void run();

private:
  friend gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
};

float random_number(float min = 0, float max = 1);

void draw_circle(cairo_t *cr, int x, int y, int rad, bool fill = false);

void quit_gui();
