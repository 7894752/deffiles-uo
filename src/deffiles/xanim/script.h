/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Token reader for model and animation export files.

#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>

struct scriptmarker_t {
    bool tokenready;
    int offset;
    int line;
    char token[256];
};

class Script {
	bool tokenready;
	std::string filename;
	const char *m_script_p;
	const char *m_end_p;
	int line;
	char token[256];
	bool releaseBuffer;
	const char *buffer;
	int length;
public:
    void MarkPosition(scriptmarker_t *mark);
    void RestorePosition(scriptmarker_t *mark);
    const char *GetRaw();
    const char *GetExpression(bool crossline);
    bool GetSpecific(const char *match);
    int LinesInFile();
    const char *GetLine(bool crossline);
    const char *Filename();
    int GetLineNumber();
    bool TokenAvailable(bool crossline);
    bool CommentAvailable(bool crossline);
    void UnGetToken();
    bool AtString(bool crossline);
    const char *Token();
	void Close();
	void Reset();
	bool SkipToEOL();
	protected:
    bool AtComment();
public:
	void Error( const char *fmt, ... );
	protected:
    void CheckOverflow();
public:
	void SkipWhiteSpace( bool crossline );
	void SkipNonToken( bool crossline );
	const char *GetString( bool crossline );
	~Script();
	Script();
	const char *GetToken( bool crossline );
	void Skip( const char *match, bool crossline );
	int GetInteger( bool crossline );
	double GetDouble( bool crossline );
	float GetFloat( bool crossline );
	void GetVector( float *v, bool crossline );
	void Parse( const char *data, int size, const char *name );
	void LoadFile( const char *name );
};



#endif
