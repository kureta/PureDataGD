#include "puredatagd.h"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Convert resource path String to a FileAccess object.
Ref<FileAccess> resource_path_to_file(const String &path) {
  Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);

  return file;
}

// Check if a resource file exists
bool file_exists(const String &path) {
  Ref<FileAccess> file = resource_path_to_file(path);
  return file.is_valid();
}

#define MIX_RATE 48000
#define PCM_BUFFER_SIZE 4096

// ============ AudioStreamPD ============

// NOTE: Hard coded mono input, stereo output
AudioStreamPD::AudioStreamPD() : mix_rate(MIX_RATE) {
  if (!pd_instance.init(1, 2, mix_rate)) {
    ERR_PRINT("Failed to initialize PureData!");
    return;
  }
  pd_instance.computeAudio(false);
}

AudioStreamPD::~AudioStreamPD() {
  pd_instance.closePatch(patch);
  patch.clear();
  pd_instance.computeAudio(false);
  pd_instance.clear();
}

// It's our responsibility to use it to create the doppler effect (I guess)
void AudioStreamPD::send_float(const String &receiver, const float value) {
  pd_instance.sendFloat(receiver.utf8().get_data(), value);
}

void AudioStreamPD::send_bang(const String &receiver) {
  pd_instance.sendBang(receiver.utf8().get_data());
}

void AudioStreamPD::send_list(const String &receiver, const Array &list) {
  pd::List vec;
  for (int i = 0; i < list.size(); ++i) {
    Variant element = list[i];
    vec.addFloat(float(element));
  }

  pd_instance.sendList(receiver.utf8().get_data(), vec);
}

void AudioStreamPD::send_symbol(const String &receiver, const String &value) {
  pd_instance.sendSymbol(receiver.utf8().get_data(), value.utf8().get_data());
}

String AudioStreamPD::get_patch_path() { return patch_path; }

void AudioStreamPD::set_patch_path(const String &path) {
  UtilityFunctions::print("Trying to set path to: ", path);
  patch_path = path;
  if (!file_exists(patch_path))
    return;

  pd_instance.closePatch(patch);
  patch.clear();
  load_patch();

  UtilityFunctions::print("Set patch path to: ", patch_path);
}

void AudioStreamPD::load_patch() {
  UtilityFunctions::print("load Trying to set path to: ", patch_path);
  if (!file_exists(patch_path))
    return;

  Ref<FileAccess> file = resource_path_to_file(patch_path);
  String absolute_path = file->get_path_absolute();

  patch = pd_instance.openPatch(absolute_path.get_file().utf8().get_data(),
                                absolute_path.get_base_dir().utf8().get_data());
  UtilityFunctions::print("load Set patch path to: ", patch_path);
}

Ref<AudioStreamPlayback> AudioStreamPD::_instantiate_playback() const {
  Ref<AudioStreamPlaybackPD> playback;
  playback.instantiate();
  playback->audioStream = Ref<AudioStreamPD>(this);
  return playback;
}

void AudioStreamPD::_bind_methods() {
  BIND_PROPERTY(AudioStreamPD, STRING, patch_path, PROPERTY_HINT_FILE, "*.pd")

  BIND_METHOD(AudioStreamPD, send_float, "receiver", "float")
  BIND_METHOD(AudioStreamPD, send_bang, "receiver")
  BIND_METHOD(AudioStreamPD, send_list, "receiver", "list")
  BIND_METHOD(AudioStreamPD, send_symbol, "receiver", "symbol")
}

void AudioStreamPD::gen_tone(float *pcm_buf, int size) {
  int ticks = size / pd_instance.blockSize();

  if (!pd_instance.processFloat(ticks, inbuf_.data(), pcm_buf)) {
    ERR_PRINT("shit hit the fan");
    return;
  }
}

// ============ AudioStreamPlaybackPD ============

#define zeromem(to, count) memset(to, 0, count)

AudioStreamPlaybackPD::AudioStreamPlaybackPD() : active(false) {
  pcm_buffer = memalloc(PCM_BUFFER_SIZE);
  zeromem(pcm_buffer, PCM_BUFFER_SIZE);
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
  active = true;
  audioStream->pd_instance.computeAudio(active);
}

void AudioStreamPlaybackPD::_stop() {
  active = false;
  audioStream->pd_instance.computeAudio(active);
}

bool AudioStreamPlaybackPD::_is_playing() const { return active; }

int32_t AudioStreamPlaybackPD::_mix(AudioFrame *buffer, float rate_scale,
                                    int32_t frames) {
  ERR_FAIL_COND_V(!active, 0);

  ERR_FAIL_COND_V(frames > PCM_BUFFER_SIZE, 0);

  // Generate 16 bits PCM samples in "buf"
  zeromem(pcm_buffer, PCM_BUFFER_SIZE);
  auto *buf = (float *)pcm_buffer;
  audioStream->gen_tone(buf, frames);

  // NOTE: locking might be necessary here because this is the only place
  // where we modify a buffer that does not belong to us.

  // AudioServer::get_singleton()->lock();
  for (int i = 0; i < frames; i++) {
    // copy interleaved
    buffer[i] = {buf[i * 2], buf[i * 2 + 1]};
  }
  // AudioServer::get_singleton()->unlock();

  return frames;
}
