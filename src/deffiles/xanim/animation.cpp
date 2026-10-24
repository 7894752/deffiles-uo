/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/
#include "xanim.h"

/*
XANIM stores little-endian scalar values and quantized quaternion keys.
*/

/*
=============
SkelModel::WriteVector
=============
*/
void SkelModel::WriteVector( float v[3] ) {
	float vec2[3];
	vec2[0] = LittleFloat( v[0] );
	vec2[1] = LittleFloat( v[1] );
	vec2[2] = LittleFloat( v[2] );
	SafeWrite( model.file, vec2, sizeof( vec2 ) );
}
/*
=============
ConvertMatToQuat
=============
*/
void ConvertMatToQuat( float m[3][3], float quat[4] ) {
	float test[4][4];
	int i;
	float result[4];
	int best;
	// Use the largest diagonal candidate to avoid division by a small component.
	test[0][0] = m[1][2] - m[2][1];
	test[0][1] = m[2][0] - m[0][2];
	test[0][2] = m[0][1] - m[1][0];
	test[0][3] = m[0][0] + m[1][1] + m[2][2] + 1.0f;
	result[0] = fabs( test[0][0] ) + fabs( test[0][1] ) + fabs( test[0][2] ) + fabs( test[0][3] );
	test[1][0] = m[0][0] - m[1][1] - m[2][2] + 1.0f;
	test[1][1] = m[1][0] + m[0][1];
	test[1][2] = m[2][0] + m[0][2];
	test[1][3] = m[1][2] - m[2][1];
	result[1] = fabs( test[1][0] ) + fabs( test[1][1] ) + fabs( test[1][2] ) + fabs( test[1][3] );
	test[2][0] = m[0][1] + m[1][0];
	test[2][1] = m[1][1] - m[0][0] - m[2][2] + 1.0f;
	test[2][2] = m[2][1] + m[1][2];
	test[2][3] = m[2][0] - m[0][2];
	result[2] = fabs( test[2][0] ) + fabs( test[2][1] ) + fabs( test[2][2] ) + fabs( test[2][3] );
	test[3][0] = m[0][2] + m[2][0];
	test[3][1] = m[1][2] + m[2][1];
	test[3][2] = m[2][2] - m[1][1] - m[0][0] + 1.0f;
	test[3][3] = m[0][1] - m[1][0];
	result[3] = fabs( test[3][0] ) + fabs( test[3][1] ) + fabs( test[3][2] ) + fabs( test[3][3] );
	best = 0;
	for ( i = 1; i < 4; i++ ) {
		if ( result[i] > result[best] ) best = i;
	}
	memcpy( quat, test[best], sizeof( test[best] ) );
	if ( quat[3] < 0 ) {
		for ( i = 0; i < 4; i++ ) {
			quat[i] = -quat[i];
		}
	}
	VectorNormalize4D( quat );
}
/*
=============
QuatToShort
=============
*/
static void QuatToShort( float *quat, short *iQuat ) {
	float S;
	int k;
	float Q2[4];
	int iQ[4];
	int w;

	VectorCopy( quat, Q2 );
	Q2[3] = quat[3];
	if ( Q2[3] < 0 ) {
		for ( k = 0; k < 4; k++ ) {
			Q2[k] = -Q2[k];
		}
	}
	// Normalize before quantizing the three stored components.
	S = Q2[0] * Q2[0] + Q2[1] * Q2[1] + Q2[2] * Q2[2] + Q2[3] * Q2[3];
	assert( S );
	S = 32767.0f / sqrt( S );

	for ( k = 0; k < 3; k++ ) {
		iQ[k] = (int)floor( S * Q2[k] + 0.5f );

		assert( iQ[k] == (short) iQ[k] );
		iQuat[k] = (short) iQ[k];
	}

	// Recover the nonnegative fourth component from the quantized vector.
	w = 32767 * 32767 - ( iQ[0] * iQ[0] + iQ[1] * iQ[1] + iQ[2] * iQ[2] );
	iQ[3] = w > 0 ? (int)floor( sqrt( (float)w ) + 0.5f ) : 0;

	assert( iQ[3] == (short) iQ[3] );
	iQuat[3] = (short) iQ[3];
}
/*
Quantize the (z, w) components of a rotation around the vertical axis.
*/
static void QuatToShort2( float *quat, short *iQuat ) {
	float S;
	int i, iQ;


	S = quat[0] * quat[0] + quat[1] * quat[1];
	assert( S );
	S = 32767.0f / sqrt( S );

	for ( i = 0; i < 2; i++ ) {
		iQ = (int)floor( S * quat[i] + 0.5f );

		assert( iQ == (short) iQ );
		iQuat[i] = (short) iQ;
	}
}
/*
=============
SkelModel::WriteQuatXYZ
=============
*/
void SkelModel::WriteQuatXYZ( const short (&iQuat)[4] ) {
	int i;
	short sQ2[3];
	sQ2[0] = iQuat[0];
	sQ2[1] = iQuat[1];
	sQ2[2] = iQuat[2];
	if ( iQuat[3] < 0 ) {
		for ( i = 0; i < 3; i++ ) {
			sQ2[i] = -sQ2[i];
		}
	}
	for ( i = 0; i < 3; i++ ) {
		sQ2[i] = LittleShort( sQ2[i] );
	}
	SafeWrite( model.file, sQ2, sizeof( sQ2 ) );
}
/*
=============
SkelModel::WriteQuatX
=============
*/
void SkelModel::WriteQuatX( short iQuat[2] ) {
	short sQ2;
	sQ2 = iQuat[0];
	if ( iQuat[1] < 0 ) {
		sQ2 = -sQ2;
	}
	sQ2 = LittleShort( sQ2 );
	SafeWrite( model.file, &sQ2, sizeof( sQ2 ) );
}
// Notetracks are named animation events, such as footsteps or shell ejection.
// The export stores channels of names and frame numbers; the file stores
// one flat list ordered by frame. The count fits in a single byte.
// The channel and frame limits are checked when reading the export.
// Equal frames may fire together, so no stable ordering is needed.
/*
=============
NoteTrackCompare
=============
*/
int NoteTrackCompare( const void *a, const void *b ) {
	return ( (const noteTrack_t *)a )->frameIndex - ( (const noteTrack_t *)b )->frameIndex;
}

