# Auto-Scan

[![GitHub](https://img.shields.io/github/license/ABackerNINI/ascan)](https://github.com/ABackerNINI/ascan/blob/master/LICENSE)

Auto scan c/c++ project and create simple makefile.

## Usage

Ascan is suitable for c/c++ projects that are:

1. Simple structured that all source codes are in one directory.
2. Source codes are `.h` or `.c` or `.cpp` or `.cc`.
3. No spaces in the source-file names.

`cd` to the project directory and run `ascan`.

## Installation

Run the following commands to install:

```bash
git clone git@github.com:ABackerNINI/ascan.git
cd ascan/src
chmod +x install.sh
./install.sh
```

## Future Work

1. Support multi-directory projects.
2. Support files with other extension, such as `.hpp`.
3. Support files that contain spaces in their names.

## LICENSE

[`GNU General Public License v3.0`](https://github.com/ABackerNINI/ascan/blob/master/LICENSE)
