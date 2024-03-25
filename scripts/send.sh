#!/bin/sh

# Sends executable and web content to Onion

ONION_PASSWORD="onioneer"
ONION_HOST="root"
ONION_DOMAIN="Omega-4D72"

bash ./scripts/ssh_cmd.sh killall -9 scannerd

sshpass -p "${ONION_PASSWORD}" scp \
    -r ./static/ \
    ./build/scannerd \
    "${ONION_HOST}@${ONION_DOMAIN}:/root"
