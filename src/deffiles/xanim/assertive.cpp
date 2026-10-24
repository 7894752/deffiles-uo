/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Assertion reporting with symbolic stack traces from linker map files.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "assertive.h"

static int Assert_DoStackTrace( char *buffer, int maxFrames, int skipFrames );

// Shared assertion state, followed by the reporting callback and map cache.
extern "C" int com_errorEntered = 0;
void ( *AssertCallback )( const char *message ) = 0;

/*
=============
FixWindowsDesktop
=============
*/
void FixWindowsDesktop() {
	unsigned short ramp[3][256];
	unsigned short i;
	HWND window;
	HDC dc;

	ChangeDisplaySettingsA( NULL, 0 );
	window = GetDesktopWindow();
	dc = GetDC( window );
	for ( i = 0; i < 256; i++ ) {
		ramp[0][i] = i * 257;
		ramp[1][i] = i * 257;
		ramp[2][i] = i * 257;
	}
	SetDeviceGammaRamp( dc, ramp );
	ReleaseDC( window, dc );
}

/*
=============
RestoreWindow
=============
*/
void RestoreWindow() {
}

/*
=============
SetAssertCallbackHandler
=============
*/
extern "C" void SetAssertCallbackHandler( void ( *hook )( const char *message ) ) {
	AssertCallback = hook;
}

/*
=============
MyAssertHandler
=============
*/
int MyAssertHandler( const char *expression, const char *file, int line , ...) {
	char szMessage[0x8000];
	char *message;
	char szModule[260];
	HGLOBAL global;
	char *text;

	if ( !GetModuleFileNameA( 0, szModule, sizeof( szModule ) ) ) {
		strcpy( szModule, "<unknown application>" );
	}

	message = szMessage;
	message += sprintf( message, "Expression:\r\n\t%s\r\n\r\nModule:\t%s\r\nFile:\t%s\r\nLine:\t%d\r\n\r\n",
		expression, szModule, file, line );
	message += Assert_DoStackTrace( message, 32, 1 );

	if ( OpenClipboard( GetDesktopWindow() ) ) {
		EmptyClipboard();
		global = GlobalAlloc( GMEM_MOVEABLE, strlen( szMessage ) + 1 );
		if ( global ) {
			text = (char *)GlobalLock( global );
			if ( text ) {
				strcpy( text, szMessage );
				GlobalUnlock( global );
				SetClipboardData( CF_TEXT, global );
			}
		}
		CloseClipboard();
	}

	if ( AssertCallback ) {
		AssertCallback( szMessage );
	}

	switch ( MessageBoxA( 0, szMessage, "ASSERTION FAILURE... (this text is on the clipboard)",
		MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND | MB_DEFBUTTON2 ) ) {
	case IDABORT:
		ExitProcess( -1 );
	case IDIGNORE:
		if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) return 2;
		return 1;
	}
	return 0;
}

struct LineNumberAddress {
public:
	const char *file;
	unsigned address;
	int number;
	LineNumberAddress *next;

	/*
	=============
	LineNumberAddress::LineNumberAddress
	=============
	*/
	LineNumberAddress( LineNumberAddress *head ) {
		next = head;
	}

	/*
	=============
	LineNumberAddress::~LineNumberAddress
	=============
	*/
	~LineNumberAddress() {
		delete next;
	}
};

struct FunctionAddress {
public:
	const char *object;
	unsigned address;
	const char *name;
	FunctionAddress *next;

	/*
	=============
	FunctionAddress::FunctionAddress
	=============
	*/
	FunctionAddress( FunctionAddress *head ) {
		next = head;
	}

	/*
	=============
	FunctionAddress::~FunctionAddress
	=============
	*/
	~FunctionAddress() {
		delete next;
	}
};

struct StringInfo {
public:
	char *string;
	unsigned hash;
	StringInfo *next;

	/*
	=============
	StringInfo::StringInfo
	=============
	*/
	StringInfo( const char *source, unsigned sourceHash, StringInfo *head ) {
		next = head;
		string = new char[strlen( source ) + 1];
		strcpy( string, source );
		hash = sourceHash;
	}

	/*
	=============
	StringEntry::~StringEntry
	=============
	*/
	~StringInfo() {
		delete next;
		delete[] string;
	}
};

class MapFileInfo {
public:
	const char *name;
	unsigned base;
	unsigned end;
	FunctionAddress *symbols;
	LineNumberAddress *lines;
	StringInfo *strings[256];
	MapFileInfo *next;

