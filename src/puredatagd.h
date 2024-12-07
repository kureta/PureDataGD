#ifndef PUREDATAGD_H
#define PUREDATAGD_H

#include <PdBase.hpp>
#include <PdTypes.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>

namespace godot {

class PureDataGD : public AudioStreamPlayer {
  // NOLINTNEXTLINE(modernize-use-auto)
  GDCLASS(PureDataGD, AudioStreamPlayer);

private:
  std::array<float, 1> inbuf_;
  std::array<float, 44100 * 2> outbuf_;
  pd::PdBase pd{};
  pd::Patch patch{};
  bool initialized = false;

protected:
  static void _bind_methods();

public:
  PureDataGD();
  ~PureDataGD() override;

  void _process(double delta) override;
};

} // namespace godot

#endif
