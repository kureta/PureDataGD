#include "puredatagd.h"
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PureDataGD::_bind_methods() {}

PureDataGD::PureDataGD() {
  UtilityFunctions::print("PureDataGD constructor");

  pd.openPatch("test.pd", "./bin");

  if (!initialized) {
    initialized = pd.init(1, 2, 44100);
    if (initialized) {
      UtilityFunctions::print("Initialized PureData.");
    } else {
      UtilityFunctions::print("Failed to initialize PureData!");
    }
  }

  pd.computeAudio(true);
}

PureDataGD::~PureDataGD() {
  UtilityFunctions::print("PureDataGD destructor");
  pd.closePatch(patch);
}

void PureDataGD::_process(double delta) {
  if (is_playing()) {
    Ref<AudioStreamGeneratorPlayback> p = get_stream_playback();
    if (p.is_valid()) {
      int nframes = std::min(p->get_frames_available(), 2048);

      int ticks = nframes / pd.blockSize();

      if (!pd.processFloat(ticks, inbuf_.data(), outbuf_.data())) {
        return;
      }

      for (int i = 0; i < nframes; i++) {
        auto v = Vector2(outbuf_[i * 2], outbuf_[(i * 2) + 1]);
        v = v.clamp(Vector2(-1, -1), Vector2(1, 1));

        p->push_frame(v);
      }
    }
  }
}
