#include "puredatagd.h"
#include "PdTypes.hpp"
#include "z_libpd.h"
#include <PdBase.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PureDataGD::_bind_methods() {}

PureDataGD::PureDataGD() {
  UtilityFunctions::print("PureDataGD constructor");

  // Initialize the PdBase object.
  pd::PdBase pd;
  pd::Patch patch = pd.openPatch("bin/test.pd", ".");

  if (::libpd_init() == 0) {
    UtilityFunctions::print("Initialized PureData");
  } else {
  }

  if (::libpd_init_audio(1, 2, 44100) == 0) {
    UtilityFunctions::print("Initialized DSP");
  } else {
    return;
  }

  initialized_ = true;

  handle_ = ::libpd_openfile("test.pd", "./bin");
  if (handle_ == nullptr) {
    UtilityFunctions::print("Error opening patch");
  } else {
    UtilityFunctions::print("Opened patch");
  }
}

PureDataGD::~PureDataGD() {
  UtilityFunctions::print("PureDataGD destructor");
  ::libpd_closefile(handle_);
}

void PureDataGD::_process(double delta) {
  if (is_playing()) {
    Ref<AudioStreamGeneratorPlayback> p = get_stream_playback();
    if (p.is_valid()) {
      int nframes = std::min(p->get_frames_available(), 2048);

      int ticks = nframes / libpd_blocksize();

      if (::libpd_process_float(ticks, inbuf_, outbuf_) != 0) {
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
