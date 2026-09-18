#!/bin/bash

set -euo pipefail

MODULE="hello_world"
PARAM_DIR="/sys/module/${MODULE}/parameters"
EXPECTED="Hello, World!"
MODINFO_FILE="./build/${MODULE}.ko"

cleanup() {
    if lsmod | grep -q "^${MODULE} "; then
        sudo rmmod "${MODULE}"
    fi
}

trap cleanup EXIT

if [[ ! -f "${MODINFO_FILE}" ]]; then
    echo "Error: ${MODINFO_FILE} not found"
    echo "Run 'make' first"
    exit 1
fi

cleanup

echo "Loading module..."
sudo insmod "${MODINFO_FILE}"

echo "Checking parameters..."

for param in idx ch_val my_str; do
    if [[ ! -e "${PARAM_DIR}/${param}" ]]; then
        echo "Error: parameter ${param} not found"
        exit 1
    fi
done

echo "Checking initial string..."

if [[ "$(cat "${PARAM_DIR}/my_str")" != "" ]]; then
    echo "Error: initial string is not empty"
    exit 1
fi

echo "Writing string: ${EXPECTED}"

for ((i = 0; i < ${#EXPECTED}; i++)); do
    ascii=$(printf '%d' "'${EXPECTED:i:1}")

    echo "${i}" | sudo tee "${PARAM_DIR}/idx" >/dev/null
    echo "${ascii}" | sudo tee "${PARAM_DIR}/ch_val" >/dev/null
done

actual=$(cat "${PARAM_DIR}/my_str")

if [[ "${actual}" != "${EXPECTED}" ]]; then
    echo "Error: wrong string"
    echo "Expected: ${EXPECTED}"
    echo "Actual:   ${actual}"
    exit 1
fi

echo "String is correct: ${actual}"

echo "Checking read-only my_str..."

if echo "BAD" | sudo tee "${PARAM_DIR}/my_str" >/dev/null 2>&1; then
    echo "Error: my_str is writable"
    exit 1
fi

echo "my_str is read-only"

echo "Checking invalid idx..."

if echo "13" | sudo tee "${PARAM_DIR}/idx" >/dev/null 2>&1; then
    echo "Error: invalid idx accepted"
    exit 1
fi

echo "Invalid idx rejected"

echo "Checking invalid ch_val..."

if echo "10" | sudo tee "${PARAM_DIR}/ch_val" >/dev/null 2>&1; then
    echo "Error: invalid ch_val accepted"
    exit 1
fi

echo "Invalid ch_val rejected"

echo "Unloading module..."
sudo rmmod "${MODULE}"

echo "Module unloaded"

echo "All checks passed!"
