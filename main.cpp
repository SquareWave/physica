#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#define assert(condition) (condition) || (*((i32*)0) = 1)

// trying this out:
// http://buffered.io/posts/the-magic-of-unity-builds/
#include "sdl_platform.cpp"
#include "game_render.cpp"
#include "hashmap.cpp"
#include "physica.cpp"
#include "test.cpp"
#pragma clang diagnostic pop