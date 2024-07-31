# scannerd
This IoT application allows users to effortlessly scan barcodes and instantly access nutritional facts. Leveraging the compact and powerful Onion Omega2 development board and a USB barcode decoder, this project delivers real-time data retrieval and display.

## Features
- **Real-Time Scanning:** Quickly scan barcodes using a USB barcode decoder for instant nutritional information.
- **Responsive Frontend:** Built with Vue, the application provides a smooth and intuitive user experience on various devices.
- **Efficient Backend:** Utilizes libuv for asynchronous I/O operations and Crow for a lightweight yet robust HTTP server.
- **IoT Integration:** Seamlessly integrates with the Onion Omega2 development board, showcasing the potential of IoT in everyday applications.

### Quick Start

```bash
mkdir build
cd build
export STAGING_DIR=/path/to/source/staging_dir
cmake \
-D CMAKE_C_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-gcc \
-D CMAKE_CXX_COMPILER=${STAGING_DIR}/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-g++ \
-D CMAKE_BUILD_TYPE=Release \
..
make

cd ../vue-scannerd
npm install
npm run build
```

### Onion Commands
```bash
# Send main executable to Onion
sshpass -p "onioneer" scp ./scannerd root@Omega-398C:/root

# Remove static folder, cuz Vue generates random files with random names
# and eventually the static folder will overflow.
sshpass -p "onioneer" ssh root@Omega-398C rm -rf /root/static

# Send static HTML files
sshpass -p "onioneer" scp -r ../static root@Omega-398C:/root

# Run on Onion
sshpass -p "onioneer" ssh root@Omega-398C /root/scannerd run /dev/input/event0 
```

### Dependancies
- [OpenWrt toolchain](https://github.com/OnionIoT/source)
