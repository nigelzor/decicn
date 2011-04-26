/*
  decicn.c
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "decicn.h"

static void die(char* message) {
	fputs(message, stderr);
	fputs("\n", stderr);
	exit(1);
}

static uint16_t getint(FILE* file) {
	int x, y;
	assert((x = fgetc(file)) != EOF);
	assert((y = fgetc(file)) != EOF);
	return (x<<8) + y;
}

static uint32_t getlong(FILE* file) {
	int x, y;
	x = getint(file);
	y = getint(file);
	return (x<<16) + y;
}

void read_CIcon(CIcon* cicn, FILE* f) {
	int len;

	cicn->PMap = calloc(1, sizeof(PixMap));
	if (NULL == cicn->PMap) die("calloc failed");

	cicn->Mask = calloc(1, sizeof(BitMap));
	if (NULL == cicn->Mask) die("calloc failed");

	cicn->BMap = calloc(1, sizeof(BitMap));
	if (NULL == cicn->BMap) die("calloc failed");

	read_PixMap(cicn->PMap, f);
	read_BitMap(cicn->Mask, f);
	read_BitMap(cicn->BMap, f);
	assert(0 == getlong(f));

	len = cicn->Mask->rowBytes * (cicn->Mask->bounds[bottom] - cicn->Mask->bounds[top]);
	cicn->Mask->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->Mask->baseAddr) die("calloc failed");
	fread(cicn->Mask->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));

	len = cicn->BMap->rowBytes * (cicn->BMap->bounds[bottom] - cicn->BMap->bounds[top]);
	cicn->BMap->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->BMap->baseAddr) die("calloc failed");
	fread(cicn->BMap->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));

	cicn->PMap->pmTable.ctSeed = getlong(f);
	cicn->PMap->pmTable.ctFlags = getint(f);
	cicn->PMap->pmTable.ctSize = getint(f);
	cicn->PMap->pmTable.ctTable = calloc(cicn->PMap->pmTable.ctSize + 1, sizeof(ColorSpec));
	if (NULL == cicn->PMap->pmTable.ctTable) die("calloc failed");
	for (len = 0; len <= cicn->PMap->pmTable.ctSize; len++) {
		cicn->PMap->pmTable.ctTable[len].value = getint(f);
		cicn->PMap->pmTable.ctTable[len].red = getint(f);
		cicn->PMap->pmTable.ctTable[len].green = getint(f);
		cicn->PMap->pmTable.ctTable[len].blue = getint(f);
	}

	len = (cicn->PMap->rowBytes & 0x7FFF) * (cicn->PMap->bounds[bottom] - cicn->PMap->bounds[top]);
	cicn->PMap->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->PMap->baseAddr) die("calloc failed");
	fread(cicn->PMap->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));
}

void read_PixMap(PixMap* pmap, FILE* f) {
	pmap->baseAddr = NULL;
	assert(0 == getlong(f));
	pmap->rowBytes = getint(f);
	assert(0x8000 == (pmap->rowBytes & 0xC000));
	pmap->bounds[top] = getint(f);
	pmap->bounds[left] = getint(f);
	pmap->bounds[bottom] = getint(f);
	pmap->bounds[right] = getint(f);
	pmap->pmVersion = getint(f);
	assert(0 == pmap->pmVersion);
	pmap->packType = getint(f);
	assert(0 == pmap->packType);
	pmap->packSize = getlong(f);
	assert(0 == pmap->packSize);
	pmap->hRes = getlong(f);
	assert(0x480000 == pmap->hRes);
	pmap->vRes = getlong(f);
	assert(0x480000 == pmap->vRes);
	pmap->pixelType = getint(f);
	assert(0 == pmap->pixelType);
	pmap->pixelSize = getint(f);
	pmap->cmpCount = getint(f);
	assert(1 == pmap->cmpCount);
	pmap->cmpSize = getint(f);
	pmap->planeBytes = getlong(f);
	assert(0 == pmap->planeBytes);
	// pmap->pmTable = NULL;
	assert(0 == getlong(f));
	pmap->pmReserved = getlong(f);
	assert(0 == pmap->pmReserved);
}

void read_BitMap(BitMap* bmap, FILE* f) {
	bmap->baseAddr = NULL;
	assert(0 == getlong(f));
	bmap->rowBytes = getint(f);
	bmap->bounds[top] = getint(f);
	bmap->bounds[left] = getint(f);
	bmap->bounds[bottom] = getint(f);
	bmap->bounds[right] = getint(f);
}

void dump_CIcon(const CIcon* cicn, FILE* stream) {
	fprintf(stream, "pmap : ");
	dump_PixMap(cicn->PMap, stream);
	fprintf(stream, "bmap : ");
	dump_BitMap(cicn->BMap, stream);
	fprintf(stream, "mask : ");
	dump_BitMap(cicn->Mask, stream);
}

void dump_PixMap(const PixMap* pmap, FILE* stream) {
	fprintf(stream, "PixMap {\n");
	fprintf(stream, "\trowBytes: %d\n", (pmap->rowBytes & 0x3FFF));
	fprintf(stream, "\tbounds: top: %d, left: %d, bottom: %d, right: %d\n", pmap->bounds[top], pmap->bounds[left], pmap->bounds[bottom], pmap->bounds[right]);
	fprintf(stream, "\tpixelSize: %d\n", pmap->pixelSize);
	fprintf(stream, "\tcmpSize: %d\n", pmap->cmpSize);
	fprintf(stream, "\t(data length %d)\n", (pmap->rowBytes & 0x7FFF) * (pmap->bounds[bottom] - pmap->bounds[top]));
	fprintf(stream, "}\n");
}

void dump_BitMap(const BitMap* bmap, FILE* stream) {
	fprintf(stream, "BitMap {\n");
	fprintf(stream, "\trowBytes: %d\n", bmap->rowBytes);
	fprintf(stream, "\tbounds: top: %d, left: %d, bottom: %d, right: %d\n", bmap->bounds[top], bmap->bounds[left], bmap->bounds[bottom], bmap->bounds[right]);
	fprintf(stream, "\t(data length %d)\n", bmap->rowBytes * (bmap->bounds[bottom] - bmap->bounds[top]));
	fprintf(stream, "}\n");
}

inline void print_P3_ColorSpec(const ColorSpec* cs, FILE* fp) {
	fprintf(fp, "%d %d %d\n", cs->red, cs->green, cs->blue);
}

void print_PixMap(const PixMap* pmap, FILE* stream) {
	unsigned int width, height, x, y, idx, color;
	width = pmap->bounds[right] - pmap->bounds[left];
	height = pmap->bounds[bottom] - pmap->bounds[top];

	fputs("P3\n# converted from cicn\n", stream);
	fprintf(stream, "%d %d\n", width, height);
	fputs("65535\n", stream);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			idx = y * (pmap->rowBytes & 0x3FFF) * 8 / pmap->pixelSize + x;
			switch (pmap->pixelSize) {
			case 1:
				color = pmap->baseAddr[idx / 8];
				color &= 0x80 >> (idx % 8);
				color >>= 7 - (idx % 8);
				break;
			case 2:
				color = pmap->baseAddr[idx / 4];
				switch (idx % 4) {
				case 0:
					color >>= 2;
				case 1:
					color >>= 2;
				case 2:
					color >>= 2;
				case 3:
					color &= 0x03;
				}
				break;
			case 4:
				color = pmap->baseAddr[idx / 2];
				if (idx % 2 == 1) {
					color &= 0x0F;
				} else {
					color = (color & 0xF0) >> 4;
				}
				break;
			case 8:
				color = pmap->baseAddr[idx];
				break;
			default:
				die("unhandled pixelSize");
			}
			for (idx = 0; idx <= pmap->pmTable.ctSize; idx++) {
				if (pmap->pmTable.ctTable[idx].value == color) {
					print_P3_ColorSpec(pmap->pmTable.ctTable + idx, stream);
					break;
				}
			}
			if (idx > pmap->pmTable.ctSize) {
				fprintf(stderr, "no matching colour found");
			}
		}
	}
}

void print_BitMap(const BitMap* bmap, FILE* stream) {
	unsigned int width, height, x, y, idx, color;
	width = bmap->bounds[right] - bmap->bounds[left];
	height = bmap->bounds[bottom] - bmap->bounds[top];

	fputs("P1\n# converted from cicn\n", stream);
	fprintf(stream, "%d %d\n", width, height);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			idx = y * bmap->rowBytes * 8 + x;
			color = bmap->baseAddr[idx / 8];
			color &= 0x80 >> (idx & 7);
			fputs(color ? "1 " : "0 ", stream);
		}
		putc('\n', stream);
	}
}

int usage() {
	printf("Usage: decicn [pmap|bmap|mask|info] file\n"
		"Extracts images (mono, colour, mask) from cicn resources.\n"
		"  pmap -> get the color pixmap (as PNM)\n"
		"  bmap -> get the monochrome bitmap\n"
		"  mask -> get the pixmap's alpha channel\n"
		"  info -> print some format details\n");
	return 1;
}

int main (int argc, char** argv) {
	enum output_type {
		none,
		pmap,
		bmap,
		mask,
		info
	} mode = none;
	FILE* input;
	CIcon cicn;

	if (argc != 3) {
		return usage();
	} else {
		if (0 == strcmp("pmap", argv[1])) {
			mode = pmap;
		} else if (0 == strcmp("bmap", argv[1])) {
			mode = bmap;
		} else if (0 == strcmp("mask", argv[1])) {
			mode = mask;
		} else if (0 == strcmp("info", argv[1])) {
			mode = info;
		}
		if (mode == none) return usage();

		input = fopen(argv[2], "rb");
		if (NULL == input) die("couldn't open file");

		read_CIcon(&cicn, input);

		fclose(input);

		switch (mode) {
		case pmap:
			print_PixMap(cicn.PMap, stdout);
			break;
		case bmap:
			print_BitMap(cicn.BMap, stdout);
			break;
		case mask:
			print_BitMap(cicn.Mask, stdout);
			break;
		case info:
			printf("loading file %s\n", argv[2]);
			dump_CIcon(&cicn, stdout);
			break;
		default:
			die("what mode is that?");
		}
	}
	return 0;
}
