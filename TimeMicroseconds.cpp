///////////////////////////////////////////////////////////////////////////////
// FILE:          TimeMicroseconds.cpp
// PROJECT:       Devices 
// SUBSYSTEM:     Device Controller 
//-----------------------------------------------------------------------------
// DESCRIPTION:   
//             
// COPYRIGHT:     Karl Hoover, No Prior Art, 2009
//
// LICENSE:       This library is free software; you can redistribute it and/or
//                modify it under the terms of the GNU Lesser General Public
//                License (LGPL) as published by the Free Software Foundation.
//                See license text at http://www.gnu.org/licenses/lgpl.txt 
//                Briefly you can use this source code pretty much however you
//                wish, as long as this notice remains intact and you
//                prominently acknowledge this copyright.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Karl Hoover  karl.hoover@gmail.com
//
// REVISIONS:     
//
// NOTES:   
// see TimeMicroseconds.h for comments and history

#include <sys/time.h>
unsigned long long TimeMicroseconds()
{
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);
	return (unsigned long long)timeNow.tv_sec * 1000000
		+ (unsigned long long)timeNow.tv_usec;
}
