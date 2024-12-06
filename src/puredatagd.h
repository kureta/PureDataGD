#ifndef PUREDATAGD_H
#define PUREDATAGD_H

#include <godot_cpp/classes/sprite2d.hpp>

namespace godot {

class PureDataGD : public Sprite2D {
  GDCLASS(PureDataGD, Sprite2D)

private:
  double time_passed;

protected:
  static void _bind_methods();

public:
  PureDataGD();
  ~PureDataGD();

  void _process(double delta) override;
  void _notification(int p_notification) {
    printf("GDExtension has been reloaded.");
    if (p_notification == NOTIFICATION_EXTENSION_RELOADED) {
      // Handle the hot-reload event
      // Re-initialize variables, reload resources, etc.
      time_passed = 0.0;
      printf("GDExtension has been reloaded.");
    }
  }
};

} // namespace godot

#endif
