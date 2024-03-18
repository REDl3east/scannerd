### Quick Start

```bash
mkdir build
cd build
export STAGING_DIR=/path/to/source/staging_dir
cmake -D CMAKE_C_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-gcc -D CMAKE_CXX_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-g++ ..
make
```

```bash
# Send to Onion
sshpass -p "onioneer" scp -r ./scannerd root@Omega-4D72:/root
```

### Dependancies
- [OpenWrt toolchain](https://github.com/OnionIoT/source)