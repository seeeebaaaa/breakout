#include "gui.h"
#include "breakout_config.h"
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

static gint64 timer_start = 0;
static gint64 last_time = 0;

static gboolean tick(GtkWidget *widget, GdkFrameClock *clock, gpointer user_data) {
  if (timer_start == 0) {
    timer_start = gdk_frame_clock_get_frame_time(clock);
    last_time = 0;
    return TRUE;
  }
  gint64 time = (gdk_frame_clock_get_frame_time(clock) - timer_start) / 1000;
  gint64 time_diff = time - last_time;

  windowed_app *app = (windowed_app *)user_data;
  app->tick(time_diff);

  gtk_widget_queue_draw(widget);
#ifdef BREAKOUT_SLOW_TICK
  std::this_thread::sleep_for(std::chrono::milliseconds(BREAKOUT_SLOW_TICK));
  last_time = time + BREAKOUT_SLOW_TICK;
#else
  last_time = time;
#endif
  return TRUE;
}

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  GtkWidget *win = gtk_widget_get_toplevel(widget);
  int width, height;
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);

  windowed_app *app = (windowed_app *)user_data;
  app->width = width;
  app->height = height;
  app->cr = cr;
  app->draw();
  app->cr = nullptr;
  return TRUE;
}

static int pressed[256];

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
  windowed_app *app = (windowed_app *)user_data;
  if (strlen(event->string) > 0) {
    app->key_down(event->string[0]);
    return TRUE;
  }
  return FALSE; // to fall through
}

static gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
  windowed_app *app = (windowed_app *)user_data;
  if (strlen(event->string) > 0) {
    app->key_up(event->string[0]);
    return TRUE;
  }
  return FALSE;
}

windowed_app::windowed_app(int w, int h, const std::string &name) : width(w), height(h) {
  char *args[] = {(char *)"ui-prog", nullptr};
  char **argv = args;
  int argc = 1;
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkCssProvider *css_prov = gtk_css_provider_new();
  const char *css = R"(* { background: rgb(0,0,172);
  background: linear-gradient(180deg, rgba(0,0,80,1) 0%, rgba(0,0,43,1) 75%);
}
)";
  gtk_css_provider_load_from_data(css_prov, css, -1, nullptr);
  GtkStyleContext *style_ctx = gtk_widget_get_style_context(window);
  gtk_style_context_add_provider(style_ctx, GTK_STYLE_PROVIDER(css_prov), GTK_STYLE_PROVIDER_PRIORITY_USER);

  GtkWidget *darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);

  gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

  g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(::draw), this);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), this);
  g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(on_key_press), this);
  g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(on_key_release), this);
  gtk_widget_add_tick_callback(window, ::tick, this, nullptr);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), w, h);
  gtk_window_set_title(GTK_WINDOW(window), name.c_str());

  for (bool &c : key_pressed)
    c = false;
  gtk_window_set_resizable(GTK_WINDOW(window), false);
}

void windowed_app::run() {
  gtk_widget_show_all(window);
  gtk_main();
}

void quit_gui() {
  gtk_main_quit();
}

void draw_circle(cairo_t *cr, int x, int y, int rad, bool fill) {
  cairo_arc(cr, x, y, rad, 0, 2 * M_PI);
  if (fill)
    cairo_fill(cr);
  else
    cairo_stroke(cr);
}

float random_number(float min, float max) {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  std::uniform_real_distribution<> dist(min, max);
  return dist(mt);
}
