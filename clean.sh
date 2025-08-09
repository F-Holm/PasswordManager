#!/bin/bash
exec > /dev/null 2>&1
#./reset_modules.sh
rm -rf ./build
rm -rf ./install
git clean -Xfd