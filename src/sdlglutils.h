#pragma once

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

GLuint loadTexture(const char * filename,int* width=0,int* height=0,bool useMipMap = true);
int takeScreenshot(const char * filename);
