#!/usr/bin/zsh

rm -rf ~/.local/include/vql
rm -rf ~/.local/lib/vql

cd $(dirname "$0")
cmake --install build --prefix ~/.local >/dev/null
