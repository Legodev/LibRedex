#!/usr/bin/env bash
rm -rf build
find Binaries/ -mindepth 1 -maxdepth 1 -type d -exec rm -rf {} \;
