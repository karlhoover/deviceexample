///////////////////////////////////////////////////////////////////////////////
// FILE:          statedefinitions.h
// PROJECT:       Devices 
// SUBSYSTEM:     Device Controller 
//-----------------------------------------------------------------------------
// DESCRIPTION:   definitions for Valve example for vacuum control system
//                this defines States and Commands for a simple
//                state transition machine model
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
// REVISIONS:     rev 1.0 March 25, 2009 
//                rev 1.1 March 27, 2009  more comments
//
// NOTES:
//

#define STATE_INITIALIZING 90     // enter this state on program startup
#define STATE_IDLE 0              // device is seen to be at rest
#define STATE_WAITING 5           // waiting for an interlock to be met
#define STATE_MOTION_COMPLETE 10  // requested motion just completed
#define STATE_CLOSING 20          // specific to open/close single axis
#define STATE_OPENING 30          // "
#define STATE_INVALID -1          // impossible state, such as "IsClosed" && "IsOpen"
// 5 seconds
#define DEMO_TIMEOUT 5000000 

#define COMMAND_IDLE 0            // null command
#define COMMAND_CLOSE 20          // specific to open/close single axis
#define COMMAND_OPEN 30           // "
#define COMMAND_RESET 90          // attempt to reach "STATE_IDLE"