	/*
	=============
	MapFileInfo::MapFileInfo
	=============
	*/
	MapFileInfo() {
		name = 0;
		base = 0;
		end = 0;
		symbols = 0;
		lines = 0;
		next = 0;
		memset( strings, 0, sizeof( strings ) );
	}

	/*
	=============
	Module::~Module
	=============
	*/
	~MapFileInfo() {
		unsigned i;

		delete next;
		delete[] name;
		delete symbols;
		delete lines;
		for ( i = 0; i < 256; i++ ) {
			delete strings[i];
		}
	}
};


static MapFileInfo *modules = 0;

/*
=============
HashString
=============
*/
static unsigned HashString( const char *string ) {
	unsigned hash;

	hash = 0;
	while ( *string ) {
		hash = hash * 31337 + *string;
		string++;
	}
	return hash;
}

// returns the unique copy of string owned by the module
/*
=============
InternString
=============
*/
static const char *InternString( MapFileInfo *module, const char *string ) {
	StringInfo *entry;
	unsigned hash, index;

	hash = HashString( string );
	index = hash & 255;
	for ( entry = module->strings[index]; entry; entry = entry->next ) {
		if ( entry->hash == hash && !strcmp( entry->string, string ) ) {
			return entry->string;
		}
	}

	module->strings[index] = new StringInfo( string, hash, module->strings[index] );
	return module->strings[index]->string;
}

/*
=============
ParseError
=============
*/
static void ParseError( const char *message ) {
	MessageBoxA( 0, message, ".map parse error", MB_ICONERROR );
}

/*
=============
ReadLine
=============
*/
static bool ReadLine( char *buffer, int size, FILE *f, bool eofIsError ) {
	if ( fgets( buffer, size, f ) ) {
		return true;
	}
	if ( eofIsError ) {
		ParseError( "Unexpected end-of-file parsing .map file" );
	}
	return false;
}

/*
=============
SkipLines
=============
*/
static bool SkipLines( int count, char *buffer, int size, FILE *f, bool eofIsError ) {
	int i;

	for ( i = 0; i < count; i++ ) {
		if ( !ReadLine( buffer, size, f, true ) ) {
			return false;
		}
	}
	return true;
}

