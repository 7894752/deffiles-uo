// Token-file reader used by the model and animation exporters.
#include "script.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "assertive.h"

extern "C" {
#include "cmdlib.h"
	}
extern "C" void Com_Error(int code, const char *fmt, ...);

Script::~Script()
	{
	Close();
	}

Script::Script()
	{
	buffer = NULL;
	m_script_p = NULL;
	m_end_p = NULL;
	line = 0;
	releaseBuffer = false;
	tokenready = false;
	memset(token, 0, sizeof(token));
	}

void Script::Close
	(
	void
	)

	{
	if (releaseBuffer && buffer) delete [] buffer;
	buffer = NULL;
	m_script_p = NULL;
	m_end_p = NULL;
	line = 0;
	releaseBuffer = false;
	tokenready = false;
	memset(token, 0, sizeof(token));
	}

/*
==============
=
= Filename
=
==============
*/

const char *Script::Filename
	(
	void
	)

	{
	return filename.c_str();
	}

/*
==============
=
= GetLineNumber
=
==============
*/

int Script::GetLineNumber
	(
	void
	)

	{
	return line;
	}

/*
==============
=
= Reset
=
==============
*/

void Script::Reset
	(
	void
	)

	{
	m_script_p = buffer;
	line = 1;
	tokenready = false;
	}

/*
==============
=
= MarkPosition
=
==============
*/

void Script::MarkPosition
	(
	scriptmarker_t *mark
	)

	{
	IW_ASSERT(mark);

	mark->tokenready = tokenready;
	mark->offset = m_script_p - buffer;
	mark->line = line;
	strcpy(mark->token, token);
	}

/*
==============
=
= RestorePosition
=
==============
*/

void Script::RestorePosition
	(
	scriptmarker_t *mark
	)

	{
	IW_ASSERT(mark);
	tokenready = mark->tokenready;
	m_script_p = buffer + mark->offset;
	line = mark->line;
	strcpy(token, mark->token);


	IW_ASSERT(m_script_p <= m_end_p);
	if (m_script_p > m_end_p) m_script_p = m_end_p;
	}

/*
==============
=
= SkipToEOL
=
==============
*/

bool Script::SkipToEOL
	(
	void
	)

	{
	if (m_script_p >= m_end_p) return true;
	while (*m_script_p != '\n') {
		if (m_script_p >= m_end_p) return true;
		m_script_p++;
	}
	return false;
	}

/*
==============
=
= CheckOverflow
=
==============
*/

void Script::CheckOverflow
	(
	void
	)

	{
	if (m_script_p >= m_end_p) Error("End of token file reached prematurely");
	}

/*
==============
=
= SkipWhiteSpace
=
==============
*/

void Script::SkipWhiteSpace
	(
	bool crossline
	)

	{
	CheckOverflow();
	while (*m_script_p <= ' ' || *m_script_p == ',') {
		if (*m_script_p++ == '\n') {
			if (!crossline) Error("line is incomplete\n");
			line++;
		}
		CheckOverflow();
	}
	}

/*
==============
=
= AtComment
=
==============
*/

bool Script::AtComment
	(
	void
	)

	{
	if (m_script_p >= m_end_p) return false;
	if (*m_script_p == ';') return true;
	if (m_script_p + 1 >= m_end_p) return false;
	if (*m_script_p == '/' && m_script_p[1] == '/') return true;
	return false;
	}

/*
==============
=
= SkipNonToken
=
==============
*/

void Script::SkipNonToken
	(
	bool crossline
	)

	{
	SkipWhiteSpace(crossline);
	while (AtComment()) {
		SkipToEOL();
		SkipWhiteSpace(crossline);
	}
	}

/*
==============
=
= TokenAvailable
=
==============
*/

bool Script::TokenAvailable
	(
	bool crossline
	)

	{
	bool done;
	if (m_script_p >= m_end_p) return false;
	while (true) {
		while (*m_script_p <= ' ') {
			if (*m_script_p == '\n') {
				if (!crossline) return false;
				line++;
			}
			m_script_p++;
			if (m_script_p >= m_end_p) return false;
		}
		if (AtComment()) {
			done = SkipToEOL();
			if (done) return false;
		} else {
			break;
		}
	}
	return true;
	}

