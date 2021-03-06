/**
	\file bmpcache.cpp
	\brief LightEngine 3D: Bitmap cache management
	\brief All platforms implementation
	\author Frederic Meslin (fred@fredslab.net)
	\twitter @marzacdev
	\website http://fredslab.net
	\copyright Frederic Meslin 2015 - 2018
	\version 1.75

	The MIT License (MIT)
	Copyright (c) 2015-2018 Frédéric Meslin

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "bmpcache.h"
#include "bmpfile.h"

#include "global.h"
#include "config.h"

#include <string.h>

#ifdef _MSC_VER
	#include "vs/vs-dirent.h"
#elif defined(__WATCOMC__)
	#include <direct.h>
#else
	#include <dirent.h>
#endif

/*****************************************************************************/
LeBmpCache bmpCache;

/*****************************************************************************/
LeBmpCache::LeBmpCache() :
	noSlots(0)
{
	memset(cacheSlots, 0, sizeof(Slot) * LE_BMPCACHE_SLOTS);

// Create the default bitmap (32x32 all white)
	LeBitmap * defBitmap = new LeBitmap();
	defBitmap->allocate(32, 32);
	memset(defBitmap->data, 0xFF, 32 * 32 * sizeof(LeColor));

// Register the default bitmap
	Slot * defSlot = &cacheSlots[0];
	defSlot->bitmap = defBitmap;
	strcpy(defSlot->path, "none");
	strcpy(defSlot->name, "default");
	defSlot->extras = NULL;
	defSlot->noExtras = 0;
	defSlot->cursor = 0;
	defSlot->flags = 0;
	noSlots = 1;
}

LeBmpCache::~LeBmpCache()
{
	clean();
}

/*****************************************************************************/
/**
	\fn void LeBmpCache::clean()
	\brief Unload all resources in cache and remove entries
*/
void LeBmpCache::clean()
{
	for (int i = 0; i < LE_BMPCACHE_SLOTS; i++)
		deleteSlot(i);
	noSlots = 0;
}

/*****************************************************************************/
/**
	\fn LeBitmap * LeBmpCache::loadBMP(const char * path)
	\brief Load a BMP file of given path and apply transforms
	\param[in] path BMP file path
	\return pointer to a new bitmap
*/
LeBitmap * LeBmpCache::loadBMP(const char * path)
{
	if (noSlots >= LE_BMPCACHE_SLOTS) {
		printf("bmpCache: no free cacheSlots!\n");
		return NULL;
	}

	LeBmpFile bmpFile = LeBmpFile(path);
	LeBitmap * bitmap = bmpFile.load();
	if (!bitmap) return NULL;

	int slot = createSlot(bitmap, path);
	if (slot < 0) {
		delete bitmap;
		return NULL;
	}

	bitmap->makeMipmaps();
	cacheSlots[slot].flags |= LE_BMPCACHE_MIPMAPPED;

	if (bitmap->flags & LE_BMPCACHE_RGBA) {
		bitmap->preMultiply();
		cacheSlots[slot].flags |= LE_BMPCACHE_RGBA;
	}

	return bitmap;
}

/*****************************************************************************/
/**
	\fn void LeBmpCache::loadDirectory(const char * path)
	\brief Load in cache all the recognized bitmap files from the given directory
*/
void LeBmpCache::loadDirectory(const char * path)
{
	char ext[LE_MAX_FILE_EXTENSION+1];
	char filePath[LE_MAX_FILE_PATH+1];

	DIR * dir = opendir(path);
	struct dirent * dd;

	while ((dd = readdir(dir))) {
		if (dd->d_name[0] == '.') continue;
		LeGlobal::getFileExtention(ext, LE_MAX_FILE_EXTENSION, (const char*) dd->d_name);

		if (strcmp(ext, "bmp") == 0) {
		// Load a Windows bmp file
			snprintf(filePath, LE_MAX_FILE_PATH, "%s/%s", path, dd->d_name);
			filePath[LE_MAX_FILE_PATH] = '\0';
			printf("bmpCache: loading bitmap: %s\n", filePath);
			loadBMP(filePath);
		}
	}
	closedir(dir);
}

/*****************************************************************************/
/**
	\fn int LeBmpCache::createSlot(LeBitmap * bitmap, const char * path)
	\brief Create a new slot to own the bitmap object
	\param[in] bitmap pointer to a valid bitmap object
	\param[in] path bitmap full path (directory + name + extension)
	\return cache slot number or -1 if no space available
*/
int LeBmpCache::createSlot(LeBitmap * bitmap, const char * path)
{
	for (int i = 0; i < LE_BMPCACHE_SLOTS; i++) {
		Slot * slot = &cacheSlots[i];
		if (slot->bitmap) continue;

	// Register the bitmap
		slot->bitmap = bitmap;
		strncpy(slot->path, path, LE_MAX_FILE_PATH);
		slot->path[LE_MAX_FILE_PATH] = '\0';
		LeGlobal::getFileName(slot->name, LE_MAX_FILE_NAME, path);

	// Initialize the flags
		slot->extras = NULL;
		slot->noExtras = 0;
		slot->cursor = 0;
		slot->flags = 0;

		noSlots++;
		return i;
	}
	return -1;
}

/**
	\fn void LeBmpCache::deleteSlot(int index)
	\brief Free a cache slot of given index
*/
void LeBmpCache::deleteSlot(int index)
{
	Slot * slot = &cacheSlots[index];
	if (slot->bitmap) delete slot->bitmap;
	if (slot->extras) delete [] slot->extras;
	memset(slot, 0, sizeof(Slot));
	noSlots--;
}

/*****************************************************************************/
/**
	\fn int LeBmpCache::getSlotFromName(const char * path)
	\brief Retrieve a bitmap slot index from a bitmap name or path
	\param[in] path bitmap path or name
	\return cache slot number or 0 (default slot) if not found
*/
int LeBmpCache::getSlotFromName(const char * path)
{
	char name[LE_MAX_FILE_NAME+1];
	LeGlobal::getFileName(name, LE_MAX_FILE_NAME, path);

// Search for resource
	for (int i = 0; i < noSlots; i++) {
		Slot * slot = &cacheSlots[i];
		if (!slot->bitmap) continue;
		if (strcmp(slot->name, name) == 0)
			return i;
	}

// Resource not found
	printf("bmpCache: %s not found!\n", path);
	return 0;
}

/**
	\fn LeBitmap * LeBmpCache::getBitmapFromName(const char * path)
	\brief Retrieve a bitmap object from its name or path
	\param[in] path bitmap path or name
	\return bitmap object or default bitmap if not found
*/
LeBitmap * LeBmpCache::getBitmapFromName(const char * path)
{
	int slot = getSlotFromName(path);
	return cacheSlots[slot].bitmap;
}