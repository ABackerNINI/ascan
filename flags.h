#pragma once

#ifndef _AUTO_SCAN_FLAGS_H_
#define _AUTO_SCAN_FLAGS_H_

#define FLAG_A 0x1  // flag -a: overwrite all sections
#define FLAG_B 0x2  // flag -b: put binaries to 'build' subdirectory
#define FLAG_F 0x4  // flag -f: force overwrite
#define FLAG_G 0x8  // flag -g: add '-g' flag to cflags or cxxflags
#define FLAG_C 0x10 // flag -c: add header comments

#endif  //_AUTO_SCAN_FLAGS_H_
