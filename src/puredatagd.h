#ifndef PUREDATAGD_H
#define PUREDATAGD_H

#include <PdBase.hpp>
#include <PdTypes.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/variant/array.hpp>

// Some macros
#define BIND_METHOD(class, method_name, ...)                                   \
  ClassDB::bind_method(D_METHOD(#method_name, ##__VA_ARGS__),                  \
                       &class ::method_name);

#define BIND_PROPERTY(class, type, property_name, hint_type, hint_string)      \
  ClassDB::bind_method(D_METHOD("get_" #property_name),                        \
                       &class ::get_##property_name);                          \
  ClassDB::bind_method(D_METHOD("set_" #property_name, "p_" #property_name),   \
                       &class ::set_##property_name);                          \
  ADD_PROPERTY(                                                                \
      PropertyInfo(Variant::type, #property_name, hint_type, hint_string),     \
      "set_" #property_name, "get_" #property_name);

namespace godot {

// Wrong but working class
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

  // Send float
  void send_float(const String receiver, const float value);
  void send_bang(const String receiver);
  void send_list(const String receiver, const Array list);
  void send_symbol(const String receiver, const String value);
};

class AudioStreamPD : public AudioStream {
  // NOLINTNEXTLINE(modernize-use-auto)
  GDCLASS(AudioStreamPD, AudioStream)
  friend class AudioStreamPlaybackPD;

private:
  // A position / phase of the signal to generate (unit: samples)
  // TODO: Should this state be in Playback instead?
  uint64_t pos;

  int mix_rate;
  bool stereo;
  int hz;

  pd::PdBase pd{};
  pd::Patch patch{};
  String patch_path{};
  void load_patch();
  std::array<float, 1> inbuf_;
  std::array<float, 2048 * 2> outbuf_;

public:
  AudioStreamPD();
  [[nodiscard]] Ref<AudioStreamPlayback> _instantiate_playback() const override;

  // Set the current position / phase of the signal to generate (in samples)
  void set_position(uint64_t pos);

  // Generate "size" PCM samples in "pcm_buf"
  void gen_tone(float *pcm_buf, int size);

  // Patch path
  String get_patch_path();
  void set_patch_path(const String path);

protected:
  static void _bind_methods();
};

class AudioStreamPlaybackPD : public AudioStreamPlayback {
  // NOLINTNEXTLINE(modernize-use-auto)
  GDCLASS(AudioStreamPlaybackPD, AudioStreamPlayback)
  friend class AudioStreamPD;

private:
  Ref<AudioStreamPD>
      audioStream; // Keep track of the AudioStream which instantiated us
  bool active;     // Are we currently playing?
  void *pcm_buffer;

public:
  AudioStreamPlaybackPD();
  ~AudioStreamPlaybackPD() override;

  /**
   * "AudioStreamPlayer uses mix callback to obtain PCM data.
   *  The callback must match sample rate and fill the buffer.
   *  Since AudioStreamPlayback is controlled by the audio thread,
   *  i/o and dynamic memory allocation are forbidden."
   */
  int32_t _mix(AudioFrame *p_buffer, float p_rate_scale,
               int32_t p_frames) override;

  [[nodiscard]] bool _is_playing() const override;
  void _start(double from_pos) override;
  void _seek(double position) override;
  void _stop() override;

protected:
  static void _bind_methods();
};

} // namespace godot

#endif
