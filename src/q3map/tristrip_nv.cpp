/*
===========================================================================
Call of Duty converter plugins (2003) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#include <stdio.h>
#include <stdlib.h>

#include "../libs/nvtristrip/NvTriStrip.h"

#include "tristrip.h"

/*
=============
GenerateTriangleGroups
=============
*/
unsigned long GenerateTriangleGroups( const unsigned short indices[],
	unsigned short numIndices, triangleIndexGroup_t *&groups,
	unsigned short remap[], unsigned short numVerts ) {
	unsigned short numGroups;
	int i;
	unsigned int j;
	unsigned int index;

	index = 0;
	numGroups = numIndices / 3;
	groups = (triangleIndexGroup_t *)malloc( numGroups * sizeof( *groups ) );
	for ( i = 0; i < numGroups; i++ ) {
		groups[i].numIndices = 3;
		groups[i].indices = (unsigned short *)malloc( groups[i].numIndices * sizeof( *indices ) );
		for ( j = 0; j < 3; j++, index++ ) {
			groups[i].indices[j] = indices[index];
		}
	}
	for ( i = 0; i < numVerts; i++ ) {
		remap[i] = i;
	}
	return numGroups;
}

/*
=============
MakeStripGroups
=============
*/
int MakeStripGroups( const unsigned short *const indices,
	unsigned short numIndices, triangleIndexGroup_t **const groups,
	unsigned short *remap, int useVertexCache ) {
	PrimitiveGroup *pPrimGroups, *pRemappedGroups;
	unsigned short uwGroupCount, numVerts;
	int i;
	unsigned int j;

	if ( useVertexCache ) {
		SetCacheSize( 16 );
	} else {
		SetCacheSize( 4096 );
	}
	SetStitchStrips( false );
	SetMinStripSize( 0 );
	SetListsOnly( false );
	GenerateStrips( indices, numIndices, &pPrimGroups, &uwGroupCount );
	numVerts = 0;
	for ( i = 0; i < uwGroupCount; i++ ) {
		for ( j = 0; j < pPrimGroups[i].numIndices; j++ ) {
			if ( numVerts <= pPrimGroups[i].indices[j] ) {
				numVerts = pPrimGroups[i].indices[j] + 1;
			}
		}
	}
	RemapIndices( pPrimGroups, uwGroupCount, numVerts, &pRemappedGroups );
	*groups = (triangleIndexGroup_t *)malloc( uwGroupCount * sizeof( **groups ) );
	for ( i = 0; i < uwGroupCount; i++ ) {
		(*groups)[i].numIndices = pRemappedGroups[i].numIndices;
		(*groups)[i].indices = (unsigned short *)malloc( (*groups)[i].numIndices * sizeof( *indices ) );
		for ( j = 0; j < pRemappedGroups[i].numIndices; j++ ) {
			(*groups)[i].indices[j] = pRemappedGroups[i].indices[j];
			remap[pPrimGroups[i].indices[j]] = pRemappedGroups[i].indices[j];
		}
	}
	delete[] pPrimGroups;
	delete[] pRemappedGroups;
	return uwGroupCount;
}

/*
=============
CreateOptimizedTriangleIndices
=============
*/
void CreateOptimizedTriangleIndices( unsigned short *const indices, unsigned short numIndices,
	unsigned short *const remap ) {
	PrimitiveGroup *pPrimGroups, *pRemappedGroups;
	unsigned short uwGroupCount, numVerts;
	unsigned int i;

	SetCacheSize( 16 );
	SetStitchStrips( false );
	SetMinStripSize( 0 );
	SetListsOnly( true );
	GenerateStrips( indices, numIndices, &pPrimGroups, &uwGroupCount );
	if ( uwGroupCount != 1 ) {
		printf( "WARNING: GenerateStrips returned multiple lists\n" );
		return;
	}

	numVerts = 0;
	for ( i = 0; i < pPrimGroups->numIndices; i++ ) {
		if ( numVerts <= pPrimGroups->indices[i] ) {
			numVerts = pPrimGroups->indices[i] + 1;
		}
	}
	RemapIndices( pPrimGroups, 1, numVerts, &pRemappedGroups );
	for ( i = 0; i < pRemappedGroups->numIndices; i++ ) {
		indices[i] = pRemappedGroups->indices[i];
		remap[pPrimGroups->indices[i]] = pRemappedGroups->indices[i];
	}
}
