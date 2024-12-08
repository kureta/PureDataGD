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
  std::array<float, 44100 * 2> outbuf_;
  pd::PdBase pd{};
  pd::Patch patch{};
  bool dsp_on = false;
  String patch_path{};
  void init();
  void load_patch();

protected:
  static void _bind_methods();

public:
  PureDataGD();
  ~PureDataGD() override;

  void _process(double delta) override;
  void set_patch_path(const String path);
  String get_patch_path();
  void set_dsp_on(const bool status);
  bool get_dsp_on();
};

} // namespace godot

#endif
