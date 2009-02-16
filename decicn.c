/*

  decicn.c
  16 september 2008
  extracts images (mono, colour, mask) from cicn resources

  Copyright (C) 2008 Neil Gentleman

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

void die(char* message) {
	fputs(message, stderr);
	fputs("\n", stderr);
	exit(1);
}

#define assert(ok) assert_(ok, __LINE__)
void assert_(int ok, int line) {
	static char assert_buf[64];
	if (!ok) {
		snprintf(assert_buf, sizeof(assert_buf), "Assertion failed at line %d", line);
		die(assert_buf);
	}
}

uint16_t getint(FILE* file) {
	int x, y;
	if ((x = fgetc(file)) == EOF) die("getint failed");
	if ((y = fgetc(file)) == EOF) die("getint failed");
	return (x<<8) + y;
}

uint32_t getlong(FILE* file) {
	int x, y;
	x = getint(file);
	y = getint(file);
	return (x<<16) + y;
}

struct ColorSpec {
	uint16_t value;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
};

struct BitMap {
	uint8_t* baseAddr;
	uint16_t rowBytes;
	uint16_t bounds[4];
};

struct ColorTable {
	uint32_t ctSeed;
	uint16_t ctFlags;
	uint16_t ctSize;
	struct ColorSpec* ctTable;
};

struct PixMap {
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
};

struct CIcon {
	struct PixMap* PMap;
	struct BitMap* Mask;
	struct BitMap* BMap;
};

void read_PixMap(struct PixMap* pmap, FILE* f) {
	pmap->baseAddr = NULL;
	assert(0 == getlong(f));
	pmap->rowBytes = getint(f);
	assert(0x8000 == (pmap->rowBytes & 0xC000));
	pmap->bounds[1] = getint(f);
	pmap->bounds[0] = getint(f);
	pmap->bounds[3] = getint(f);
	pmap->bounds[2] = getint(f);
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

void read_BitMap(struct BitMap* bmap, FILE* f) {
	bmap->baseAddr = NULL;
	assert(0 == getlong(f));
	bmap->rowBytes = getint(f);
	bmap->bounds[1] = getint(f);
	bmap->bounds[0] = getint(f);
	bmap->bounds[3] = getint(f);
	bmap->bounds[2] = getint(f);
}

void read_CIcon(struct CIcon* cicn, FILE* f) {
	int len;

	cicn->PMap = calloc(1, sizeof(struct PixMap));
	if (NULL == cicn->PMap) die("calloc failed");

	cicn->Mask = calloc(1, sizeof(struct BitMap));
	if (NULL == cicn->Mask) die("calloc failed");

	cicn->BMap = calloc(1, sizeof(struct BitMap));
	if (NULL == cicn->BMap) die("calloc failed");

	read_PixMap(cicn->PMap, f);
	read_BitMap(cicn->Mask, f);
	read_BitMap(cicn->BMap, f);
	assert(0 == getlong(f));

	len = cicn->Mask->rowBytes * (cicn->Mask->bounds[3] - cicn->Mask->bounds[1]);
	cicn->Mask->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->Mask->baseAddr) die("calloc failed");
	fread(cicn->Mask->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));

	len = cicn->BMap->rowBytes * (cicn->BMap->bounds[3] - cicn->BMap->bounds[1]);
	cicn->BMap->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->BMap->baseAddr) die("calloc failed");
	fread(cicn->BMap->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));

	cicn->PMap->pmTable.ctSeed = getlong(f);
	cicn->PMap->pmTable.ctFlags = getint(f);
	cicn->PMap->pmTable.ctSize = getint(f);
	cicn->PMap->pmTable.ctTable = calloc(cicn->PMap->pmTable.ctSize + 1, sizeof(struct ColorSpec));
	if (NULL == cicn->PMap->pmTable.ctTable) die("calloc failed");
	for (len = 0; len <= cicn->PMap->pmTable.ctSize; len++) {
		cicn->PMap->pmTable.ctTable[len].value = getint(f);
		cicn->PMap->pmTable.ctTable[len].red = getint(f);
		cicn->PMap->pmTable.ctTable[len].green = getint(f);
		cicn->PMap->pmTable.ctTable[len].blue = getint(f);
	}

	len = (cicn->PMap->rowBytes & 0x7FFF) * (cicn->PMap->bounds[3] - cicn->PMap->bounds[1]);
	cicn->PMap->baseAddr = calloc(len, sizeof(uint8_t));
	if (NULL == cicn->PMap->baseAddr) die("calloc failed");
	fread(cicn->PMap->baseAddr, sizeof(uint8_t), len, f);
	assert(!ferror(f));
}

void print_P3_ColorSpec(struct ColorSpec* cs, FILE* fp) {
	fprintf(fp, "%d %d %d\n", cs->red, cs->green, cs->blue);
}

void print_PixMap(struct PixMap* pmap, FILE* stream) {
	unsigned int x, y, i, j, idx, color;
	x = pmap->bounds[2] - pmap->bounds[0];
	y = pmap->bounds[3] - pmap->bounds[1];

	fputs("P3\n# converted from cicn\n", stream);
	fprintf(stream, "%d %d\n", x, y);
	fputs("65535\n", stream);

	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			idx = i * (pmap->rowBytes & 0x3FFF) * 8 / pmap->pixelSize + j;
			switch (pmap->pixelSize) {
			case 1:
				color = pmap->baseAddr[idx / 8];
				color &= 0x80 >> (idx & 7);
				color >>= 7 - (idx & 7);
				break;
			case 2:
				color = pmap->baseAddr[idx / 4];
				switch (idx & 3) {
				case 0:
					color &= 0xC0;
					color >>= 6;
				case 1:
					color &= 0x30;
					color >>= 4;
				case 2:
					color &= 0x0C;
					color >>= 2;
				case 3:
					color &= 0x03;
					color >>= 0;
				}
				break;
			case 4:
				color = pmap->baseAddr[idx / 2];
				if ((color & 1) == 1) {
					color &= 0x0F;
					color >>= 0;
				} else {
					color &= 0xF0;
					color >>= 4;
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
		}
	}
}

void print_BitMap(struct BitMap* bmap, FILE* stream) {
	unsigned int x, y, i, j, idx, color;
	x = bmap->bounds[2] - bmap->bounds[0];
	y = bmap->bounds[3] - bmap->bounds[1];

	fputs("P1\n# converted from cicn\n", stream);
	fprintf(stream, "%d %d\n", x, y);

	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			idx = i * bmap->rowBytes * 8 + j;
			color = bmap->baseAddr[idx / 8];
			color &= 0x80 >> (idx & 7);
			fputs(color ? "1 " : "0 ", stream);
		}
		putc('\n', stream);
	}
}

int usage() {
	puts("usage: decicn [pmap|bmap|mask] file");
	return 1;
}

int main (int argc, char** argv) {
	enum output_type {
		none,
		pmap,
		bmap,
		mask
	} mode = none;
	FILE* input;
	struct CIcon cicn;

	if (argc != 3) {
		return usage();
	} else {
		if (0 == strcmp("pmap", argv[1])) {
			mode = pmap;
		} else if (0 == strcmp("bmap", argv[1])) {
			mode = bmap;
		} else if (0 == strcmp("mask", argv[1])) {
			mode = mask;
		}
		if (mode == none) return usage();

		input = fopen(argv[2], "rb");
		if (NULL == input) die("couldn't open file");

		read_CIcon(&cicn, input);

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
		default:
			die("what mode is that?");
		}

		fclose(input);
	}
	return 0;
}

