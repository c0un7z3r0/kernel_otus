#!/bin/sh
set -eu

MODULE=kernel_alloc
PARAM=/sys/module/$MODULE/parameters

if [ "$(id -u)" -ne 0 ]; then
    echo "Run as root: sudo $0"
    exit 1
fi

if [ ! -d "$PARAM" ]; then
    echo "Module $MODULE is not loaded."
    echo "Load it first: insmod kernel_alloc.ko"
    exit 1
fi

echo "== Initial stats =="
cat "$PARAM/stats"

echo
echo "== Allocate 4 KiB =="
echo 4096 > "$PARAM/alloc"
A1=$(cat "$PARAM/alloc")
echo "Address A1: $A1"

echo
echo "== Allocate 12 KiB (3 blocks) =="
echo 12288 > "$PARAM/alloc"
A2=$(cat "$PARAM/alloc")
echo "Address A2: $A2"

echo
echo "== Allocate 20 KiB (5 blocks) =="
echo 20480 > "$PARAM/alloc"
A3=$(cat "$PARAM/alloc")
echo "Address A3: $A3"

echo
echo "== Stats after allocations =="
cat "$PARAM/stats"

echo
echo "== Bitmap =="
cat "$PARAM/kbitmap_info"

echo
echo "== Free middle allocation A2 =="
echo "$A2" > "$PARAM/free"

echo
echo "== Stats after freeing A2 =="
cat "$PARAM/stats"

echo
echo "== Free first allocation A1 =="
echo "$A1" > "$PARAM/free"

echo
echo "== Stats after freeing A1 =="
cat "$PARAM/stats"

echo
echo "== Free last allocation A3 =="
echo "$A3" > "$PARAM/free"

echo
echo "== Final stats =="
cat "$PARAM/stats"

echo
echo "== Negative tests =="

if printf '0\n' > "$PARAM/alloc" 2>/dev/null; then
    echo "ERROR: zero-byte allocation was accepted"
    exit 1
else
    echo "OK: zero-byte allocation rejected"
fi

if printf '9999999999\n' > "$PARAM/alloc" 2>/dev/null; then
    echo "ERROR: oversized allocation was accepted"
    exit 1
else
    echo "OK: oversized allocation rejected"
fi

if printf '0x1\n' > "$PARAM/free" 2>/dev/null; then
    echo "ERROR: invalid free address was accepted"
    exit 1
else
    echo "OK: invalid free address rejected"
fi

echo
echo "Test completed."
