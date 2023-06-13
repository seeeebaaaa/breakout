#pragma once
#include <cairo.h>
#include <ft2build.h>
#include <map>
#include <string>
#include FT_FREETYPE_H
#include <cairo-ft.h>

struct texel {
public:
  // explicit texel(cairo_surface_t *p) : img(p){};
  // ~texel() {
  //   cairo_surface_destroy(img);
  // };
  cairo_surface_t *img;
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