/*
=============
ParseMapFile
=============
*/
static bool ParseMapFile( FILE *f, MapFileInfo *module, unsigned base ) {
	char szLineBuffer[0x8000];
	int i;
	int iLoadAddress;
	int iGroup, iOffset, iAddress;
	int iLineNumber[4], iLineGroup[4], iLineOffset[4];
	char szFunction[1024], szFile[1024];
	const char *file;
	unsigned endOffset;
	char *p, *end;
	int count;

	szLineBuffer[sizeof( szLineBuffer ) - 1] = 0;

	do {
		if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
	} while ( sscanf( szLineBuffer, " Preferred load address is %x\r\n", &iLoadAddress ) != 1 );
	module->base = base;

	if ( !SkipLines( 2, szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
		return false;
	}

	// segments
	for ( ;; ) {
		if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
		if ( !strcmp( szLineBuffer, "\r\n" ) ) {
			goto publics;
		}
		if ( sscanf( szLineBuffer, "%x:%x %xH %s %s", &iGroup, &iOffset, &iAddress, szFunction, szFile ) != 5 ) {
			ParseError( "Unknown line format in the segments section" );
			return false;
		}
		if ( iGroup == 1 ) {
			endOffset = iOffset + iAddress + base + 0x1000;
			if ( module->end < endOffset ) module->end = endOffset;
		}
	}

publics:

	// public symbols
	do {
		if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
	} while ( strcmp( szLineBuffer, "  Address         Publics by Value              Rva+Base     Lib:Object\r\n" ) );
	if ( !SkipLines( 1, szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
		return false;
	}
	for ( ;; ) {
		if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
		if ( !strcmp( szLineBuffer, "\r\n" ) ) {
			goto statics;
		}
		if ( sscanf( szLineBuffer, "%x:%x %s %x", &iGroup, &iOffset, szFunction, &iAddress ) != 4 ) {
			ParseError( "Unknown line format in the public symbols section" );
			return false;
		}
		file = strrchr( szLineBuffer, ' ' );
		if ( !file || sscanf( file + 1, "%s", szFile ) != 1 ) {
			ParseError( "Couldn't parse file name in the public symbols section" );
			return false;
		}
		module->symbols = new FunctionAddress( module->symbols );
		module->symbols->object = InternString( module, szFile );
		module->symbols->address = iAddress - iLoadAddress + base;
		module->symbols->name = InternString( module, szFunction );
	}

statics:
	if ( !SkipLines( 2, szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
		return false;
	}
	if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
		return false;
	}
	if ( !strcmp( szLineBuffer, " Static symbols\r\n" ) ) {
		if ( !SkipLines( 1, szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
		for ( ;; ) {
			if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, false ) ) break;
			if ( !szLineBuffer[0] || !strcmp( szLineBuffer, "\r\n" ) ) {
				break;
			}
			if ( sscanf( szLineBuffer, "%x:%x %s %x", &iGroup, &iOffset, szFunction, &iAddress ) != 4 ) {
				ParseError( "Unknown line format in the static symbols section" );
				return false;
			}
			file = strrchr( szLineBuffer, ' ' );
			if ( !file || sscanf( file + 1, "%s", szFile ) != 1 ) {
				ParseError( "Couldn't parse file name in the static symbols section" );
				return false;
			}
			module->symbols = new FunctionAddress( module->symbols );
			module->symbols->object = InternString( module, szFile );
			module->symbols->address = iAddress;
			module->symbols->name = InternString( module, szFunction );
		}
	}

	// line numbers
	for ( ;; ) {
			if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, false ) ) break;
		if ( strncmp( szLineBuffer, "Line numbers for ", 17 ) ) {
			ParseError( "Expected line number section" );
			return false;
		}
		p = strchr( szLineBuffer, '(' );
		if ( !p ) {
			ParseError( "Couldn't find '(' for the name of the source file in line number section" );
			return false;
		}
		end = strchr( p, ')' );
		if ( !end ) {
			ParseError( "Couldn't find ')' for the name of the source file in line number section" );
			return false;
		}
		strncpy( szFile, p + 1, end - p - 1 );
		szFile[end - p - 1] = 0;
		file = InternString( module, szFile );

		if ( !SkipLines( 1, szLineBuffer, sizeof( szLineBuffer ) - 1, f, true ) ) {
			return false;
		}
		for ( ;; ) {
			if ( !ReadLine( szLineBuffer, sizeof( szLineBuffer ) - 1, f, false ) ) break;
			if ( !szLineBuffer[0] || !strcmp( szLineBuffer, "\r\n" ) ) {
				break;
			}
			count = sscanf( szLineBuffer, "%i %x:%x %i %x:%x %i %x:%x %i %x:%x\r\n",
				&iLineNumber[0], &iLineGroup[0], &iLineOffset[0],
				&iLineNumber[1], &iLineGroup[1], &iLineOffset[1],
				&iLineNumber[2], &iLineGroup[2], &iLineOffset[2],
				&iLineNumber[3], &iLineGroup[3], &iLineOffset[3] );
			if ( count % 3 != 0 || count / 3 <= 0 ) {
				ParseError( "unknown line format in the line number section" );
				return false;
			}
			for ( i = 0; i * 3 < count; i++ ) {
				module->lines = new LineNumberAddress( module->lines );
				module->lines->file = file;
				module->lines->address = iLineOffset[i] + base + 0x1000;
				module->lines->number = iLineNumber[i];
			}
		}
	}
	return true;
}

/*
=============
GetModuleBase
=============
*/
static unsigned GetModuleBase( const char *mapName ) {
	char szModuleName[260];
	int length, i;
	HMODULE module;

	length = strlen( mapName );
	for ( i = length - 1; i >= 0; i-- ) {
		if ( mapName[i] == '.' || mapName[i] == '/' || mapName[i] == '\\' ) {
			break;
		}
	}
	if ( i >= 0 && mapName[i] == '.' ) {
		length = i;
	}
	memcpy( szModuleName, mapName, length );
	strcpy( szModuleName + length, ".exe" );
	module = GetModuleHandleA( szModuleName );
	if ( module ) return (unsigned)module;
	strcpy( szModuleName + length, ".dll" );
	module = GetModuleHandleA( szModuleName );
	return (unsigned)module;
}

