/**
	\file objfile.cpp
	\brief LightEngine 3D: Wavefront object file importer
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

#include "objfile.h"
#include "bmpcache.h"

#include "global.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>

LeObjMaterial defMaterial;

/*****************************************************************************/
LeObjFile::LeObjFile(const char * filename) :
	path(NULL),
	materials(NULL), curMaterial(&defMaterial), noMaterials(0),
	normals(NULL), noNormals(0)
{
	memset(line, 0, LE_OBJ_MAX_LINE+1);
	if (filename) path = _strdup(filename);
}

LeObjFile::~LeObjFile()
{
	if (path) free(path);
	if (materials) delete[] materials;
}

/*****************************************************************************/
/**
	\fn LeMesh * LeObjFile::load(int index)
	\brief Load the mesh of the given index from the file
	\return pointer to a new loaded mesh, else NULL (error)
*/
LeMesh * LeObjFile::load(int index)
{
	FILE * file = fopen(path, "rb");
	if (!file) return NULL;
	if (!materials)	loadMaterialLibraries(file);

	LeMesh * mesh = new LeMesh();

	fseek(file, 0, SEEK_SET);
	int noObjects = 0;
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "o ", 2) == 0) {
			if (noObjects ++ == index) {
				strncpy(mesh->name, &line[2], LE_OBJ_MAX_NAME);
				mesh->name[LE_OBJ_MAX_NAME] = '\0';
				importMeshAllocate(file, mesh);
				importMeshData(file, mesh);
				break;
			}
		}
		len = readLine(file);
	}
	
	fclose(file);
	return mesh;
}

/*****************************************************************************/
/**
	\fn LeMesh * LeObjFile::save(int index)
	\brief Load the mesh of the given index from the file
	\return pointer to a new loaded mesh, else NULL (error)
*/
void LeObjFile::save(const LeMesh * mesh)
{
	FILE * objFile = fopen(path, "wb");
	if (!objFile) return;

	char mtlName[LE_MAX_FILE_NAME+1];
	mtlName[0] = '\0';
	char mtlPath[LE_MAX_FILE_PATH+1];
	mtlPath[0] = '\0';
	
	LeGlobal::getFileName(mtlName, LE_MAX_FILE_NAME, path);
	LeGlobal::getFileDirectory(mtlPath, LE_MAX_FILE_PATH, path);
	int len = strlen(mtlName);
	if (len < 3) {
		fclose(objFile);
		return;
	}
	mtlName[len-3] = 'm';
	mtlName[len-2] = 't';
	mtlName[len-1] = 'l';
	strcat(mtlPath, "/");
	strcat(mtlPath, mtlName);
	
	FILE * mtlFile = fopen(mtlPath, "wb");
	if (!mtlFile) {
		fclose(objFile);
		return;
	}
	
	fseek(mtlFile, 0, SEEK_SET);
	exportHeader(mtlFile, mesh);
	exportMaterials(mtlFile, mesh);
		
	fseek(objFile, 0, SEEK_SET);
	exportHeader(objFile, mesh);
	fprintf(objFile, "mtllib %s\n", mtlName);
	fprintf(objFile, "o %s\n", mesh->name);
	exportVertexes(objFile, mesh);
	exportTexCoords(objFile, mesh);
	exportTriangles(objFile, mesh);
	
	fclose(mtlFile);
	fclose(objFile);
}

/*****************************************************************************/
void LeObjFile::exportHeader(FILE * file, const LeMesh * mesh)
{
	fprintf(file, "# LE3D - ObjFile exporter\n");
	fprintf(file, "# V1.0 - 30.04.18\n");
	fprintf(file, "# https://github.com/Marzac/le3d\n\n");
}

void LeObjFile::exportMaterials(FILE * file, const LeMesh * mesh)
{
	fprintf(file, "newmtl Default\n");
	fprintf(file, "Ns 0.8\n");
	fprintf(file, "Ka 1.0 1.0 1.0\n");
	fprintf(file, "Kd 1.0 1.0 1.0\n");
	fprintf(file, "Ks 0.5 0.5 0.5\n\n");

	if (!mesh->texSlotList) return;
	
	for (int i = 0; i < mesh->noTriangles; i++) {
		bool texNew = true;
		int tex = mesh->texSlotList[i];
		for (int j = 0; j < i; j++) {
			if (mesh->texSlotList[j] == tex) {
				texNew = false;
				break;
			}
		}
		if (!texNew) continue;
		
		char * name = bmpCache.cacheSlots[tex].name;
		fprintf(file, "newmtl %s\n", name);
		fprintf(file, "Ns 0.8\n");
		fprintf(file, "Ka 1.0 1.0 1.0\n");
		fprintf(file, "Kd 1.0 1.0 1.0\n");
		fprintf(file, "Ks 0.5 0.5 0.5\n");
		fprintf(file, "map_Kd %s\n\n", name);
	}
}

