/**
	\file trilist.cpp
	\brief LightEngine 3D: Triangle lists
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

#include "trilist.h"

#include "global.h"
#include "config.h"

/*****************************************************************************/
LeTriList::LeTriList() :
	fog(),
	srcIndices(NULL), dstIndices(NULL),
	noAllocated(0), noUsed(0), noValid(0)
{
	allocate(LE_TRILIST_MAX);
}

LeTriList::LeTriList(int noTriangles) :
	srcIndices(NULL), dstIndices(NULL),
	noAllocated(0), noUsed(0), noValid(0)
{
	allocate(noTriangles);
}

LeTriList::~LeTriList()
{
	if (triangles) delete[] triangles;
	if (srcIndices) delete[] srcIndices;
	if (dstIndices) delete[] dstIndices;
}

/*****************************************************************************/
/**
	\fn void LeTriList::allocate(int noTriangles)
	\brief Allocate memory to hold triangles
	\param[in] noTriangles maximum number of triangles
*/
void LeTriList::allocate(int noTriangles)
{
	triangles = new LeTriangle[noTriangles];
	srcIndices = new int[noTriangles * 3];
	dstIndices = new int[noTriangles * 3];
	noAllocated = noTriangles;
}

/**
	\fn void LeTriList::zSort()
	\brief Sort triangles according to their view distance (descending order)
*/
void LeTriList::zSort()
{
	if (!noValid) return;
	zMergeSort(srcIndices, dstIndices, noValid);
}

/*****************************************************************************/
void LeTriList::zMergeSort(int indices[], int tmp[], int nb)
{
	int h1 = nb >> 1;
	int h2 = nb - h1;
	if (h1 >= 2) zMergeSort(&indices[0], &tmp[0], h1);
	if (h2 >= 2) zMergeSort(&indices[h1], &tmp[h1], h2);
	int u = 0;
	int v = h1;
	float a = triangles[indices[u]].vd;
	float b = triangles[indices[v]].vd;
	for (int i = 0; i < nb; i++) {
		if (a > b) {
			tmp[i] = indices[u++];
			if (u == h1) {
				for (i++; i < nb; i++)
					tmp[i] = indices[v ++];
				for (int i = 0; i < nb; i++)
					indices[i] = tmp[i];
				return;
			}
			a = triangles[indices[u]].vd;

		}else{
			tmp[i] = indices[v++];
			if (v == nb) {
				for (i++; i < nb; i++)
					tmp[i] = indices[u ++];
				for (int i = 0; i < nb; i++)
					indices[i] = tmp[i];
				return;
			}
			b = triangles[indices[v]].vd;
		}
	}
}
