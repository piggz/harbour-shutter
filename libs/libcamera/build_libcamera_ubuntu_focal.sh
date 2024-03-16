# This is a script for building libcamera on Ubuntu 20.04 (focal)

# This is where PIP will install meson, so we need to add it to the path
export PATH=$PATH:`pwd`/.clickable/home/.local/bin/
# This is the best way to install the latest meson, according to https://mesonbuild.com/Quick-guide.html
pip3 install --user --ignore-installed meson

rm -rf libcamra && git clone https://git.libcamera.org/libcamera/libcamera.git

cd libcamera
echo "[binaries]" > arm64_armv8_linux_gcc
echo "c = ['ccache', 'aarch64-linux-gnu-gcc']" >> arm64_armv8_linux_gcc
echo "cpp = ['ccache', 'aarch64-linux-gnu-g++']" >> arm64_armv8_linux_gcc
echo "ar = 'aarch64-linux-gnu-gcc-ar'" >> arm64_armv8_linux_gcc
echo "strip = 'aarch64-linux-gnu-strip'" >> arm64_armv8_linux_gcc
echo "pkg-config = 'aarch64-linux-gnu-pkg-config'" >> arm64_armv8_linux_gcc
echo "pcap-config = ''" >> arm64_armv8_linux_gcc
echo "" >> arm64_armv8_linux_gcc
echo "[host_machine]" >> arm64_armv8_linux_gcc
echo "system = 'linux'" >> arm64_armv8_linux_gcc
echo "cpu_family = 'aarch64'" >> arm64_armv8_linux_gcc
echo "cpu = 'armv8-a'" >> arm64_armv8_linux_gcc
echo "endian = 'little'" >> arm64_armv8_linux_gcc
echo "" >> arm64_armv8_linux_gcc
echo "[properties]" >> arm64_armv8_linux_gcc
echo "# Generate binaries that are portable across all Armv8 machines" >> arm64_armv8_linux_gcc
echo "platform = 'generic'" >> arm64_armv8_linux_gcc
echo "pkg_config_path = ['/usr/lib/aarch64-linux-gnu/pkgconfig']" >> arm64_armv8_linux_gcc
# We have to disable documentation because the build is currently bugged on libcamera's side
meson setup build -Ddocumentation=disabled -Dv4l2=true -Dcam=enabled --cross-file arm64_armv8_linux_gcc --wipe
ninja -C build
chown -R user:user ../libcamera
DESTDIR="libcamera-install" ninja -C build install
