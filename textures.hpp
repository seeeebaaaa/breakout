#pragma once
#include <cairo.h>
#include <map>
#include <string>

struct texel {
  // explicit texel(cairo_surface_t *p) : img(p){};
  ~texel() {
    cairo_surface_destroy(img);
  };
  cairo_surface_t *img = nullptr;
  void texel_set_source_surface(cairo_t *cr, int x, int y, float scale_x, float scale_y, bool centered = true); //  cairo context, position x & y, scale of image, when centered then pos is coord of image center
};

class texture_hub {
  std::map<std::string, texel *> container;

public:
  texture_hub();
  void add_texture(const std::string &key, const std::string &filename);           // adds a new texture or throws error if key exits already
  texel *get_texel(const std::string &key);                                        // returns texture pointer or error when key not found
  texel *get_or_create_texel(const std::string &key, const std::string &filename); // returns texture pointer or creates texture when key not found
  void delete_texel(const std::string &key);                                       // deletes a texel entry from the map
  // void remove_texture(std::string key, std::string filename);
  ~texture_hub();
};