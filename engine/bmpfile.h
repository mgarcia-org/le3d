/**
	\file bmpfile.h
	\brief LightEngine 3D: BMP file importer / exporter
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

#ifndef LE_BMPFILE_H
#define LE_BMPFILE_H

#include "global.h"
#include "config.h"

#include "bitmap.h"
#include <stdio.h>

/*****************************************************************************/
/**
	\class LeBmpFile
	\brief Load and store bitmaps in uncompressed MS Windows bitmap format 
*/
class LeBmpFile
{
public:
	LeBmpFile(const char * filename);
	~LeBmpFile();

	LeBitmap * load();
	void save(const LeBitmap * bitmap);

private:
	int readBitmap(FILE * file, LeBitmap * bitmap);
	int writeBitmap(FILE * file, const LeBitmap * bitmap);
	char * path;
};

#endif // LE_BMPFILE_H
