### Quick Start

```bash
mkdir build
cd build
export STAGING_DIR=/path/to/source/staging_dir
cmake -D CMAKE_C_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-gcc -D CMAKE_CXX_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-g++ -D CMAKE_BUILD_TYPE=Release ..
make

cd ../vue-scannerd
npm install
npm run build
```

```bash
# Send main executable to Onion
sshpass -p "onioneer" scp ./scannerd root@Omega-4D72:/root

# Remove static folder, cuz Vue generates random files with random names
# and eventually the static folder will overflow.
sshpass -p "onioneer" ssh root@Omega-4D72 rm -rf /root/static

# Send static HTML files
sshpass -p "onioneer" scp -r ../static root@Omega-4D72:/root

# Run on Onion
sshpass -p "onioneer" ssh root@Omega-4D72 /root/scannerd run /dev/input/event0 
```

### Dependancies
- [OpenWrt toolchain](https://github.com/OnionIoT/source)