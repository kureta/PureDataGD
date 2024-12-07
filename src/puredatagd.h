#ifndef PUREDATAGD_H
#define PUREDATAGD_H

#include <godot_cpp/classes/audio_stream_player.hpp>

namespace godot {

class PureDataGD : public AudioStreamPlayer {
  // NOLINTNEXTLINE(modernize-use-auto)
  GDCLASS(PureDataGD, AudioStreamPlayer);

  void *handle_{};

private:
  bool initialized_{};
  float inbuf_[1];
  float outbuf_[44100 * 2];

protected:
  static void _bind_methods();

public:
  PureDataGD();
  ~PureDataGD();

  void _process(double delta) override;
  void _notification(int p_notification) {
    printf("GDExtension has been reloaded.");
    if (p_notification == NOTIFICATION_EXTENSION_RELOADED) {
      // Handle the hot-reload event
      // Re-initialize variables, reload resources, etc.
      printf("GDExtension has been reloaded.");
    }
  }
};

} // namespace godot

#endif
