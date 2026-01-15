#!/bin/bash

make || exit 1
sudo cp bin/ascan /usr/local/bin/ascan
