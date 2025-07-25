#!/bin/bash
exec > /dev/null 2>&1
git submodule sync
git submodule update -f --init --recursive --checkout
git submodule update -f --remote --recursive