/*
=============
LoadMapFilesForDir
=============
*/
static void LoadMapFilesForDir( const char *dir ) {
	WIN32_FIND_DATA wfd;
	HANDLE handle;
	unsigned base;
	char szSearchSpec[2048];
	char szFileName[2048];
	char szMapName[2048];
	MapFileInfo *m;
	FILE *f;

	if ( *dir ) {
		sprintf( szSearchSpec, "%s/*.map", dir );
	} else {
		strcpy( szSearchSpec, "*.map" );
	}

	handle = FindFirstFileA( szSearchSpec, &wfd );
	if ( handle != INVALID_HANDLE_VALUE ) {

	do {
		base = GetModuleBase( wfd.cFileName );
		if ( !base ) {
			continue;
		}

		strcpy( szMapName, wfd.cFileName );
		szMapName[strlen( szMapName ) - 4] = 0;
		for ( m = modules; m; m = m->next ) {
			if ( !stricmp( m->name, szMapName ) ) {
				goto nextFile;
			}
		}

		if ( *dir ) {
			sprintf( szFileName, "%s/%s", dir, wfd.cFileName );
		} else {
			strcpy( szFileName, wfd.cFileName );
		}
		f = fopen( szFileName, "rb" );
		if ( f ) {
			m = new MapFileInfo;
			if ( ParseMapFile( f, m, base ) ) {
				m->name = InternString( m, szMapName );
				m->next = modules;
				modules = m;
			} else {
				delete m;
			}
			fclose( f );
		}

nextFile:
		;
	} while ( FindNextFileA( handle, &wfd ) );

	FindClose( handle );
	}
}

/*
=============
LoadMapFiles
=============
*/
static void LoadMapFiles() {
	LoadMapFilesForDir( "" );
}

// writes one stack frame description, returns the number of characters written
/*
=============
FormatStackFrame
=============
*/
static int FormatStackFrame( char *out, unsigned address, bool *stop ) {
	char *p;
	MapFileInfo *module;
	FunctionAddress *symbol, *bestSymbol;
	LineNumberAddress *line, *bestLine;
	const char *moduleName;
	char szBuffer[1024];
	char *file;
	const char *functionName;
	char *filename;

	p = out;
	*stop = false;
	bestSymbol = 0;
	bestLine = 0;
	moduleName = "<unknown module>";

	for ( module = modules; module; module = module->next ) {
		if ( address < module->base || address >= module->end ) continue;
		moduleName = module->name;
		for ( symbol = module->symbols; symbol; symbol = symbol->next ) {
			if ( symbol->address <= address && ( !bestSymbol || bestSymbol->address < symbol->address ) ) {
				bestSymbol = symbol;
			}
		}
		for ( line = module->lines; line; line = line->next ) {
			if ( line->address <= address && ( !bestLine || bestLine->address < line->address ) ) {
				bestLine = line;
			}
		}
		break;
	}

	p += sprintf( p, "%s:\t", moduleName );

	if ( bestSymbol ) {
		functionName = bestSymbol->name;
		if ( *functionName == '_' || *functionName == '?' ) {
			functionName++;
		}
		strcpy( szBuffer, functionName );
		file = strchr( szBuffer, '@' );
		if ( file ) {
			*file = 0;
		}
		functionName = szBuffer;
		if ( !strcmp( functionName, "WinMain" ) || !strcmp( functionName, "main" ) ) {
			*stop = true;
		}
	} else {
		functionName = "<unknown function>";
		*stop = true;
	}

	if ( bestLine ) {
		filename = strrchr( bestLine->file, '\\' );
		if ( filename ) filename++;
		else filename = (char *)bestLine->file;
		p += sprintf( p, "%s\t\t...%s, line %i", functionName, filename, bestLine->number );
	} else if ( bestSymbol ) {
		filename = strrchr( bestSymbol->object, '\\' );
		if ( filename ) filename++;
		else filename = (char *)bestSymbol->object;
		p += sprintf( p, "%s\t\t...%s", functionName, filename );
	} else {
		p += sprintf( p, "%s", functionName );
	}

	p += sprintf( p, "\r\n" );
	return p - out;
}

/*
=============
Assert_DoStackTrace
=============
*/
static int Assert_DoStackTrace( char *buffer, int maxFrames, int skipFrames ) {
	unsigned *frame;
	unsigned address;
	int i;
	char *out;
	bool bAbort;

	LoadMapFiles();

	__asm mov frame, ebp

	out = buffer;
	bAbort = false;
	for ( i = 0; i < maxFrames + skipFrames; i++ ) {
		__asm {
			mov ecx, frame
			mov eax, [ecx]
			mov frame, eax
			mov eax, [ecx + 4]
			mov address, eax
		}
		if ( i >= skipFrames ) {
			out += FormatStackFrame( out, address - 5, &bAbort );
			if ( bAbort ) {
				break;
			}
		}
	}
	return out - buffer;
}

