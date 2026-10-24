/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#ifndef SHADERLIB_H
#define SHADERLIB_H

class ConvertValue;

struct ShaderType {
	ShaderType *next;
	char name[0x400];
	char path[0x400];
	char *text;
};

struct ShaderFile {
	ShaderFile *next;
	char path[0x400];
};

class ShaderInfo {
public:

	ConvertValue *cv;
	char gameDir[0x40];
	char dir[0x40];
	ShaderType *types;
	char *text;
	ShaderFile *files;
	bool loaded;

	ShaderInfo();
	~ShaderInfo();

	char *ReadShaderFileData();
	const char *FindShaderText( const char *name );
protected:
	const char *FindShaderTypeText( const char *name );

};

extern ShaderInfo shaderLibrary;

#include "surfaceflags.h"

#endif
