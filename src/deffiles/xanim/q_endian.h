// Byte order accessors for the little-endian converter tools.
#ifndef Q_ENDIAN_H
#define Q_ENDIAN_H

extern "C" inline short LittleShort( short l );
extern "C" inline int LittleLong( int l );
extern "C" inline float LittleFloat( float l );

extern "C" short LittleShort( short l ) { return l; }
extern "C" int LittleLong( int l ) { return l; }
extern "C" float LittleFloat( float l ) { return l; }

#endif
