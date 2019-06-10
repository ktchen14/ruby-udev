RSpec.describe Udev::Device do
  let(:cpu0) { described_class.from_syspath('/sys/devices/system/cpu/cpu0') }
  let(:node) { described_class.from_syspath('/sys/devices/system/node') }
  let(:disk) { described_class.from_syspath('/sys/devices/pci0000:00/0000:00:10.0/host2/target2:0:0/2:0:0:0/block/sda') }
  let(:null) { described_class.from_syspath('/sys/devices/virtual/mem/null') }

  describe '.from_syspath' do
    it 'returns a device when a device is at the syspath' do
      device = described_class.from_syspath('/sys/devices/virtual/mem/null')
      expect(device).to be_an_instance_of(described_class)
    end

    it "raises a TypeError when the syspath isn't a string" do
      expect do
        described_class.from_syspath(1234)
      end.to raise_error(TypeError)

      expect do
        described_class.from_syspath(nil)
      end.to raise_error(TypeError)
    end

    it 'raises ENODEV when no device is at the syspath' do
      expect do
        described_class.from_syspath('/sys/devices/none')
      end.to raise_error(Errno::ENODEV)
    end
  end

  describe '#syspath' do
    it 'returns the syspath of the device' do
      expect(null.syspath).to eq('/sys/devices/virtual/mem/null')
    end
  end

  describe '#path' do
    it 'is an alias of #syspath' do
      expect(described_class.instance_method(:path)).to eq(described_class.instance_method(:syspath))
    end
  end

  describe '#sysname' do
    it 'returns the kernel name of the device' do
      expect(null.sysname).to eq('null')
    end
  end

  describe '#sysnum' do
    it "returns nil when the device doesn't have an assigned sysnum" do
      expect(null.sysnum).to be_nil
    end

    it 'returns the sysnum of the device' do
      expect(cpu0.sysnum).to eq('0')
    end
  end

  describe '#name' do
    it 'is an alias of #sysname' do
      expect(described_class.instance_method(:name)).to eq(described_class.instance_method(:sysname))
    end
  end

  describe '#devpath' do
    it 'returns the devpath of the device' do
      expect(null.devpath).to eq('/devices/virtual/mem/null')
    end
  end

  describe '#devnode' do
    it "returns nil when the device doesn't have an associated devnode" do
      expect(cpu0.devnode).to be_nil
    end

    it 'returns the devnode of the device' do
      expect(null.devnode).to eq('/dev/null')
    end
  end

  describe '#devnum' do
    it "returns nil when the device doesn't have a device number" do
      expect(cpu0.devnum).to be_nil
    end

    it 'returns the device number of the device' do
      expect(null.devnum).to eq(259)
    end
  end

  describe '#major_number' do
    it "returns nil when the device doesn't have a major number" do
      expect(cpu0.major_number).to be_nil
    end

    it 'returns the major number of the device' do
      expect(null.major_number).to eq(1)
    end
  end

  describe '#minor_number' do
    it "returns nil when the device doesn't have a minor number" do
      expect(cpu0.minor_number).to be_nil
    end

    it 'returns the minor number of the device' do
      expect(null.minor_number).to eq(3)
    end
  end

  describe '#devtype' do
    it "returns nil when the device doesn't have an associated devnode" do
      expect(cpu0.devtype).to be_nil
    end

    it 'returns the devtype of the device' do
      expect(disk.devtype).to eq('disk')
    end
  end

  describe '#subsystem' do
    it "returns nil when the device isn't in a subsystem" do
      expect(node.subsystem).to be_nil
    end

    it 'returns the subsystem of the device' do
      expect(null.subsystem).to eq('mem')
    end
  end

  describe '#driver' do
    it "returns nil when the device doesn't have an associated driver" do
      expect(null.driver).to be_nil
    end

    it 'returns the driver of the device' do
      expect(cpu0.driver).to eq('processor')
    end
  end

  describe '#parent' do
    context 'with no arguments' do
      it "returns nil when the device doesn't have a parent device" do
        expect(null.parent).to be_nil
      end

      it 'returns the parent device of the device' do
        expect(cpu0.parent).to_not be_nil
        expect(cpu0.parent.syspath).to eq('/sys/devices/system/cpu')
      end
    end

    context 'with a subsystem and a type' do
    end

    context 'with a subsystem and no type' do
    end

    context 'with no subsystem and a type' do
      it 'raises an ArgumentError' do
      end
    end
  end

  describe '#initialized?' do
    it 'returns true if the device has been initialized by udev' do
    end

    it "returns false if the device hasn't been initialized by udev" do
    end
  end
end
