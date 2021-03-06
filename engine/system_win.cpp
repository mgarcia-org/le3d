/**
	\file system_win.cpp
	\brief LightEngine 3D: Native OS / HW system manager
	\brief Windows OS implementation
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

/*****************************************************************************/
#if defined(_WIN32)

#include "system.h"

#include "global.h"
#include "config.h"

#include <stdlib.h>  
#include <signal.h>  
#include <windows.h>
#include <stdio.h>

/*****************************************************************************/
LeSystem sys;
void SignalHandler(int signal);

/*****************************************************************************/
LeSystem::LeSystem() :
	running(false),
	status(LE_SYSTEM_UNKNOWN)
{
}

LeSystem::~LeSystem()
{
}

/*****************************************************************************/
/**
	\fn void LeSystem::initialize()
	\brief Initialize native OS / HW specifics
*/
void LeSystem::initialize()
{
	running = true;
	status = LE_SYSTEM_ALRIGHT;

	signal(SIGTERM, SignalHandler);
    signal(SIGABRT, SignalHandler);  
    signal(SIGINT, SignalHandler);
}

/**
	\fn void LeSystem::terminate()
	\brief Terminate native OS / HW specifics
*/
void LeSystem::terminate()
{
	running = false;
}

/*****************************************************************************/
/**
	\fn void LeSystem::update()
	\brief Update native OS / HW specifics
*/
void LeSystem::update()
{
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			status = LE_SYSTEM_EXIT_QUIT;
			running = false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/*****************************************************************************/
/**
	\fn int LeSystem::getStatus()
	\brief Retrieve native OS / HW current status
	\return OS status
*/
int LeSystem::getStatus()
{
	return status;
}

/*****************************************************************************/
void SignalHandler(int signal)  
{
    switch (signal) {
		case SIGTERM:	sys.status = LE_SYSTEM_EXIT_QUIT;	break;
		case SIGABRT:	sys.status = LE_SYSTEM_EXIT_ABORT;	break;
		case SIGINT:	sys.status = LE_SYSTEM_EXIT_ABORT;	break;
		default: 		sys.status = LE_SYSTEM_UNKNOWN;		break;
    }
	sys.running = false;
}

#endif