/*
=============
SkelModel::SaveNoteTracks
=============
*/
void SkelModel::SaveNoteTracks() {
	int i;
	int j;
	NotetrackChannel *channel;
	unsigned short sFrameIndex;
	int totalNotetracks;
	noteTrack_t noteTrackList[MAX_NOTETRACK];
	noteTrack_t *noteTrack;
	unsigned char cTotalNotetracks;

	// Flatten the channel lists.
	// Each entry retains its event name and sFrameIndex.
	totalNotetracks = 0;
	for ( i = 0; i < model.numChannels; i++ ) {
		channel = &model.channels[i];
		for ( j = 0; j < channel->numFrames; j++ ) {
			assert( totalNotetracks < MAX_NOTETRACK );
			noteTrack = &noteTrackList[totalNotetracks];
			strcpy( noteTrack->name, channel->name );
			noteTrack->frameIndex = channel->frames[j];
			totalNotetracks++;
		}
	}

	// The runtime reads these events in sFrameIndex order.
	// Simultaneous events do not require a stable sort.
	qsort( noteTrackList, totalNotetracks, sizeof( noteTrack_t ), NoteTrackCompare );

	assert( totalNotetracks == (unsigned char) totalNotetracks );
	cTotalNotetracks = (unsigned char) totalNotetracks;
	SafeWrite( model.file, &cTotalNotetracks, 1 );

	for ( i = 0; i < totalNotetracks; i++ ) {
		noteTrack = &noteTrackList[i];
		SafeWrite( model.file, noteTrack->name, strlen( noteTrack->name ) + 1 );

		assert( noteTrack->frameIndex == (unsigned short) noteTrack->frameIndex );
		sFrameIndex = (unsigned short) noteTrack->frameIndex;
		SafeWrite( model.file, &sFrameIndex, 2 );
	}
}
/*
Write quantized quaternion keys; return true when all keys rotate around z.
*/
bool SkelModel::SaveQuats( float (*_quatFrames)[4], bool bKeepIdentity ) {
	union { unsigned char *b; unsigned short *s; } quatIndices;
	int i;
	float frac;
	int frame;
	int end;
	float lerpedQuat[4];
	float *quat;
	short (*sQuat)[4];
	short *prevQuat;
	bool bByteIndices;
	unsigned short numQuatIndices;
	short (*quatKeys)[4];
	short (*_sQuatFrames)[4];
	bool bZOnly;
	float tolerance;
	float error;

	// Quantize before reducing the frames.
	// The file stores three signed short components of each quaternion.
	// Its fourth component is reconstructed by the loader.
	// Comparing at that precision prevents rounding from changing which
	// frames pass the error test after the animation has been written.
	// The angular threshold uses the same eigenvalue measure for both
	// the full quaternion and the two-component rotation writer.
	tolerance = AngleEigenTrace( model.rotationThreshold );
	_sQuatFrames = (short (*)[4])alloca( model.numframes * sizeof( short[4] ) );
	for ( i = 0; i < model.numframes; i++ ) {
		QuatToShort( _quatFrames[i], _sQuatFrames[i] );
		assert( _quatFrames[i][3] >= 0 );
		assert( _sQuatFrames[i][3] >= 0 );
	}

	// Keep consecutive rotations on the same hemisphere.
	for ( i = 1; i < model.numframes; i++ ) {
		sQuat = &_sQuatFrames[i];
		prevQuat = _sQuatFrames[i - 1];
		if ( (*sQuat)[0] * prevQuat[0] +
		     (*sQuat)[1] * prevQuat[1] +
		     (*sQuat)[2] * prevQuat[2] +
		     (*sQuat)[3] * prevQuat[3] < 0 ) {
			(*sQuat)[0] = -(*sQuat)[0];
			(*sQuat)[1] = -(*sQuat)[1];
			(*sQuat)[2] = -(*sQuat)[2];
			(*sQuat)[3] = -(*sQuat)[3];
			quat = _quatFrames[i];
			quat[0] = -quat[0];
			quat[1] = -quat[1];
			quat[2] = -quat[2];
			quat[3] = -quat[3];
		}
	}

	bByteIndices = model.numframes <= 256;
	if ( bByteIndices ) {
		quatIndices.b = (unsigned char *)alloca( model.numframes );
	} else {
		quatIndices.s = (unsigned short *)alloca( model.numframes * 2 );
	}
	quatKeys = (short (*)[4])alloca( model.numframes * sizeof( short[4] ) );

	numQuatIndices = 0;
	frame = 0;
	sQuat = &quatKeys[0];
	prevQuat = _sQuatFrames[0];
	(*sQuat)[0] = prevQuat[0];
	(*sQuat)[1] = prevQuat[1];
	(*sQuat)[2] = prevQuat[2];
	(*sQuat)[3] = prevQuat[3];
	if ( bByteIndices ) {
		quatIndices.b[0] = 0;
	} else {
		quatIndices.s[0] = 0;
	}

	// Find the longest segment whose interpolated frames stay within tolerance.
	while ( frame < model.numframes - 1 ) {
		for ( end = frame + 2; end < model.numframes; end++ ) {
			sQuat = &_sQuatFrames[end];
			prevQuat = _sQuatFrames[frame];
			if ( (*sQuat)[0] * prevQuat[0] +
			     (*sQuat)[1] * prevQuat[1] +
			     (*sQuat)[2] * prevQuat[2] +
			     (*sQuat)[3] * prevQuat[3] <= 0 ) {
				break;
			}
			for ( i = frame + 1; i < end; i++ ) {
				frac = (float)( i - frame ) / (float)( end - frame );
				lerpedQuat[0] = _quatFrames[frame][0] + ( _quatFrames[end][0] - _quatFrames[frame][0] ) * frac;
				lerpedQuat[1] = _quatFrames[frame][1] + ( _quatFrames[end][1] - _quatFrames[frame][1] ) * frac;
				lerpedQuat[2] = _quatFrames[frame][2] + ( _quatFrames[end][2] - _quatFrames[frame][2] ) * frac;
				lerpedQuat[3] = _quatFrames[frame][3] + ( _quatFrames[end][3] - _quatFrames[frame][3] ) * frac;
				VectorNormalize4D( lerpedQuat );
				error = QuatRatioEigenTrace( lerpedQuat, _quatFrames[i] );
				if ( error >= tolerance ) {
					goto nextKey;
				}
			}
		}

nextKey:
		numQuatIndices++;
		frame = end - 1;
		sQuat = &quatKeys[numQuatIndices];
		prevQuat = _sQuatFrames[frame];
		(*sQuat)[0] = prevQuat[0];
		(*sQuat)[1] = prevQuat[1];
		(*sQuat)[2] = prevQuat[2];
		(*sQuat)[3] = prevQuat[3];
		if ( bByteIndices ) {
			quatIndices.b[numQuatIndices] = (unsigned char)frame;
		} else {
			quatIndices.s[numQuatIndices] = (unsigned short)frame;
		}
	}

	if ( !bKeepIdentity ) {
		for ( i = 0; i <= numQuatIndices; i++ ) {
			sQuat = &quatKeys[i];
			if ( (*sQuat)[0] != 0 || (*sQuat)[1] != 0 || (*sQuat)[2] != 0 ) {
				break;
			}
		}
		if ( i > numQuatIndices ) {
			numQuatIndices = 0;
			SafeWrite( model.file, &numQuatIndices, 2 );
			return true;
		}
	}

	if ( numQuatIndices == 0 ) {
		numQuatIndices = 1;
		SafeWrite( model.file, &numQuatIndices, 2 );
		sQuat = &quatKeys[0];
		if ( (*sQuat)[0] || (*sQuat)[1] ) {
			WriteQuatXYZ( *sQuat );
			return false;
		}
		WriteQuatX( &(*sQuat)[2] );
		return true;
	}
	if ( numQuatIndices == 1 ) {
		sQuat = &quatKeys[0];
		prevQuat = quatKeys[1];
		if ( (*sQuat)[0] == prevQuat[0] && (*sQuat)[1] == prevQuat[1] &&
		     (*sQuat)[2] == prevQuat[2] && (*sQuat)[3] == prevQuat[3] ) {
			SafeWrite( model.file, &numQuatIndices, 2 );
			if ( (*sQuat)[0] || (*sQuat)[1] ) {
				WriteQuatXYZ( *sQuat );
				return false;
			}
			WriteQuatX( &(*sQuat)[2] );
			return true;
		}
	}

	// File layout of the rotation of one bone:
	//
	// short: number of keys, 0 for no rotation.
	// When there are fewer keys than frames, the frame numbers follow:
	// a byte per key for short animations, otherwise a short per key.
	//
	// The keys follow their frame numbers. If all the rotations are
	// around z, each key is a single short; otherwise it is short[3].
	// The w component is implied by the other three components.
	//
	// A single key never has a frame number. The caller records the
	// rotation encoding in a bit field alongside the bone names.
	//
	// The reduction count excludes the first key; include it in the file.
	// Index data is omitted when every frame is still a key.
	numQuatIndices++;
	SafeWrite( model.file, &numQuatIndices, 2 );
	assert( numQuatIndices <= model.numframes );
	if ( numQuatIndices < model.numframes ) {
		if ( bByteIndices ) SafeWrite( model.file, quatIndices.b, numQuatIndices );
		else SafeWrite( model.file, quatIndices.s, numQuatIndices * 2 );
	}
	bZOnly = true;
	for ( i = 0; i < numQuatIndices; i++ ) {
		sQuat = &quatKeys[i];
		if ( (*sQuat)[0] != 0 || (*sQuat)[1] != 0 ) bZOnly = false;
	}
	if ( !bZOnly ) {
		for ( i = 0; i < numQuatIndices; i++ ) WriteQuatXYZ( quatKeys[i] );
		return false;
	}
	for ( i = 0; i < numQuatIndices; i++ ) WriteQuatX( &quatKeys[i][2] );
	return true;
}

