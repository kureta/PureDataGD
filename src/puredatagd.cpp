#include "puredatagd.h"
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// TODO: set and get via path_name but store FileAccess object as state.
// Convert resource path String to a FileAccess object.
Ref<FileAccess> resource_path_to_file(const String &path) {
  Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);

  return file;
}

// TODO: compile for Android.
// Check if a resource exists
bool file_exists(const String &path) {
  Ref<FileAccess> file = resource_path_to_file(path);
  if (!file.is_valid())
    ERR_PRINT(String("File does not exist: ") + path);
  return file.is_valid();
}

enum {
  // Native Godot sample rate (use AudioStreamPlaybackResampled for other
  // values)
  MIX_RATE = 48000,
  // A buffer of about 93ms (at 44100 mix rate)
  PCM_BUFFER_SIZE = 4096,
  // TODO Document this (see core implementations). Note that 4096=2^13
  MIX_FRAC_BITS = 13
};

// ============ AudioStreamPD ============

// TODO: stereo doesn't work
AudioStreamPD::AudioStreamPD() : mix_rate(MIX_RATE), stereo(false), hz(639) {
  int n_channels = stereo ? 2 : 1;
  if (!pd.init(1, n_channels, mix_rate)) {
    ERR_PRINT("Failed to initialize PureData!");
    return;
  }
  pd.computeAudio(true);
  load_patch();
}

// TODO: Doppler effects sends in a `pitch_scale` parameter
// It's our responsibility to use it to create the doppler effect (I guess)
void AudioStreamPD::send_float(const String receiver, const float value) {
  pd.sendFloat(receiver.utf8().get_data(), value);
}

String AudioStreamPD::get_patch_path() { return patch_path; }

void AudioStreamPD::set_patch_path(const String path) {
  UtilityFunctions::print("Trying to set path to: ", path);
  patch_path = path;
  if (!file_exists(patch_path))
    return;

  pd.closePatch(patch);
  patch.clear();
  load_patch();

  UtilityFunctions::print("Set patch path to: ", patch_path);
}

void AudioStreamPD::load_patch() {
  UtilityFunctions::print("load Trying to set path to: ", patch_path);
  if (!file_exists(patch_path))
    return;

  patch = pd.openPatch(
      resource_path_to_file(patch_path)->get_path_absolute().utf8().get_data(),
      "/");
  UtilityFunctions::print("load Set patch path to: ", patch_path);
}

Ref<AudioStreamPlayback> AudioStreamPD::_instantiate_playback() const {
  Ref<AudioStreamPlaybackPD> playback;
  playback.instantiate();
  playback->audioStream = Ref<AudioStreamPD>(this);
  return playback;
}

void AudioStreamPD::set_position(uint64_t p) { pos = p; }

// TODO: Add buffer size and sample rate as properties
void AudioStreamPD::_bind_methods() {
  BIND_PROPERTY(AudioStreamPD, STRING, patch_path, PROPERTY_HINT_FILE, "*.pd")

  BIND_METHOD(AudioStreamPD, send_float, "receiver", "float")
}

void AudioStreamPD::gen_tone(float *pcm_buf, int size) {
  int ticks = size / pd.blockSize();

  if (!pd.processFloat(ticks, inbuf_.data(), pcm_buf)) {
    ERR_PRINT("shit hit the fan");
    return;
  }
}

#define zeromem(to, count) memset(to, 0, count)

// ============ AudioStreamPlaybackPD ============

AudioStreamPlaybackPD::AudioStreamPlaybackPD() : active(false) {
  // TODO Is locking actually required?
  AudioServer::get_singleton()->lock();
  pcm_buffer = memalloc(PCM_BUFFER_SIZE);
  zeromem(pcm_buffer, PCM_BUFFER_SIZE);
  AudioServer::get_singleton()->unlock();
}

AudioStreamPlaybackPD::~AudioStreamPlaybackPD() {
  if (pcm_buffer) {
    memfree(pcm_buffer);
    pcm_buffer = nullptr;
  }
}

void AudioStreamPlaybackPD::_bind_methods() {
  // Required by GDCLASS macro
}

void AudioStreamPlaybackPD::_start(double from_pos) {
  _seek(from_pos);
  active = true;
  audioStream->pd.computeAudio(active);
}

void AudioStreamPlaybackPD::_stop() {
  active = false;
  audioStream->set_position(0);
  audioStream->pd.computeAudio(active);
}

void AudioStreamPlaybackPD::_seek(double position) {
  if (position < 0) {
    position = 0;
  }

  // TODO What does this mean? What is the unit of "position"?
  // Note that set_position expects "samples"
  audioStream->set_position(uint64_t(position * audioStream->mix_rate)
                            << MIX_FRAC_BITS);
}

bool AudioStreamPlaybackPD::_is_playing() const { return active; }

