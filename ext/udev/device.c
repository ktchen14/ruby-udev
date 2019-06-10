#include <errno.h>
#include <libudev.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "udev.h"
#include "device.h"

VALUE rb_cUdevDevice;

// Return the struct udev_device * in a Udev::Device instance
struct udev_device *rbudev_get_device(VALUE self);

// Set the struct udev_device * in a Udev::Device instance
void rbudev_set_device(VALUE self, struct udev_device *device);

// Call udev_device_unref on the struct udev_device *
void rbudev_device_free(void *data);

// Return the size of a struct udev_device *
size_t rbudev_device_size(const void *data);

// Allocate a Udev::Device instance
VALUE rbudev_device_alloc(VALUE self);

// Wrap a struct udev_device * in a TypedData struct
static const rb_data_type_t udev_device_type = {
  .wrap_struct_name = "udev_device_p",
  .function = {
    .dfree = rbudev_device_free,
    .dsize = rbudev_device_size,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

struct udev_device *rbudev_get_device(VALUE self) {
  struct udev_device **devicep;
  TypedData_Get_Struct(self, struct udev_device *, &udev_device_type, devicep);

  if (*devicep == NULL)
    rb_raise(rb_eTypeError, "uninitialized udev device");

  return *devicep;
}

void rbudev_set_device(VALUE self, struct udev_device *device) {
  struct udev_device **devicep;
  TypedData_Get_Struct(self, struct udev_device *, &udev_device_type, devicep);
  *devicep = device;
}

void rbudev_device_free(void *data) {
  udev_device_unref(*(struct udev_device **) data);
}

size_t rbudev_device_size(const void *data) {
  return sizeof(struct udev_device *);
}

VALUE rbudev_device_alloc(VALUE self) {
  struct udev_device **devicep;
  *devicep = NULL;
  return TypedData_Make_Struct(
    self, struct udev_device *, &udev_device_type, devicep);
}

VALUE rbudev_device_from_syspath(VALUE self, VALUE rb_syspath) {
  Check_Type(rb_syspath, T_STRING);
  char *syspath = StringValueCStr(rb_syspath);

  struct udev *udev = rbudev_get_udev();

  // Create the struct udev_device *
  struct udev_device *device;
  if ((device = udev_device_new_from_syspath(udev, syspath)) == NULL) {
    if (errno == ENOMEM)
      rb_memerror();
    rb_syserr_fail(errno, NULL);
  }

  // Create and initialize the Udev::Device instance
  VALUE rb_device = rb_class_new_instance(0, NULL, self);

  // Record the struct udev_device * in the instance
  rbudev_set_device(rb_device, device);

  return rb_device;
}

VALUE rbudev_device_from_devnum(VALUE self, VALUE rb_type, VALUE rb_devnum) {
  Check_Type(rb_type, T_STRING);
  char *type = StringValueCStr(rb_type);

  if (RSTRING_LEN(rb_type) != 0 || type[0] != 'b' && type[0] != 'c')
    rb_raise(rb_eArgError, "type %+" PRIsVALUE " must be 'b' or 'c'", rb_type);

  Check_Type(rb_devnum, T_FIXNUM);
  dev_t devnum = NUM2DEVT(rb_devnum);

  struct udev *udev = rbudev_get_udev();

  // Create the struct udev_device *
  struct udev_device *device;
  if ((device = udev_device_new_from_devnum(udev, type[0], devnum)) == NULL) {
    if (errno == ENOMEM)
      rb_memerror();
    rb_syserr_fail(errno, NULL);
  }

  // Create and initialize the Udev::Device instance
  VALUE rb_device = rb_class_new_instance(0, NULL, self);

  // Record the struct udev_device * in the instance
  rbudev_set_device(rb_device, device);

  return rb_device;
}

VALUE rbudev_device_syspath(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_syspath(device)) == NULL) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_sysname(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_sysname(device)) == NULL) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_sysnum(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_sysnum(device)) == NULL) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_devpath(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_devpath(device)) == NULL) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_devnode(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_devnode(device)) == NULL) {
    if (errno == ENOENT)
      return Qnil;
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_devnum(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  dev_t result;
  errno = 0;
  result = udev_device_get_devnum(device);
  if (major(result) == 0 && minor(result) == 0) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return DEVT2NUM(result);
}

VALUE rbudev_device_major_number(VALUE self) {
  VALUE rb_result = rbudev_device_devnum(self);
  if (NIL_P(rb_result))
    return rb_result;
  return UINT2NUM(major(NUM2DEVT(rb_result)));
}

VALUE rbudev_device_minor_number(VALUE self) {
  VALUE rb_result = rbudev_device_devnum(self);
  if (NIL_P(rb_result))
    return rb_result;
  return UINT2NUM(minor(NUM2DEVT(rb_result)));
}

VALUE rbudev_device_devtype(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_devtype(device)) == NULL) {
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_subsystem(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_subsystem(device)) == NULL) {
    if (errno == ENOENT)
      return Qnil;
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_driver(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  const char *result;
  errno = 0;
  if ((result = udev_device_get_driver(device)) == NULL) {
    if (errno == ENOENT)
      return Qnil;
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }
  return rb_str_new_cstr(result);
}

VALUE rbudev_device_parent(VALUE self) {
  struct udev_device *device = rbudev_get_device(self);

  struct udev_device *parent;
  errno = 0;
  if ((parent = udev_device_get_parent(device)) == NULL) {
    if (errno == ENOENT)
      return Qnil;
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }

  // Must acquire a reference here so that the parent device isn't deallocated
  // when the device itself is unreferenced
  udev_device_ref(parent);

  VALUE type = CLASS_OF(self);
  VALUE rb_parent = rb_class_new_instance(0, NULL, type);
  struct udev_device **parentp;
  TypedData_Get_Struct(
    rb_parent, struct udev_device *, &udev_device_type, parentp
  );
  *parentp = parent;

  return rb_parent;
}

VALUE rbudev_device_parent_with_subsystem_devtype(
    VALUE self, VALUE rb_subsystem, VALUE rb_devtype) {
  Check_Type(rb_subsystem, T_STRING);
  char *subsystem = StringValueCStr(rb_subsystem);
  Check_Type(rb_devtype, T_STRING);
  char *devtype = StringValueCStr(rb_devtype);

  struct udev_device *device = rbudev_get_device(self);

  struct udev_device *parent;
  errno = 0;
  parent = udev_device_get_parent_with_subsystem_devtype(
      device, subsystem, devtype);
  if (parent == NULL) {
    if (errno == ENOENT)
      return Qnil;
    if (errno != 0)
      rb_syserr_fail(errno, NULL);
    return Qnil;
  }

  // Must acquire a reference here so that the parent device isn't deallocated
  // when the device itself is unreferenced
  udev_device_ref(parent);

  VALUE type = CLASS_OF(self);
  VALUE rb_parent = rb_class_new_instance(0, NULL, type);
  struct udev_device **parentp;
  TypedData_Get_Struct(
    rb_parent, struct udev_device *, &udev_device_type, parentp
  );
  *parentp = parent;

  return rb_parent;
}

void Init_udev_device(void) {
  rb_cUdevDevice = rb_define_class_under(rb_mUdev, "Device", rb_cData);
  rb_define_alloc_func(rb_cUdevDevice, rbudev_device_alloc);

  rb_define_singleton_method(
    rb_cUdevDevice, "from_syspath", rbudev_device_from_syspath, 1
  );
  rb_define_singleton_method(
    rb_cUdevDevice, "from_devnum", rbudev_device_from_devnum, 2
  );

  rb_define_method(rb_cUdevDevice, "syspath", rbudev_device_syspath, 0);
  rb_define_method(rb_cUdevDevice, "sysname", rbudev_device_sysname, 0);
  rb_define_method(rb_cUdevDevice, "sysnum", rbudev_device_sysnum, 0);
  rb_define_method(rb_cUdevDevice, "devpath", rbudev_device_devpath, 0);
  rb_define_method(rb_cUdevDevice, "devnode", rbudev_device_devnode, 0);
  rb_define_method(rb_cUdevDevice, "devnum", rbudev_device_devnum, 0);
  rb_define_method(rb_cUdevDevice, "major_number",
      rbudev_device_major_number, 0);
  rb_define_method(rb_cUdevDevice, "minor_number",
      rbudev_device_minor_number, 0);
  rb_define_method(rb_cUdevDevice, "devtype", rbudev_device_devtype, 0);
  rb_define_method(rb_cUdevDevice, "subsystem", rbudev_device_subsystem, 0);
  rb_define_method(rb_cUdevDevice, "driver", rbudev_device_driver, 0);
  rb_define_method(rb_cUdevDevice, "parent", rbudev_device_parent, 0);
  rb_define_method(rb_cUdevDevice, "parent_with_subsystem_devtype",
      rbudev_device_parent_with_subsystem_devtype, 2);
  /* rb_define_method(rb_cUdevDevice, "is_initialized", rbudev_device_is_initialized, 0); */
  /* rb_define_method(rb_cUdevDevice, "action", rbudev_device_action, 0); */
}