// Same reduction for the translation of a bone.  Keys are dropped as long as
// the interpolated position stays within translationThreshold of the real one.
// Unlike the rotation there is no hemisphere test: a position can simply be
// interpolated linearly.  The distance is compared squared, so the tolerance
// is squared once up front.
// File layout of the translation of one bone:
//	short	number of keys, 0 if the bone never leaves the origin
//	then, only if there are keys on less frames than the animation has:
//		byte or short	frame number of every key (byte if numframes <= 256)
//	then float[3] for every key
//
// Two keys that are closer than the tolerance are merged into their average
// and written as a single key.

/*
=============
SkelModel::SaveTrans
=============
*/
void SkelModel::SaveTrans( float (*trans)[3] ) {
	union { unsigned char *b; unsigned short *s; } transIndices;
	int i;
	float frac;
	int frame;
	int end;
	float lerpedTrans[3];
	bool bByteIndices;
	unsigned short numTransIndices;
	float (*transKeys)[3];
	float toleranceSq;

	toleranceSq = model.translationThreshold * model.translationThreshold;

	bByteIndices = model.numframes <= 256;
	if ( bByteIndices ) {
		transIndices.b = (unsigned char *)alloca( model.numframes );
	} else {
		transIndices.s = (unsigned short *)alloca( model.numframes * 2 );
	}
	transKeys = (float (*)[3])alloca( model.numframes * sizeof( float[3] ) );

	numTransIndices = 0;
	frame = 0;
	transKeys[0][0] = trans[0][0];
	transKeys[0][1] = trans[0][1];
	transKeys[0][2] = trans[0][2];
	if ( bByteIndices ) {
		transIndices.b[0] = 0;
	} else {
		transIndices.s[0] = 0;
	}

	while ( frame < model.numframes - 1 ) {
		for ( end = frame + 2; end < model.numframes; end++ ) {
			for ( i = frame + 1; i < end; i++ ) {
				frac = (float)( i - frame ) / (float)( end - frame );
				lerpedTrans[0] = trans[frame][0] + ( trans[end][0] - trans[frame][0] ) * frac;
				lerpedTrans[1] = trans[frame][1] + ( trans[end][1] - trans[frame][1] ) * frac;
				lerpedTrans[2] = trans[frame][2] + ( trans[end][2] - trans[frame][2] ) * frac;
				if ( VectorDistanceSquared( lerpedTrans, trans[i] ) >= toleranceSq ) {
					goto nextKey;
				}
			}
		}
		// end either failed interpolation or reached the frame count.
		// Keep the preceding frame as a key and start a new segment there.
		// Its frame number is stored separately for runtime lookup.
nextKey:

		numTransIndices++;
		frame = end - 1;
		VectorCopy( trans[frame], transKeys[numTransIndices] );
		if ( bByteIndices ) {
			transIndices.b[numTransIndices] = (unsigned char)frame;
		} else {
			transIndices.s[numTransIndices] = (unsigned short)frame;
		}
	}
	for ( i = 0; i <= numTransIndices; i++ ) {
		if ( transKeys[i][0] * transKeys[i][0] + transKeys[i][1] * transKeys[i][1] + transKeys[i][2] * transKeys[i][2] >= toleranceSq ) {
			break;
		}
	}
	if ( i > numTransIndices ) {
		// the bone never moves
		numTransIndices = 0;
		SafeWrite( model.file, &numTransIndices, 2 );
		return;
	}
	if ( numTransIndices == 0 ) {
		numTransIndices = 1;
		SafeWrite( model.file, &numTransIndices, 2 );
		WriteVector( transKeys[0] );
		return;
	}
	if ( numTransIndices == 1 && VectorDistanceSquared( transKeys[0], transKeys[1] ) < toleranceSq ) {
		SafeWrite( model.file, &numTransIndices, 2 );
		transKeys[0][0] = transKeys[0][0] + transKeys[1][0];
		transKeys[0][1] = transKeys[0][1] + transKeys[1][1];
		transKeys[0][2] = transKeys[0][2] + transKeys[1][2];
		VectorScale( transKeys[0], 0.5f, transKeys[0] );
		WriteVector( transKeys[0] );
		return;
	}
	// more than one key: count, the frame numbers of the keys unless every
	// frame is a key, then the positions
	//
	// the count is an unsigned short, so numframes itself is limited to 65535
	// The initial key has no separate index in the reduced list.
	// Include the first key in the stored count.
	numTransIndices++;

	SafeWrite( model.file, &numTransIndices, 2 );
	assert( numTransIndices <= model.numframes );
	if ( numTransIndices < model.numframes ) {
		if ( bByteIndices ) SafeWrite( model.file, transIndices.b, numTransIndices );
		else SafeWrite( model.file, transIndices.s, numTransIndices * 2 );
	}
	for ( i = 0; i < numTransIndices; i++ ) {
		WriteVector( transKeys[i] );
	}
}
// Store a z-only rotation as quantized (z, w) pairs.
// Reduce keys using the quaternion interpolation error.
// Write a short key count, optional byte/short frame indices, then z values.
// The sign of w is folded into z; zero keys denote no rotation.

