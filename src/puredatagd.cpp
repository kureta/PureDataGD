#include "puredatagd.h"
#include "PdTypes.hpp"
#include <PdBase.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void PureDataGD::_bind_methods() {}

PureDataGD::PureDataGD() {
  // Initialize any variables here.
  time_passed = 0.0;

  // Initialize the PdBase object.
  pd::PdBase pd;
  pd::Patch patch = pd.openPatch("bin/test.pd", ".");
}

void PureDataGD::_process(double delta) {
  time_passed += delta;

  Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)),
                                 10.0 + (10.0 * cos(time_passed * 1.5)));

  set_position(new_position);
}
