#include "puredatagd.h"
#include "PdTypes.hpp"
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GenPD::_bind_methods() {}

// TODO: PdBase object refers to a single static PureData instance.
// Changing patch does not properly close the previous patch (maybe).
// TODO: Seems kine I haven't been using the GDExtension interface.
// TODO: I am extending the wrong class. Should extend AudioStreamGenerator

// Define things that will be called by or seen inside the Godot GUI.
void PureDataGD::_bind_methods() {
  // Properties
  BIND_PROPERTY(STRING, patch_path, PROPERTY_HINT_FILE, "*.pd")
  BIND_PROPERTY(BOOL, dsp_on, PROPERTY_HINT_NONE, "")

  // Methods
  BIND_METHOD(send_float, "receiver", "float")
  BIND_METHOD(send_bang, "receiver")
  BIND_METHOD(send_list, "receiver", "list")
  BIND_METHOD(send_symbol, "receiver", "symbol")
}

// Convert resource path String to a FileAccess object.
// TODO: set and get via path_name but store FileAccess object as state.
Ref<FileAccess> resource_path_to_file(const String &path) {
  Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);

  return file;
}

// Check if a resource exists
bool file_exists(const String &path) {
  Ref<FileAccess> file = resource_path_to_file(path);
  if (!file.is_valid())
    ERR_PRINT(String("File does not exist: ") + path);
  return file.is_valid();
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
