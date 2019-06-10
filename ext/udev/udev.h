#ifndef RUBY_UDEV_H
#define RUBY_UDEV_H

#include <libudev.h>

#include "ruby.h"

extern VALUE rb_mUdev;
extern VALUE rb_cUdevContext;

struct udev *rbudev_get_udev(void);

#endif /* RUBY_UDEV_H */
