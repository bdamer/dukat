#pragma once

// Dukat Library version
#define DUKAT_VERSION 1

#ifdef __ANDROID__
// Include latest header files possible
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif
#else
#include <GL/glew.h>
#endif

// Define OPENGL_VERSION targeted by the application.
#ifdef __ANDROID__
// OpenGL ES 3.0
#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 0
#else
// OpenGL 2.1 compat mode
// #define OPENGL_MAJOR_VERSION 2
// #define OPENGL_MINOR_VERSION 1
// OpenGL 3.1
#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 1
#endif

#ifndef OPENGL_VERSION
#define OPENGL_VERSION (10 * OPENGL_MAJOR_VERSION + OPENGL_MINOR_VERSION)
#endif

// Define either OPENGL_CORE or OPENGL_ES as shorthand to access the 
// target version & profile.
#ifdef GL_VERSION_1_1
#define OPENGL_CORE OPENGL_VERSION
#else
#define OPENGL_ES OPENGL_VERSION
#endif