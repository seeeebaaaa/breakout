#include "textures.hpp"
#include <cairo.h>
#include <iostream>
#include <map>
#include <string>

void texture_hub::add_texture(const std::string &key, const std::string &filename) {
  std::cout << "\e[34;1mAdded new texture \"" << key << "\" from file '" << filename << "'\e[0m" << std::endl;
  if (container.find(key) != container.end()) {
    std::cout << "\e[31;1mERROR: key exists when adding\e[0m" << std::endl;
    throw new std::invalid_argument("TEXTURES: Key " + key + " already exits.");
  }
  cairo_surface_t *imageSurface = cairo_image_surface_create_from_png(filename.c_str());
  texel *hold = new texel;
  hold->img = imageSurface;
  container[key] = hold;
}

texel *texture_hub::get_texel(const std::string &key) {
  if (auto res = container.find(key); res == container.end()) {
    std::cout << "\e[31;1mERROR: key does not exists when getting\e[0m" << std::endl;
    throw new std::invalid_argument("TEXTURES: Key " + key + " was not found.");
  }

  return container[key];
}

texel *texture_hub::get_or_create_texel(const std::string &key, const std::string &filename) {
  if (container.find(key) != container.end())
    return container[key];
  else {
    add_texture(key, filename);
    return container[key];
  }
}

void texture_hub::delete_texel(const std::string &key) {
  if (auto res = container.find(key); res == container.end()) {
    std::cout << "\e[31;1mERROR: key does not exists when getting\e[0m" << std::endl;
    throw new std::invalid_argument("TEXTURES: Key " + key + " was not found.");
  }
  container.erase(key);
}

void texel::texel_set_source_surface(cairo_t *cr, int x, int y, float scale_x, float scale_y, bool centered) {
  if (!img) {
    std::cout << "\e[31;1mERROR: for some reasion this texel has no img to draw\e[0m" << std::endl;
    throw new std::invalid_argument("NULLPRTRTR");
  }
  int pos_x = x / scale_x - (centered ? cairo_image_surface_get_width(img) / 2.0 : 0);
  int pos_y = y / scale_y - (centered ? cairo_image_surface_get_height(img) / 2.0 : 0);

  cairo_set_source_surface(cr, img, pos_x, pos_y);
}

texture_hub::texture_hub() {}

texture_hub::~texture_hub() {}
