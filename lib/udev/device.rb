require 'udev/udev'

module Udev
  class Device
    def inspect
      "#<#{self.class.name} #{syspath.inspect}>"
    end

    alias path syspath
    alias name sysname
  end
end
