#ifndef PUREDATAGD_H
#define PUREDATAGD_H

#include <PdBase.hpp>
#include <PdTypes.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/file_access.hpp>

namespace godot {

class PureDataGD : public AudioStreamPlayer {
  // NOLINTNEXTLINE(modernize-use-auto)
  GDCLASS(PureDataGD, AudioStreamPlayer);

private:
  std::array<float, 1> inbuf_;
  std::array<float, 2048 * 2> outbuf_;
  pd::PdBase pd{};
  pd::Patch patch{};
  bool dsp_on = false;
  String patch_path{};
  void init();
  void load_patch();
  float freq = 440.0;

protected:
  static void _bind_methods();

public:
  PureDataGD();
  ~PureDataGD() override;

  // Send PureData buffer to Godot
  void _process(const double delta) override;

  // Patch path
  String get_patch_path();
  void set_patch_path(const String path);

  // DSP On/Off
  bool get_dsp_on();
  void set_dsp_on(const bool status);

  // Osc Frequency
  double get_freq();
  void set_freq(const float f);

  // Send float
  void send_float(const String receiver, const float value);
};

} // namespace godot

#endif
