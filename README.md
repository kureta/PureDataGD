# PureData GDExtension

**This code is licensed under [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)**

## [TODO]

- [ ] Compile for Android. Use the following command as a starting point:

  ```bash
  cmake \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    --build .
  ```

- [ ] Tidy up the code. Make it more readable
- [ ] Create a better demo Godot project
- [ ] Add error handling
- [ ] Remove hard-coded buffer size, block size, sample rate, etc.
      and make them all configurable from within the Godot editor.
      We should get the sample rate from Godot.
- [ ] Do something with `pitch_scale` parameter sent from Godot for Doppler effect.
- [ ] Better understand the `_mix` method and its arguments.
- [ ] See if/where `AudioServer::get_singleton()->lock();` is really necessary.
- [ ] Improve log messages