int32_t AudioStreamPlaybackPD::_mix(AudioFrame *buffer, float rate_scale,
                                    int32_t frames) {
  ERR_FAIL_COND_V(!active, 0);

  // TODO What is the max possible value for "frames"?
  ERR_FAIL_COND_V(frames > PCM_BUFFER_SIZE, 0);

  // Generate 16 bits PCM samples in "buf"
  zeromem(pcm_buffer, PCM_BUFFER_SIZE);
  auto *buf = (float *)pcm_buffer;
  AudioServer::get_singleton()->lock();
  audioStream->gen_tone(buf, frames);
  AudioServer::get_singleton()->unlock();

  // Convert samples to Godot format (floats in [-1; 1])
  for (int i = 0; i < frames; i++) {
    buffer[i] = {buf[i], buf[i]};
  }

  return frames;
}

// TODO: PdBase object refers to a single static PureData instance.
//       enable multi instance so that we can define different sources
//       of positional audio.
// TODO: I am extending the wrong class. Should extend AudioStreamGenerator
//       Or in some other way I should be able to use pd patches in any
//       audio node (2D, 3D, ...)
// TODO: Redirect all stdout/stderr from libpd to Godot.
// TODO: libpd cannot access/autoload other patches in the same directory.
// TODO: buffer size, block size, sampler rate, etc. should be configurable
//       from Godot. Currently, somewhere in here or in libpd, the buffer
//       is set to 25ms. If buffer set in the Generator in Godot is not exactly
//       the same (or maybe higher) audio gets borked.

// Define things that will be called by or seen inside the Godot GUI.
void PureDataGD::_bind_methods() {
  // Properties
  BIND_PROPERTY(PureDataGD, STRING, patch_path, PROPERTY_HINT_FILE, "*.pd")
  BIND_PROPERTY(PureDataGD, BOOL, dsp_on, PROPERTY_HINT_NONE, "")

  // Methods
  BIND_METHOD(PureDataGD, send_float, "receiver", "float")
  BIND_METHOD(PureDataGD, send_bang, "receiver")
  BIND_METHOD(PureDataGD, send_list, "receiver", "list")
  BIND_METHOD(PureDataGD, send_symbol, "receiver", "symbol")
}

void PureDataGD::init() {
  // I guess there is just one global instance of PureData.
  // We keep track of its initialization state but I don't really understand
  // this part. I guess that's why the other guy implemented a static
  // singleton.
  if (!pd.init(1, 2, 48000)) {
    ERR_PRINT("Failed to initialize PureData!");
    return;
  }
  pd.computeAudio(dsp_on);
}

void PureDataGD::load_patch() {
  if (!file_exists(patch_path))
    return;

  patch = pd.openPatch(
      resource_path_to_file(patch_path)->get_path_absolute().utf8().get_data(),
      "/");
}

PureDataGD::PureDataGD() {
  UtilityFunctions::print("PureDataGD constructor");
  init();
  load_patch();
}

// Closing the patch in the destructor works but if we close or
// `clear()` the pd object there's no sound in Godot.
PureDataGD::~PureDataGD() {
  UtilityFunctions::print("PureDataGD destructor");
  pd.closePatch(patch);
  patch.clear();
  pd.computeAudio(false);
  pd.clear();
}

String PureDataGD::get_patch_path() { return patch_path; }

void PureDataGD::set_patch_path(const String path) {
  UtilityFunctions::print("Trying to set path to: ", path);
  patch_path = path;
  if (!file_exists(patch_path))
    return;

  pd.closePatch(patch);
  patch.clear();
  load_patch();

  UtilityFunctions::print("Set patch path to: ", patch_path);
}

bool PureDataGD::get_dsp_on() { return dsp_on; }
void PureDataGD::set_dsp_on(bool status) {
  UtilityFunctions::print("Setting DSP status to ", status);
  pd.computeAudio(status);
  dsp_on = status;
}

void PureDataGD::send_float(const String receiver, const float value) {
  pd.sendFloat(receiver.utf8().get_data(), value);
}

void PureDataGD::send_bang(const String receiver) {
  pd.sendBang(receiver.utf8().get_data());
}

void PureDataGD::send_list(const String receiver, const Array list) {
  pd::List vec;
  for (int i = 0; i < list.size(); ++i) {
    Variant element = list[i];
    vec.addFloat(float(element));
  }

  pd.sendList(receiver.utf8().get_data(), vec);
}

void PureDataGD::send_symbol(const String receiver, const String value) {
  pd.sendSymbol(receiver.utf8().get_data(), value.utf8().get_data());
}

// This will suffice for a long time. Just runs PD and fills Godot's audio
// buffer.
void PureDataGD::_process(double delta) {
  if (is_playing()) {
    Ref<AudioStreamGeneratorPlayback> p = get_stream_playback();
    if (p.is_valid()) {
      int nframes = std::min(p->get_frames_available(), 2048);
      int ticks = nframes / pd.blockSize();

      if (!pd.processFloat(ticks, inbuf_.data(), outbuf_.data())) {
        ERR_PRINT("shit hit the fan");
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
