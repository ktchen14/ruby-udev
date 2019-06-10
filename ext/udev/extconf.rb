require 'mkmf'

pkg_config('libudev')

create_makefile('udev/udev')
