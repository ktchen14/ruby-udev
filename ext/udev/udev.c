#include <errno.h>
#include <libudev.h>

#include "udev.h"
#include "device.h"

VALUE rb_mUdev;
VALUE rb_cUdevContext;

void rbudev_context_free(void *data) {
  udev_unref(*(struct udev **) data);
}

size_t rbudev_context_size(const void *data) {
  return sizeof(struct udev *);
}

static const rb_data_type_t udev_context_type = {
  .wrap_struct_name = "udev",
  .function = {
    .dfree = rbudev_context_free,
    .dsize = rbudev_context_size,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE rbudev_context_alloc(VALUE self) {
  struct udev **udevp;
  *udevp = NULL;
  return TypedData_Make_Struct(self, struct udev *, &udev_context_type, udevp);
}

VALUE rbudev_context_initialize(VALUE self) {
  struct udev **udevp;
  TypedData_Get_Struct(self, struct udev *, &udev_context_type, udevp);

  if ((*udevp = udev_new()) == NULL) {
    if (errno == ENOMEM)
      rb_memerror();
    rb_syserr_fail(errno, NULL);
  }

  return self;
}

struct udev *rbudev_get_udev(void) {
  VALUE rb_udev = rb_iv_get(rb_mUdev, "context");

  if (NIL_P(rb_udev))
    rb_raise(rb_eTypeError, "uninitialized udev context");

  struct udev **udevp;
  TypedData_Get_Struct(rb_udev, struct udev *, &udev_context_type, udevp);

  if (*udevp == NULL)
    rb_raise(rb_eTypeError, "uninitialized udev context");

  return *udevp;
}

void Init_udev(void) {
  rb_mUdev = rb_define_module("Udev");

  // Initialize the Udev::Context instance that will hold the struct udev *
  rb_cUdevContext = rb_define_class_under(rb_mUdev, "Context", rb_cData);
  rb_define_alloc_func(rb_cUdevContext, rbudev_context_alloc);
  rb_define_method(rb_cUdevContext, "initialize", rbudev_context_initialize, 0);
  VALUE udev_context = rb_class_new_instance(0, NULL, rb_cUdevContext);

  // Set the instance variable without a @ so that it's invisible from Ruby
  rb_iv_set(rb_mUdev, "context", udev_context);

  Init_udev_device();
}