void LeObjFile::exportVertexes(FILE * file, const LeMesh * mesh)
{
	if (!mesh->vertexes) return;
	for (int i = 0; i < mesh->noVertexes; i++)
		fprintf(file, "v %f %f %f\n", mesh->vertexes[i].x, mesh->vertexes[i].y, mesh->vertexes[i].z);
	fprintf(file, "\n");
}

void LeObjFile::exportTexCoords(FILE * file, const LeMesh * mesh)
{
	if (!mesh->texCoords) return;
	for (int i = 0; i < mesh->noTexCoords; i++)
		fprintf(file, "vt %f %f\n", mesh->texCoords[i*2+0], mesh->texCoords[i*2+1]);
	fprintf(file, "\n");
}

void LeObjFile::exportNormals(FILE * file, const LeMesh * mesh)
{
	if (!mesh->normals) return;
	for (int i = 0; i < mesh->noVertexes; i++)
		fprintf(file, "vn %f %f %f\n", mesh->normals[i].x, mesh->normals[i].y, mesh->normals[i].z);
	fprintf(file, "\n");
}
	
void LeObjFile::exportTriangles(FILE * file, const LeMesh * mesh)
{
	if (!mesh->vertexesList) return;
	if (!mesh->texCoordsList) return;
	if (!mesh->texSlotList) return;
	
	int texLast = -1;
	for (int i = 0; i < mesh->noTriangles; i++) {
		int tex = mesh->texSlotList[i];
		if (texLast != tex) {
			if (!tex) {
				fprintf(file, "usemtl Default\n");
			}else{
				char * name = bmpCache.cacheSlots[tex].name;
				fprintf(file, "usemtl %s\n", name);
			}
			fprintf(file, "s off\n");
			texLast = tex;
		}
		
		if (mesh->texCoordsList && mesh->normals) {
			fprintf(file, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
				mesh->vertexesList[i * 3 + 0] + 1, mesh->texCoordsList[i * 3 + 0] + 1, i * 3 + 1,
				mesh->vertexesList[i * 3 + 1] + 1, mesh->texCoordsList[i * 3 + 1] + 1, i * 3 + 2,
				mesh->vertexesList[i * 3 + 2] + 1, mesh->texCoordsList[i * 3 + 2] + 1, i * 3 + 3
			);
		}else if (mesh->texCoordsList) {
			fprintf(file, "f %i/%i %i/%i %i/%i\n",
				mesh->vertexesList[i * 3 + 0] + 1, mesh->texCoordsList[i * 3 + 0] + 1,
				mesh->vertexesList[i * 3 + 1] + 1, mesh->texCoordsList[i * 3 + 1] + 1,
				mesh->vertexesList[i * 3 + 2] + 1, mesh->texCoordsList[i * 3 + 2] + 1
			);
		}else if (mesh->normals) {
			fprintf(file, "f %i//%i %i//%i %i//%i\n",
				mesh->vertexesList[i * 3 + 0] + 1, i * 3 + 1,
				mesh->vertexesList[i * 3 + 1] + 1, i * 3 + 2,
				mesh->vertexesList[i * 3 + 2] + 1, i * 3 + 3
			);
		}else{
			fprintf(file, "f %i %i %i\n",
				mesh->vertexesList[i * 3 + 0] + 1,
				mesh->vertexesList[i * 3 + 1] + 1,
				mesh->vertexesList[i * 3 + 2] + 1
			);
		}
	}
	fprintf(file, "\n");
}

/*****************************************************************************/
/**
	\fn int LeObjFile::getNoMeshes()
	\brief Get the number of meshes in the file
	\return number of meshes
*/
int LeObjFile::getNoMeshes()
{
	FILE * file = fopen(path, "rb");
	if (!file) return 0;

	int noObjects = 0;
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "o ", 2) == 0) noObjects ++;
		len = readLine(file);
	}

	fclose(file);
	return noObjects;
}

/**
	\fn const char * LeObjFile::getMeshName(int index)
	\brief Get the name of the mesh of the given index
	\return name of the mesh, NULL else (error)
*/
const char * LeObjFile::getMeshName(int index)
{
	FILE * file = fopen(path, "rb");
	if (!file) return NULL;

	int noObjects = 0;
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "o ", 2) == 0) {
			if (noObjects ++ == index) {
				fclose(file);
				return &line[2];
			}
		}
		len = readLine(file);
	}

	fclose(file);
	return NULL;
}