/*
==============
=
= CommentAvailable
=
==============
*/

bool Script::CommentAvailable
	(
	bool crossline
	)

	{
	const char *p;
	p = m_script_p;
	if (p >= m_end_p) return false;
	while (*p <= ' ') {
		if (*p == '\n') {
			if (!crossline) return false;
		}
		p++;
		if (p >= m_end_p) return false;
	}
	return true;
	}

/*
==============
=
= UnGetToken
=
==============
*/

void Script::UnGetToken
	(
	void
	)

	{
	tokenready = true;
	}

/*
==============
=
= AtString
=
==============
*/

bool Script::AtString
	(
	bool crossline
	)

	{
	SkipNonToken(crossline);
	return *m_script_p == '"';
	}

/*
==============
=
= GetToken
=
==============
*/

const char *Script::GetToken
	(
	bool crossline
	)

	{
	char *token_p;
	if (tokenready) {
		tokenready = false;
		return token;
	}
	SkipNonToken(crossline);
	if (*m_script_p == '"') return GetString(crossline);
	token_p = token;
	while (*m_script_p > ' ' && *m_script_p != ',' && !AtComment()) {
		if (*m_script_p == '\\' && m_script_p < m_end_p - 1) {
			m_script_p++;
			switch (*m_script_p) {
			case 'n': *token_p++ = '\n'; break;
			case 'r': *token_p++ = '\n'; break;
			case '\'': *token_p++ = '\''; break;
			case '"': *token_p++ = '"'; break;
			case '\\': *token_p++ = '\\'; break;
			default: *token_p++ = *m_script_p; break;
			}
			m_script_p++;
		} else {
			*token_p++ = *m_script_p++;
		}
		if (token_p == token + sizeof(token)) Error("Token too large");
		if (m_script_p == m_end_p) break;
	}
	*token_p = 0;
	return token;
	}

/*
==============
=
= GetLine
=
==============
*/

const char *Script::GetLine
	(
	bool crossline
	)

	{
	const char *start;
	int len;
	if (tokenready) {
		tokenready = false;
		return token;
	}
	SkipNonToken(crossline);
	start = m_script_p;
	SkipToEOL();
	len = m_script_p - start;
	if (len < (int)sizeof(token) - 1) {
		memcpy(token, start, len);
		token[len] = 0;
	} else {
		Error("Token too large");
	}
	return token;
	}

/*
==============
=
= GetRaw
=
==============
*/

const char *Script::GetRaw
	(
	void
	)

	{
	const char *start;
	int len;
	SkipWhiteSpace(true);
	start = m_script_p;
	SkipToEOL();
	len = m_script_p - start;
	if (len < (int)sizeof(token) - 1) {
		memset(token, 0, sizeof(token));
		memcpy(token, start, len);
	} else {
		Error("Token too large");
	}
	return token;
	}

/*
==============
=
= GetString
=
==============
*/

const char *Script::GetString
	(
	bool crossline
	)

	{
	int startLine;
	char *token_p;
	if (tokenready) {
		tokenready = false;
		return token;
	}
	SkipNonToken(crossline);
	if (*m_script_p != '"') Error("Expecting string");
	m_script_p++;
	startLine = line;
	token_p = token;
	while (*m_script_p != '"') {
		if (*m_script_p == '\n') Error("Line is incomplete while reading string");
		if (*m_script_p == '\\' && m_script_p < m_end_p - 1) {
			m_script_p++;
			switch (*m_script_p) {
			case 'n': *token_p++ = '\n'; break;
			case 'r': *token_p++ = '\n'; break;
			case '\'': *token_p++ = '\''; break;
			case '"': *token_p++ = '"'; break;
			case '\\': *token_p++ = '\\'; break;
			default: *token_p++ = *m_script_p; break;
			}
			m_script_p++;
		} else {
			*token_p++ = *m_script_p++;
		}
		if (m_script_p >= m_end_p) Error("End of token file reached prematurely while reading string");
		if (token_p == token + sizeof(token)) Error("String too large");
	}
	*token_p = 0;
	m_script_p++;
	return token;
	}

