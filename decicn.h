/*
  decicn.h
  2 March 2009
  Extracts images (mono, colour, mask) from cicn resources.

  Copyright (C) 2008-2009 Neil Gentleman

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgement in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#ifndef H_DECICN
#define H_DECICN

typedef struct ColorSpec {
	uint16_t value;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} ColorSpec;

/* http://developer.apple.com/documentation/mac/QuickDraw/QuickDraw-26.html */
typedef struct BitMap {
	uint8_t* baseAddr;
	uint16_t rowBytes;
	uint16_t bounds[4];
} BitMap;

typedef struct ColorTable {
	uint32_t ctSeed;
	uint16_t ctFlags;
	uint16_t ctSize;
	struct ColorSpec* ctTable;
} ColorTable;

/* http://developer.apple.com/documentation/mac/QuickDraw/QuickDraw-202.html */
typedef struct PixMap {
	uint8_t* baseAddr;
	uint16_t rowBytes;
	uint16_t bounds[4];
	uint16_t pmVersion;
	uint16_t packType;
	uint32_t packSize;
	uint32_t hRes;
	uint32_t vRes;
	uint16_t pixelType;
	uint16_t pixelSize;
	uint16_t cmpCount;
	uint16_t cmpSize;
	uint32_t planeBytes;
	struct ColorTable pmTable;
	uint32_t pmReserved;
} PixMap;

typedef struct CIcon {
	struct PixMap* PMap;
	struct BitMap* Mask;
	struct BitMap* BMap;
} CIcon;

enum Bounds {
	top = 0, left, bottom, right
};

uint16_t getint(FILE* file);
uint32_t getlong(FILE* file);

void read_PixMap(PixMap* pmap, FILE* f);
void read_BitMap(BitMap* bmap, FILE* f);
void read_CIcon(CIcon* cicn, FILE* f);

void dump_CIcon(const CIcon* cicn, FILE* f);
void dump_PixMap(const PixMap* pmap, FILE* stream);
void dump_BitMap(const BitMap* bmap, FILE* stream);

void print_PixMap(const PixMap* pmap, FILE* stream);
void print_BitMap(const BitMap* bmap, FILE* stream);

#endif
