#include "udev.h"

VALUE rb_mUdev;

void Init_udev(void) {
  rb_mUdev = rb_define_module("Udev");
}