/*
==============
=
= GetExpression
=
==============
*/

const char *Script::GetExpression
	(
	bool crossline
	)

	{
	char *token_p;
	if (tokenready) {
		tokenready = false;
		return token;
	}
	SkipNonToken(crossline);
	if (*m_script_p == '"') return GetString(crossline);
	token_p = token;
	while (*m_script_p != ',' && !AtComment()) {
		if (*m_script_p == '\\' && m_script_p < m_end_p - 1) {
			m_script_p++;
			switch (*m_script_p) {
			case 'n': *token_p++ = '\n'; break;
			case 'r': *token_p++ = '\n'; break;
			case '\'': *token_p++ = '\''; break;
			case '"': *token_p++ = '"'; break;
			case '\\': *token_p++ = '\\'; break;
			default: *token_p++ = *m_script_p; break;
			}
			m_script_p++;
		} else {
			*token_p++ = *m_script_p++;
		}
		if (token_p == token + sizeof(token)) Error("Token too large");
		if (m_script_p == m_end_p) break;
	}
	*token_p = 0;
	return token;
	}

/*
==============
=
= Skip
=
==============
*/

void Script::Skip
	(
	const char *match,
	bool crossline
	)

	{
	GetToken(crossline);
	if (strcmp(token, match)) Error("Expecting '%s'", match);
	}

/*
==============
=
= GetSpecific
=
==============
*/

bool Script::GetSpecific
	(
	const char *match
	)

	{
	do {
		if (!TokenAvailable(true)) return false;
		GetToken(true);
	} while (strcmp(token, match));
	return true;
	}

/*
==============
=
= GetInteger
=
==============
*/

int Script::GetInteger
	(
	bool crossline
	)

	{
	GetToken(crossline);
	return atoi(token);
	}

/*
==============
=
= GetDouble
=
==============
*/

double Script::GetDouble
	(
	bool crossline
	)

	{
	GetToken(crossline);
	return atof(token);
	}

/*
==============
=
= GetFloat
=
==============
*/

float Script::GetFloat
	(
	bool crossline
	)

	{
	return (float)GetDouble(crossline);
	}

/*
==============
=
= GetVector
=
==============
*/

void Script::GetVector
	(
	float *v,
	bool crossline
	)

	{
	v[0] = GetFloat(crossline);
	v[1] = GetFloat(crossline);
	v[2] = GetFloat(crossline);
	}

/*
==============
=
= LinesInFile
=
==============
*/

int Script::LinesInFile
	(
	void
	)

	{
	bool oldtokenready;
	const char *oldscript_p;
	int oldline;
	char temp_token[256];
	int count;

	oldtokenready = tokenready;
	oldscript_p = m_script_p;
	oldline = line;
	strcpy(temp_token, token);
	count = 0;
	Reset();
	while (TokenAvailable(true)) {
		GetLine(true);
		count++;
	}
	tokenready = oldtokenready;
	m_script_p = oldscript_p;
	line = oldline;
	strcpy(token, temp_token);
	return count;
	}

/*
==============
=
= Parse
=
==============
*/

void Script::Parse
	(
	const char *data,
	int size,
	const char *name
	)

	{
	Close();
	buffer = data;
	Reset();
	m_end_p = m_script_p + size;
	length = size;
	filename = name;
	}

/*
==============
=
= LoadFile
=
==============
*/

void Script::LoadFile
	(
	const char *name
	)

	{
	int size;
	char *data;
	FILE *file;
	Close();
	file = fopen(name, "rt");
	if (!file) ::Error("Script::LoadFile :: Couldn't load %s\n", name);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = new char[size];
	fread(data, 1, size, file);
	fclose(file);
	Parse(data, size, name);
	releaseBuffer = true;
	}

/*
==============
=
= Token
=
==============
*/

const char *Script::Token
	(
	void
	)

	{
	return token;
	}

/*
==============
=
= Error
=
==============
*/

void Script::Error
	(
	const char *fmt,
	...
	)

	{
	va_list args;
	char text[1024];
	va_start(args, fmt);
	vsprintf(text, fmt, args);
	va_end(args);
	::Error("%s(%d): %s\n", filename.c_str(), line, text);
	}
