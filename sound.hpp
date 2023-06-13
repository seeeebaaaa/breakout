#ifndef SOUND_OWN_H
#define SOUND_OWN_H
#ifdef __APPLE__
#define MA_NO_RUNTIME_LINKING
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio_extra.h"
#include <map>
#include <string>
namespace sound {

class sound_cloud {
  struct sound_container {
    sound_cloud *owner;
    std::string path;
    ma_sound main_sound;
    unsigned max_instances = 1;
    ma_sound *instance_container;
    bool at_max_ignore_new_sounds = true; // if true, then playing more sounds then max_instances does nothing, if false, creats a new sound from file, which takes some time;

    sound_container(std::string path, unsigned max_instances, sound_cloud *sc_owner);
    void play_sound(double volume, double pitch, double pan);
    // void set_sound();
    ~sound_container();
  };
  friend std::ostream &operator<<(std::ostream &os, const sound_container &t);
  ma_result result;
  ma_engine engine;

  std::map<std::string, sound_container *> container;

  // ma_sound_init_copy

  /**
   * save one instance of a sound in file, when this sound is requested and already playing, create a copy and play/return that.
   */

public:
  sound_cloud();
  void set_sound(std::string path, unsigned max_at_once);                     // max at once are the number of sounds from this file that can be played at once. Each one has its own allocated space, so be careful
  void play_sound(std::string path, double volume, double pitch, double pan); // plays the sound; handles instance amnagement with multplie palying sounds
  ~sound_cloud();
};

} // namespace sound
#endif