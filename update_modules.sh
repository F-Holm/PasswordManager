#!/bin/bash
git submodule sync
git submodule update -f --init --recursive --checkout
git submodule update -f --remote --recursive