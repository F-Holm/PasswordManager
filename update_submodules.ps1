#!/usr/bin/env pwsh

git submodule sync
git submodule update --init --recursive --force
git submodule update --remote
git submodule update --init --recursive
