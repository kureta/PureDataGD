#include "puredatagd.h"
#include <cstring>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

#define MIX_RATE 48000

#define print UtilityFunctions::print

struct FileInfo {
  std::string name;
  std::string path;
};

// PureData needs file name and path separately for loading
FileInfo parse_file_info(const Ref<FileAccess> file) {
  String absolute_path = file->get_path_absolute();

  FileInfo file_info = {absolute_path.get_file().utf8().get_data(),
                        absolute_path.get_base_dir().utf8().get_data()};

  return file_info;
}

// ============ AudioStreamPD ============

// NOTE: Hard coded mono input, stereo output
AudioStreamPD::AudioStreamPD() : mix_rate(MIX_RATE) {
  mix_rate = AudioServer::get_singleton()->get_mix_rate();
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

String AudioStreamPD::get_patch_path() {
  if (!patch_file.is_valid())
    return "";

  return patch_file->get_path();
}

void AudioStreamPD::set_patch_path(const String &path) {
  print("Trying to set patch path to: ", path);
  patch_file = FileAccess::open(path, FileAccess::READ);
  if (!patch_file.is_valid()) {
    WARN_PRINT("Set path but it is not a valid pd patch.");
    return;
  }

  pd_instance.closePatch(patch);
  patch.clear();
  load_patch();

  print("Loaded pd patch: ", patch_file->get_path());
}

void AudioStreamPD::load_patch() {
  if (!patch_file.is_valid())
    return;

  FileInfo file_info = parse_file_info(patch_file);
  patch = pd_instance.openPatch(file_info.name, file_info.path);
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

  if (!pd_instance.processFloat(ticks, dummy_inbuf.data(), pcm_buf)) {
    ERR_PRINT("PureData process borked!");
    return;
  }
}

// ============ AudioStreamPlaybackPD ============

#define zeromem(to, count) memset(to, 0, count)

AudioStreamPlaybackPD::AudioStreamPlaybackPD() : active(false) {
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

  audioStream->gen_tone(pcm_buffer, frames);

  // output buffer of pd is interleaved, AudioFrame is a struct {left, right}
  // so copying the buffer directly on the array of AudioFrames works out
  memcpy(buffer, pcm_buffer, 2 * frames * sizeof(float));

  return frames;
}