/*
=============
SkelModel::WriteQuat
=============
*/
void SkelModel::WriteQuat( float _quatFrames[][2] ) {
	union { unsigned char *b; unsigned short *s; } quatIndices;
	int i;
	float frac;
	int frame;
	int end;
	float lerpedQuat[2];
	float *quatFrame;
	short *sQuat;
	short *prevQuat;
	bool bByteIndices;
	unsigned short numQuatIndices;
	short (*quatKeys)[2];
	short (*_sQuatFrames)[2];
	float lerpedQuat2[4];
	float quat[4];
	float tolerance;
	float error;

	// Compare the quantized keys against the angular tolerance.
	// Only z and w are stored for these rotations.
	// Quantization uses the same range as a full quaternion, so the
	// reduced keys can share the runtime's rotation decoding code.
	// Interpolation error is measured after widening the pair to
	// a normalized quaternion with zero x and y components.
	tolerance = AngleEigenTrace( model.rotationThreshold );
	_sQuatFrames = (short (*)[2])alloca( model.numframes * sizeof( short[2] ) );
	for ( i = 0; i < model.numframes; i++ ) {
		QuatToShort2( _quatFrames[i], _sQuatFrames[i] );
	}
	assert( _quatFrames[0][1] >= 0 );
	assert( _sQuatFrames[0][1] >= 0 );

	for ( i = 1; i < model.numframes; i++ ) {
		sQuat = _sQuatFrames[i];
		prevQuat = _sQuatFrames[i - 1];
		if ( sQuat[0] * prevQuat[0] + sQuat[1] * prevQuat[1] < 0 ) {
			sQuat[0] = -sQuat[0];
			sQuat[1] = -sQuat[1];
			quatFrame = _quatFrames[i];
			quatFrame[0] = -quatFrame[0];
			quatFrame[1] = -quatFrame[1];
		}
	}

	// Find the farthest next key whose intervening frames can be interpolated.
	//
	// The first frame is always a key. The count below excludes that frame.
	// quatKeys holds the quantized pairs; quatIndices holds frame numbers.
	// Short animations use byte indices and longer animations use shorts.
	//
	// Endpoints must have a positive dot product. A perpendicular pair
	// would interpolate through the zero vector and cannot be normalized.
	//
	// Widen each interpolated pair to (0, 0, z, w) before measuring error.
	// The real frame is widened in the same way, so the comparison can
	// use the full-quaternion eigenvalue error measure.
	// Keys and indices live on the stack for the duration of this writer.
	bByteIndices = model.numframes <= 256;
	if ( bByteIndices ) quatIndices.b = (unsigned char *)alloca( model.numframes );
	else quatIndices.s = (unsigned short *)alloca( model.numframes * 2 );
	quatKeys = (short (*)[2])alloca( model.numframes * sizeof( short[2] ) );

	numQuatIndices = 0;
	frame = 0;
	sQuat = quatKeys[0];
	prevQuat = _sQuatFrames[0];
	sQuat[0] = prevQuat[0];
	sQuat[1] = prevQuat[1];
	if ( bByteIndices ) {
		quatIndices.b[0] = 0;
	} else {
		quatIndices.s[0] = 0;
	}

	// The first frame is already a key; reduce only if another frame exists.
	while ( frame < model.numframes - 1 ) {
			for ( end = frame + 2; end < model.numframes; end++ ) {
				sQuat = _sQuatFrames[end];
				prevQuat = _sQuatFrames[frame];
				if ( sQuat[0] * prevQuat[0] + sQuat[1] * prevQuat[1] <= 0 ) {
					break;
				}
				for ( i = frame + 1; i < end; i++ ) {
					frac = (float)( i - frame ) / (float)( end - frame );
					lerpedQuat[0] = _quatFrames[frame][0] + ( _quatFrames[end][0] - _quatFrames[frame][0] ) * frac;
					lerpedQuat[1] = _quatFrames[frame][1] + ( _quatFrames[end][1] - _quatFrames[frame][1] ) * frac;
					VectorNormalize2D( lerpedQuat );
					lerpedQuat2[0] = 0;
					lerpedQuat2[1] = 0;
					lerpedQuat2[2] = lerpedQuat[0];
					lerpedQuat2[3] = lerpedQuat[1];
					quat[0] = 0;
					quat[1] = 0;
					quat[2] = _quatFrames[i][0];
					quat[3] = _quatFrames[i][1];
					error = QuatRatioEigenTrace( lerpedQuat2, quat );
					if ( error >= tolerance ) {
						goto nextKey;
					}
				}
			}
	nextKey:
			numQuatIndices++;
			frame = end - 1;
			sQuat = quatKeys[numQuatIndices];
			prevQuat = _sQuatFrames[frame];
			sQuat[0] = prevQuat[0];
			sQuat[1] = prevQuat[1];
			if ( bByteIndices ) {
				quatIndices.b[numQuatIndices] = (unsigned char)frame;
			} else {
				quatIndices.s[numQuatIndices] = (unsigned short)frame;
			}
	}

	for ( i = 0; i <= numQuatIndices; i++ ) {
		sQuat = quatKeys[i];
		if ( sQuat[0] != 0 ) {
			break;
		}
	}
	if ( i > numQuatIndices ) {
		numQuatIndices = 0;
		SafeWrite( model.file, &numQuatIndices, 2 );
		return;
	}

	if ( numQuatIndices == 0 ) {
		numQuatIndices = 1;
		SafeWrite( model.file, &numQuatIndices, 2 );
		sQuat = quatKeys[0];
		WriteQuatX( sQuat );
		return;
	}

	if ( numQuatIndices == 1 ) {
		sQuat = quatKeys[0];
		prevQuat = quatKeys[1];
		if ( sQuat[0] == prevQuat[0] && sQuat[1] == prevQuat[1] ) {
		SafeWrite( model.file, &numQuatIndices, 2 );
			WriteQuatX( sQuat );
			return;
		}
	}

	// Every key is written as a single short.  A rotation around z is
	// (0, 0, sin(a/2), cos(a/2)); with w dropped and its sign folded
	// into z, one short per key is all the loader needs.
	//
	// The count includes the first key, which is not in numQuatIndices yet.
	// (A bone with exactly one key is handled above and never gets here.)
	numQuatIndices++;
	SafeWrite( model.file, &numQuatIndices, 2 );
	assert( numQuatIndices <= model.numframes );
	if ( numQuatIndices < model.numframes ) {
		if ( bByteIndices ) SafeWrite( model.file, quatIndices.b, numQuatIndices );
		else SafeWrite( model.file, quatIndices.s, numQuatIndices * 2 );
	}
	for ( i = 0; i < numQuatIndices; i++ ) {
		WriteQuatX( quatKeys[i] );
	}
}

