// clang++ -std=c++20 -framework CoreFoundation -framework CoreAudio -framework AudioToolbox sound.cpp -o sound

#include "sound.hpp"
#include <iostream>
#include <stdexcept>
#include <stdio.h>

namespace sound {
sound_cloud::sound_container::sound_container(std::string path, unsigned max_instances, sound_cloud *sc_owner) {
  owner = sc_owner;

  // playback holder
  this->max_instances = max_instances;
  instance_container = new ma_sound[this->max_instances];
  this->path = path;
  owner->result = ma_sound_init_from_file(&owner->engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &main_sound);
  if (owner->result != MA_SUCCESS) {
    throw std::runtime_error("SOUND: Sound " + path + "could not be initialized as main!");
  }
  for (int i = 0; i < max_instances; i++) {
    owner->result = ma_sound_init_copy(&owner->engine, &main_sound, MA_SOUND_FLAG_DECODE, NULL, &instance_container[i]);
    if (owner->result != MA_SUCCESS) {
      throw std::runtime_error("SOUND: Sound " + path + "could not be initialized from copy! [I:" + std::to_string(i) + "]");
    }
  }
}

std::ostream &operator<<(std::ostream &os, const sound_cloud::sound_container &t) {
  os << "Sound Contaner:" << std::endl
     << "  Owner:\t" << t.owner << std::endl
     << "  Path:\t" << t.path << std::endl
     << "  Max Instances:\t" << t.max_instances << std::endl
     << "  At max ignore:\t" << t.at_max_ignore_new_sounds << std::endl;
  return os;
}

void sound_cloud::sound_container::play_sound(double volume, double pitch, double pan) {
  // go through instance container and get first free spot
  for (int i = 0; i < max_instances; i++) {
    if (ma_sound_is_playing(&instance_container[i]))
      continue;
    // std::cout << "Played:" << std::endl
    //           << *this
    //           << "With:" << std::endl
    //           << "Volume:\t" << volume << std::endl
    //           << "Pitch:\t" << pitch << std::endl
    //           << "Pan:\t" << pan << std::endl;
    ma_sound_set_volume(&instance_container[i], volume);
    ma_sound_set_pitch(&instance_container[i], pitch);
    ma_sound_set_pan(&instance_container[i], pan);
    ma_sound_start(&instance_container[i]);
    break;
  }
}

sound_cloud::sound_container::~sound_container() {
  for (int i = 0; i < max_instances; i++) {
    ma_sound_uninit(&instance_container[i]);
  }
  delete[] instance_container;
}

// sound cloud constructor
sound_cloud::sound_cloud() {

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    throw std::runtime_error("SOUND: Sound engine could not be initialized!");
  }
}

// needs to be called at least once to inititialize a sound from file.
void sound_cloud::set_sound(std::string path, unsigned max_at_once) {
  // check if sound already exists
  if (container.find(path) != container.end()) {
    std::cout << "\e[31;1mERROR: key already exists when getting\e[0m" << std::endl;
    throw new std::runtime_error("SOUND: <set_sound> Key " + path + " already exists.");
  }
  container[path] = new sound_container(path, max_at_once, this);
}

// play a sound, after setting it at least once
void sound_cloud::play_sound(std::string path, double volume, double pitch, double pan) {
  if (container.find(path) == container.end()) {
    std::cout << "\e[31;1mERROR: key does not exists when getting\e[0m" << std::endl;
    throw new std::runtime_error("SOUND: <play_sound> Key " + path + " was not found.");
  }
  container[path]->play_sound(volume, pitch, pan);
}

// sound cloud destructor
sound_cloud::~sound_cloud() {
  container.clear();
  ma_engine_uninit(&engine);
}

} // namespace sound

// int main() {
//   // test own
//   sound::sound_cloud main_cloud;
//   main_cloud.set_sound("./assets/audio/audio_elephant.wav", 5);
//   main_cloud.play_sound("./assets/audio/audio_elephant.wav", 1, .4, 0);
//   main_cloud.play_sound("./assets/audio/audio_elephant.wav", 1, 1, 0);
//   main_cloud.play_sound("./assets/audio/audio_elephant.wav", 1, 4, 0);

//   getchar();

//   return 0;
// }