/*****************************************************************************/
void LeObjFile::loadMaterialLibraries(FILE * file)
{
	char filename[LE_OBJ_MAX_PATH+1];
	char libName[LE_OBJ_MAX_PATH+LE_OBJ_MAX_NAME+1];

	noMaterials = 0;

	fseek(file, 0, SEEK_SET);
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "mtllib ", 7) == 0) {
			strncpy(libName, &line[7], LE_OBJ_MAX_NAME);
			libName[LE_OBJ_MAX_NAME] = '\0';
			LeGlobal::getFileDirectory(filename, LE_OBJ_MAX_PATH, path);
			strcat(filename, libName);

			FILE * libFile = fopen(filename, "rb");
			if (!libFile) continue;
			importMaterialAllocate(libFile);
			importMaterialData(libFile);
			fclose(libFile);
		}
		len = readLine(file);
	}
}

/*****************************************************************************/
void LeObjFile::importMaterialAllocate(FILE * file)
{
	int lastNoMaterials = noMaterials;

	int start = ftell(file);
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "newmtl ", 7) == 0) noMaterials ++;
		len = readLine(file);
	}
	
	fseek(file, start, SEEK_SET);
	if (noMaterials != lastNoMaterials) {
		LeObjMaterial * newMaterials = new LeObjMaterial[noMaterials];
		for (int m = 0; m < lastNoMaterials; m++)
			newMaterials[m] = materials[m];
		if (materials) delete[] materials;
		materials = newMaterials;
	}
}

void LeObjFile::importMaterialData(FILE * file)
{
	int materialIndex = -1;
	int start = ftell(file);
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "newmtl ", 7) == 0) {
			materialIndex ++;
			strncpy(materials[materialIndex].name, &line[7], LE_OBJ_MAX_NAME);
			materials[materialIndex].name[LE_OBJ_MAX_NAME] = '\0';
		}else{
			if (materialIndex < 0) continue;
			if (strncmp(line, "Ka ", 3) == 0) {
				readColor(&line[3], materials[materialIndex].ambient);
			}else if (strncmp(line, "Kd ", 3) == 0) {
				readColor(&line[3], materials[materialIndex].diffuse);
			}else if (strncmp(line, "Ks ", 3) == 0) {
				readColor(&line[3], materials[materialIndex].specular);
			}else if (strncmp(line, "Ns ", 3) == 0) {
				sscanf(&line[3], "%f", &materials[materialIndex].shininess);
			}else if (strncmp(line, "d ", 2) == 0) {
				sscanf(&line[2], "%f", &materials[materialIndex].transparency);
			}else if (strncmp(line, "map_Kd ", 7) == 0) {
				strncpy(materials[materialIndex].texture, &line[7], LE_OBJ_MAX_NAME);
				materials[materialIndex].texture[LE_OBJ_MAX_NAME] = '\0';
			}
		}
		len = readLine(file);
	}
	
	fseek(file, start, SEEK_SET);
}

LeObjMaterial * LeObjFile::getMaterialFromName(const char * name)
{
	for (int i = 0; i < noMaterials; i++)
		if (strcmp(name, materials[i].name) == 0)
			return &materials[i];
	return &defMaterial;
}

/*****************************************************************************/
void LeObjFile::importMeshAllocate(FILE * file, LeMesh * mesh)
{
	int start = ftell(file);
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "v ", 2) == 0) mesh->noVertexes ++;
		else if (strncmp(line, "vt ", 3) == 0) mesh->noTexCoords ++;
		else if (strncmp(line, "vn ", 3) == 0) noNormals ++;
		else if (strncmp(line, "f ", 2) == 0) mesh->noTriangles ++;
		else if (strncmp(line, "o ", 2) == 0) break;
		len = readLine(file);
	}
	
	fseek(file, start, SEEK_SET);
	mesh->allocate(mesh->noVertexes, mesh->noTexCoords, mesh->noTriangles);
	if (noNormals) mesh->allocateNormals();
	normals = new LeVertex[noNormals];
}