// Write version14: frame/bone counts, loop/delta flags and frame rate.
// Delta animations carry root motion; relative animations skip the root.
// Store bone flags/names, reduced rotation/translation keys and notetracks.
// Omit the repeated last frame when looping; baseFrames is the bind pose.
/*
=============
SkelModel::SaveAnimation
=============
*/
void SkelModel::SaveAnimation( const char *filename, int type, loadbone_t *baseFrames, QuatBoneEntry *rotations ) {
	int i;
	int j;
	loadbone_t *bfCurrent;
	loadbone_t *bfParent;
	short sNumframes;
	int numframes;
	short sNumbones;
	Bone *boneName;
	int parentIndex;
	float tranInv[3];
	float parentMatrixInv[4][3];
	float mat[3][3];
	float Q[4];
	float (*quatFrames)[4];
	float (*zQuatFrames)[2];
	float (*transFrames)[3];
	float modelTrans[3];
	short framerate;
	char *rotationBits;
	int numBitBytes;
	short version;
	int numAnimBones;
	unsigned char flags;
	int numRoots;
	int firstBone;
	bool bDelta;
	bool bSkipRoot;
	int rootIndex;
	int bitIndex;
	short quat[4];
	char *flagBits;
	int filePos;
	int endPos;
	bool bFlag;
	const QuatBoneEntry *rotation;
	char msg[2048];

	bDelta = ( type == 2 );
	bSkipRoot = ( type == 2 || type == 1 );

	// The two header bits describe looping and root delta data.
	// A relative animation omits the root but writes no delta channel.
	// Absolute animations retain every selected bone.
	// Node-relative movement is meaningful only for a delta animation.
	// The looping flag also determines whether the final repeated frame
	// contributes to the frame count stored in the header.
	flags = 0;
	if ( model.looping ) {
		flags |= 1;
	}
	if ( bDelta ) {
		flags |= 2;
	}
	if ( !bDelta && model.nodePart != -1 ) {
		Error( "cannot specify node if type is not delta." );
	}

	Swap_Init();

	// Looping exports repeat the first frame at the end.
	// The runtime wraps back to frame zero, so that duplicate is omitted.
	// A one-frame animation still needs its single frame.
	// Keep the export's full frame count for interpolation below.
	numframes = model.looping && model.numframes != 1 ? model.numframes - 1 : model.numframes;
	if ( numframes == 0 ) {
		Error( "xanim '%s' has 0 frames", filename );
	}

	// Locate the root used by relative and delta animations.
	// Stabilizers attached to a root participate in this selection.
	// Delta movement requires exactly one selected root.
	rootIndex = -1;
	if ( bSkipRoot ) {
		numRoots = 0;
		for ( i = 0; i < model.numbones; i++ ) {
			if ( !model.bones[i].used ) {
				continue;
			}
			if ( model.bones[i].parent < 0 ) {
					rootIndex = i;
					numRoots++;
				} else if ( model.bones[i].stabilizer && model.bones[model.bones[i].parent].parent < 0 ) {
					rootIndex = i;
					numRoots++;
				}
		}
		if ( bDelta && numRoots != 1 ) {
			sprintf( msg, "Not exactly one root bone for xanim '%s' with delta:\n", filename );
			for ( i = 0; i < model.numbones; i++ ) {
				if ( !model.bones[i].used ) {
					continue;
				}
				if ( model.bones[i].parent < 0 ) {
					strcat( msg, model.bones[i].name );
					strcat( msg, "\n" );
				}
			}
			Error( msg );
		}
	}
	firstBone = bSkipRoot ? rootIndex + 1 : 0;
	numAnimBones = 0;
	for ( j = firstBone; j < model.numbones; j++ ) {
		if ( !model.bones[j].used ) {
			continue;
		}
		numAnimBones++;
	}
	// Header fields use the little-endian representation of the runtime.
	// Check the signed-short limits before narrowing their counts.
	model.file = SafeOpenWrite( filename );
	version = LittleShort( 14 );
	SafeWrite( model.file, &version, 2 );
	assert( numframes == (short) numframes );
	assert( model.numframes == (short) model.numframes );
	sNumframes = LittleShort( numframes );
	SafeWrite( model.file, &sNumframes, 2 );

	assert( numAnimBones == (short) numAnimBones );
	sNumbones = LittleShort( numAnimBones );
	SafeWrite( model.file, &sNumbones, 2 );

	SafeWrite( model.file, &flags, 1 );

	assert( model.framerate == (short) model.framerate );
	framerate = LittleShort( (short) model.framerate );
	SafeWrite( model.file, &framerate, 2 );

	// Allocate per-frame bone data, rebuilt for every bone.
	quatFrames = (float (*)[4])alloca( model.numframes * sizeof( float[4] ) );
	transFrames = (float (*)[3])alloca( model.numframes * sizeof( float[3] ) );
	zQuatFrames = (float (*)[2])alloca( model.numframes * sizeof( float[2] ) );

	if ( bDelta ) {
		assert( bSkipRoot );
		assert( rootIndex >= 0 );

		// the root bone has to be animated
		// (a bone nobody references would not exist in the animation)
		boneName = &model.bones[rootIndex];
		if ( !boneName->used ) {
			fclose( model.file );
			Error( "Part '%s' was not animated.", boneName->name );
		}
		assert( boneName->parent == -1 );

		if ( model.nodePart == -1 ) {
			bfParent = &model.frames[0].bones[rootIndex];
			MatrixTranspose( bfParent->axis, parentMatrixInv );
			VectorScale( bfParent->offset,
			             -1,
			             tranInv );
			MatrixTransformVector( tranInv, parentMatrixInv, parentMatrixInv[3] );
		}

		// The root bone is reduced to a rotation around z and a translation,
		// both relative to the node: the inverse of the node transform is
		// applied to the root of every frame.  Without a node, the pose of
		// the root in the first frame is the origin of the movement.
		//
		// parentMatrixInv is that inverse transform: the transposed rotation of the
		// node and, in the fourth row, its translation.
		for ( i = 0; i < model.numframes; i++ ) {
			if ( model.nodePart != -1 ) {
				bfParent = &model.nodeFrames[i];
				MatrixTranspose( bfParent->axis, parentMatrixInv );
				VectorScale( bfParent->offset,
			             -1,
			             tranInv );
				MatrixTransformVector( tranInv, parentMatrixInv, parentMatrixInv[3] );
			}
			bfCurrent = &model.frames[i].bones[rootIndex];
			MatrixMultiply( bfCurrent->axis, parentMatrixInv, mat );
			MatrixTransformVector43( bfCurrent->offset, parentMatrixInv, transFrames[i] );
			ConvertMatToQuat( mat, Q );
			if ( Q[0] * Q[0] + Q[1] * Q[1] >= ( Q[2] * Q[2] + Q[3] * Q[3] ) * 0.001f ) {
				fclose( model.file );
				Error( "xanim '%s' has bad angle delta on frame %d", filename, model.frames[i].number );
			}
			zQuatFrames[i][0] = Q[2];
			zQuatFrames[i][1] = Q[3];
		}

			assert( model.numframes > 0 );
		if ( model.looping ) {
			Vector2Copy( zQuatFrames[0],
			             zQuatFrames[model.numframes - 1] );
		}
		WriteQuat( zQuatFrames );
		SaveTrans( transFrames );
	}

	if ( numAnimBones != 0 ) {
		// Two bit fields precede the bone data.  The bits are not known before
		// the bones were written, so the fields are written as zeroes first
		// and filled in at the end: the position is remembered and the
		// file is rewritten from there.  The names of all bones come next
		// so that the loader can resolve them before it reads any frame.
		//
		// The bytes are cleared by memset, as the stack has no defined content.
		numBitBytes = ( ( numAnimBones - 1 ) >> 3 ) + 1;
		rotationBits = (char *)alloca( numBitBytes );
		memset( rotationBits, 0, numBitBytes );
		flagBits = (char *)alloca( numBitBytes );
		memset( flagBits, 0, numBitBytes );

		filePos = ftell( model.file );
		SafeWrite( model.file, rotationBits, numBitBytes );
		SafeWrite( model.file, flagBits, numBitBytes );

		for ( j = firstBone; j < model.numbones; j++ ) {
			boneName = &model.bones[j];
			if ( !boneName->used ) continue;
			SafeWrite( model.file, boneName->name, strlen( boneName->name ) + 1 );
		}

		bitIndex = 0;
		for ( j = firstBone; j < model.numbones; j++ ) {
			boneName = &model.bones[j];
			if ( !boneName->used ) {
				continue;
			}
			// Walk past bones that are absent from the exported animation.
			// The first selected ancestor supplies the parent transform.
			// If none remains, use this bone's base pose as its parent.
			// Bone order is checked when the frame transforms are generated.
			parentIndex = boneName->parent;
			while ( parentIndex >= 0 ) {
				if ( model.bones[parentIndex].used ) break;
				parentIndex = model.bones[parentIndex].parent;
			}

			// Model-match entries can override the base rotation of a bone.
			// The last entry terminates a group with its more flag.
			// Retain the matching entry for the test below; a null cursor
			// means that the base pose must supply the reference rotation.
			// Names are compared without case, like the export's bone lookup.
			for ( rotation = rotations; rotation; rotation = rotation->more ? rotation + 1 : 0 ) {
				if ( stricmp( boneName->name, rotation->name.c_str() ) ) continue;
				Q[0] = rotation->quat[0];
				Q[1] = rotation->quat[1];
				Q[2] = rotation->quat[2];
				Q[3] = rotation->quat[3];
				break;
			}
			if ( !rotation ) {
				bfCurrent = &baseFrames[boneName->fileIndex];
				bfParent = &baseFrames[parentIndex >= 0 ? model.bones[parentIndex].fileIndex : boneName->fileIndex];
				MatrixTranspose( bfParent->axis, parentMatrixInv );
				MatrixMultiply( bfCurrent->axis, parentMatrixInv, mat );
				ConvertMatToQuat( mat, Q );
			}

			for ( i = 0; i < model.numframes; i++ ) {
				bfCurrent = &model.frames[i].bones[j];
				if ( parentIndex >= 0 ) {
					// a parent always comes before its children
					assert( parentIndex < j );
					bfParent = &model.frames[i].bones[parentIndex];
				} else {
					bfParent = &baseFrames[boneName->fileIndex];
				}
				MatrixTranspose( bfParent->axis, parentMatrixInv );
				VectorScale( bfParent->offset, -1, tranInv );
				MatrixTransformVector( tranInv, parentMatrixInv, parentMatrixInv[3] );
				MatrixTransformVector43( bfCurrent->offset, parentMatrixInv, transFrames[i] );
				MatrixMultiply( bfCurrent->axis, parentMatrixInv, mat );
				ConvertMatToQuat( mat, quatFrames[i] );
			}


			// the last frame of a looping animation is a copy of the first one

			assert( model.numframes > 0 );
			if ( model.looping ) {
				Vector4Copy(
					quatFrames[0],
					quatFrames[model.numframes - 1]
				);
			}
			QuatToShort( quatFrames[0], quat );
			// Preserve the first key's sign relative to the bind pose.
			// WriteQuatFrames uses it when dropping the fourth component.
			bFlag = quat[0] * Q[0] + quat[1] * Q[1] + quat[2] * Q[2] + quat[3] * Q[3] < 0;
			if ( bFlag ) rotationBits[bitIndex >> 3] |= 1 << ( bitIndex & 7 );
			if ( model.useBones && parentIndex >= 0 ) {
				bfCurrent = &baseFrames[boneName->fileIndex];
				bfParent = &baseFrames[model.bones[parentIndex].fileIndex];
				MatrixTranspose( bfParent->axis, parentMatrixInv );
				VectorScale( bfParent->offset, -1, tranInv );
				MatrixTransformVector( tranInv, parentMatrixInv, parentMatrixInv[3] );
				MatrixTransformVector43( bfCurrent->offset, parentMatrixInv, modelTrans );
				for ( i = 0; i < model.numframes; i++ ) {
					// Store movement relative to the base-pose translation.
					// The parent transform has already put both in bone space.
					// Apply the same displacement to every exported frame.
					// Loop closure is restored after this subtraction.
					VectorSubtract( transFrames[i], modelTrans, transFrames[i] );
				}
			}
			assert( model.numframes > 0 );
			if ( model.looping ) {
				VectorCopy( transFrames[0], transFrames[model.numframes - 1] );
			}

			if ( SaveQuats( quatFrames, bFlag ) ) {
				flagBits[bitIndex >> 3] |= 1 << ( bitIndex & 7 );
			}
			SaveTrans( transFrames );
			bitIndex++;
		}

		endPos = ftell( model.file );
		fseek( model.file, filePos, SEEK_SET );
		SafeWrite( model.file, rotationBits, numBitBytes );
		SafeWrite( model.file, flagBits, numBitBytes );
		fseek( model.file, endPos, SEEK_SET );
	}

	SaveNoteTracks();
	fclose( model.file );
}

/*
=============
SkelModel::WriteBlend
=============
*/
void SkelModel::WriteBlend( blendvert_t *blend, bool noWeight, vec3_t offset ) {
	short boneIndex;


	assert( blend->bone == (short) blend->bone );
	boneIndex = LittleShort((short)blend->bone);
	SafeWrite(model.file, &boneIndex, sizeof(boneIndex));
	WriteVector(offset);
	if (noWeight) return;
	float boneWeight = LittleFloat(blend->weight);
	SafeWrite(model.file, &boneWeight, sizeof(boneWeight));
}

