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
  cairo_surface_t *imageSurface = cairo_image_surface_create_from_png(filename);
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
texture_hub::texture_hub() {}

texture_hub::~texture_hub() {}