void LeObjFile::importMeshData(FILE * file, LeMesh * mesh)
{
	int vertexesIndex = 0;
	int texCoordsIndex = 0;
	int normalsIndex = 0;
	int trianglesIndex = 0;
	
	curMaterial = &defMaterial;

	int start = ftell(file);
	int len = readLine(file);
	while (len) {
		if (strncmp(line, "v ", 2) == 0) readVertex(mesh, vertexesIndex ++);
		else if (strncmp(line, "vt ", 3) == 0) readTexCoord(mesh, texCoordsIndex ++);
		else if (strncmp(line, "vn ", 3) == 0) readNormal(mesh, normalsIndex ++);
		else if (strncmp(line, "f ", 2) == 0) {
			readTriangle(mesh, trianglesIndex);
			mesh->colors[trianglesIndex] = curMaterial->diffuse;
			int slot = 0;
			if (curMaterial->texture[0]) {
				slot = bmpCache.getSlotFromName(curMaterial->texture);
				if (!slot) printf("objFile: using default texture (instead of %s)!\n", curMaterial->texture);
			}
			mesh->texSlotList[trianglesIndex ++] = slot;
		}else if (strncmp(line, "usemtl ", 7) == 0) {
			curMaterial = getMaterialFromName(&line[7]);
		}else if (strncmp(line, "o ", 2) == 0) break;
		len = readLine(file);
	}
	
	fseek(file, start, SEEK_SET);
	if (normals) {
		delete[] normals;
		noNormals = 0;
	}
}

/*****************************************************************************/
void LeObjFile::readVertex(LeMesh * mesh, int index)
{
	LeVertex v;
	sscanf(line, "v %f %f %f %f", &v.x, &v.y, &v.z, &v.w);
	mesh->vertexes[index] = v;
}

void LeObjFile::readTexCoord(LeMesh * mesh, int index)
{
	float u = 0.0f, v = 0.0f, w = 0.0f;
	sscanf(line, "vt %f %f %f", &u, &v, &w);
	mesh->texCoords[index * 2] = u;
	mesh->texCoords[index * 2 + 1] = 1.0f - v;
}

void LeObjFile::readNormal(LeMesh * mesh, int index)
{
	LeVertex n;
	sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
	normals[index] = n;
}

void LeObjFile::readTriangle(LeMesh * mesh, int index)
{
	int * vList = &mesh->vertexesList[index * 3];
	int * tList = &mesh->texCoordsList[index * 3];
	
	const char * buffer = &line[2];
	for (int i = 0; i < 3; i++) {
		int v = 0, t = 0, n = 0;
		bool vOk = true, tOk = true, nOk = true;
		int r = sscanf(buffer, "%i/%i/%i", &v, &t, &n);
		if (r != 3) {
			vOk = true, tOk = true, nOk = false;
			r = sscanf(buffer, "%i/%i", &v, &t);
			if (r != 2) {
				vOk = true, tOk = false, nOk = true;
				r = sscanf(buffer, "%i//%i", &v, &n);
				if (r != 2) {	
					vOk = true, tOk = false, nOk = false;
					r = sscanf(buffer, "%i", &v);
					if (r != 1) {
						vOk = false, tOk = false, nOk = false;
					}
				}
			}
		}

		if (vOk) {
			if (v <= 0 || v > mesh->noVertexes) {
				printf("objFile: error detected in vertex indices!\n");
				continue;
			}
			vList[i] = v - 1;
		}
		
		if (tOk) {
			if (t <= 0 || t > mesh->noTexCoords) {
				printf("objFile: error detected in texture coordinate indices!\n");
				continue;
			}
			tList[i] = t - 1;
		}
		
		if (nOk) {
			if (n <= 0 || n > noNormals) {
				printf("objFile: error detected in normal indices!\n");
				continue;
			}
			mesh->normals[index] = normals[n - 1];
		}
			
		buffer = strchr(buffer, ' ');
		if (!buffer) break;
		buffer ++;
	}
}

/*****************************************************************************/
void LeObjFile::readColor(const char * buffer, LeColor & color)
{
	float r = 0.0f, g = 0.0f, b = 0.0f;
	sscanf(buffer, "%f %f %f", &r, &g, &b);
	color = LeColor(
		(uint8_t) cmbound(r * 255.0f, 0.0f, 255.0f),
		(uint8_t) cmbound(g * 255.0f, 0.0f, 255.0f),
		(uint8_t) cmbound(b * 255.0f, 0.0f, 255.0f),
		0
	);
}

/*****************************************************************************/
int LeObjFile::readLine(FILE * file)
{
	int len = 0;
	bool skip = false;

	while(!feof(file)) {
		int c = fgetc(file);
		if (c =='\n' || c == '\r') {
			if (len != 0) break;
			else {
				skip = false;
				continue;
			}
		}

		if (skip || c == '#') {
			skip = true;
			continue;
		}
		
		if (len < LE_OBJ_MAX_LINE)
			line[len ++] = c;
	}
	
	line[len] = '\0';
	return len;
}
