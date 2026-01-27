#!/usr/bin/env pwsh

git submodule sync
git submodule update --init --recursive --depth 1 --force
git submodule update --remote --depth 1
git submodule update --init --recursive --depth 1
