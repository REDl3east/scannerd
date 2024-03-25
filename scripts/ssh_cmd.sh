#!/bin/sh

ONION_PASSWORD="onioneer"
ONION_HOST="root"
ONION_DOMAIN="Omega-4D72"

sshpass -p "${ONION_PASSWORD}" ssh "${ONION_HOST}@${ONION_DOMAIN}" "$@"