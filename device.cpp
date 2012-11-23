///////////////////////////////////////////////////////////////////////////////
// FILE:          device.cpp
// PROJECT:       Devices 
// SUBSYSTEM:     Device Controller 
//-----------------------------------------------------------------------------
// DESCRIPTION:   implementation of device example classes
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
// for revision history and comments see device.h
// this is rev 1.1
 
#include "device.h"
#include "TimeMicroseconds.h"

bool s_defaultBoolValue = false;
double  s_defaultDoubleValue = 0.;

// definitions for class Device

bool Device::Ready() const 
{ 
	return (STATE_IDLE == State());
};

// definitions for class Valve

// this will run some time after the client issues SetCommand(COMMAND_CLOSE)
// or SetCommand(COMMAND_OPEN) 
// same logic for SingleThrowValve and DoubleThrowValve
bool Valve::Update()
{
	bool ret = false;
	static int pendingCommand; // command to be queued, waiting for interlocks
	switch( State() )
	{
		case STATE_IDLE: // at rest
			switch ( Command())
			{
				case COMMAND_CLOSE:
					if (Close()) // interlock was met and command was issued 
					{
				        m_motionStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_CLOSING);
					}
					else
					{
				        m_waitStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_WAITING);
						pendingCommand = Command(); // wait for interlock 
					}
					SetCommand(COMMAND_IDLE);
					break;
				case COMMAND_OPEN:
					if (Open())
					{
        				m_motionStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_OPENING);
					}
					else
					{
				        m_waitStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_WAITING);
						pendingCommand = Command();
					}
					SetCommand(COMMAND_IDLE); // COMMAND WAS ACCEPTED
					break;
				default:// more commands.
					break; 
			} // end of switch on Command 
			if( COMMAND_IDLE == Command()) 
			{
				ret = true; // command was accepted or queued waiting for 
				// interlocks to be met
			}
		break; // STATE_IDLE
			
		case STATE_WAITING: // WAITING FOR INTERLOCK FOR QUEUED COMMAND
			switch ( pendingCommand) //very similar to switch statement as above
			{
				case COMMAND_CLOSE:
					if (Close()) // interlock was met and command was issued 
					{
				        m_motionStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_CLOSING);
					}
					else
					{
						if (TimeMicroseconds() - m_waitStartTime > DEMO_TIMEOUT)
						{
							SetState(STATE_INVALID);
						}
					}
					break;
				case COMMAND_OPEN:
					if (Open())
					{
        				m_motionStartTime = TimeMicroseconds(); // microseconds.
						SetState(STATE_OPENING);
					}
					else
					{
						if (TimeMicroseconds() - m_waitStartTime > DEMO_TIMEOUT)
						{
							SetState(STATE_INVALID);
						}
					}
					break;
				default:// more commands.
					break; 
			} // end of switch on previousCommand 

		break;
		
		case STATE_OPENING:
			if (IsOpened()) 
			{
				SetState(STATE_IDLE);
			}
			
			else
			{
				if (TimeMicroseconds() - m_motionStartTime > DEMO_TIMEOUT)
				{
					SetState(STATE_INVALID);
				}
			}
		break;

		case STATE_CLOSING:
			if (IsClosed()) 
			{
				SetState(STATE_IDLE);
			}
			
			else
			{
				if (TimeMicroseconds() - m_motionStartTime > DEMO_TIMEOUT)
				{
					SetState(STATE_INVALID);
				}
			}
		break;
			
		case STATE_INVALID:
			if (COMMAND_RESET == Command())
			{
				SetCommand(COMMAND_IDLE);
				IdleOutput();
				SetState(STATE_IDLE);
			}
		break;
		
	} // switch State()
	
	if (COMMAND_IDLE != Command()) // we were not able to process the command at this time,
	//  try to queue it
	{
		if (COMMAND_IDLE == pendingCommand)
		{
			pendingCommand = Command();
		}
		else // is this always just a programming bug? might we get here because IO 
		//from interlock signals/conditions is faulty? to do!
		{
			cerr << Name() << "could not accept command " << Command() << endl;
		}
		SetCommand(COMMAND_IDLE);
	}
	
	if (InvalidSensorState()) // if, for example, both the OPENED? and CLOSED? 
	// sensors are made
	{
		SetState(STATE_INVALID);
	}
	
}

int Valve::DoProcessCallBacks()  // called when data changes
{
	return Update()?1:0;
}	
	
int Valve::DoProcessTimeouts()   // called periodically to check completion
// of requested motions
{
	return Update()?1:0;
}


// definitions for SingleThrowValve

bool SingleThrowValve::InMotion()  // let's say it has one sensor
// for "closed?" one command for "close!"
{
    return (m_dos["CLOSE!"].Value() != m_dis["CLOSED?"].Value());
    
};

bool SingleThrowValve::IsOpened()
{
	return (!m_dis["CLOSED?"].Value());
};

bool SingleThrowValve::IsClosed()
{
	return (!InMotion() && !IsClosed());
}


bool SingleThrowValve::Close()
{
	bool ret = m_dis["CLOSE_OK?"].Value();
	if (ret)
	{
		m_dos["CLOSE!"].Set(true);
	}
	return ret;
}

bool SingleThrowValve::Open()
{
	bool ret = m_dis["OPEN_OK?"].Value();
	
	if (ret)
	{
		m_dos["CLOSE!"].Set(false);
	}
	return ret;
};

void  SingleThrowValve::IdleOutput()
{
	m_dos["CLOSE!"].Set(false);
}

// definitions for DoubleThrowValve

bool DoubleThrowValve::InMotion()
{
	bool ret;
	ret = (m_dos["CLOSE!"].Value() && !m_dis["CLOSED?"].Value()) || 
			(m_dos["OPEN!"].Value() && !m_dis["OPENED?"].Value());
	return ret;
	//if (SecondsNow() - MotionStartTime() > DEMO_TIMEOUT)
	
}
bool DoubleThrowValve::IsOpened()
{
	return (!m_dis["CLOSED?"].Value() && m_dis["OPENED?"].Value());
}

bool DoubleThrowValve::IsClosed()
{
	return (m_dis["CLOSED?"].Value() && !m_dis["OPENED?"].Value());
};


bool DoubleThrowValve::InvalidSensorState()
{
	return (m_dis["CLOSED?"].Value() && m_dis["OPENED?"].Value());
}

bool DoubleThrowValve::Close()
{
	bool ret = m_dis["CLOSE_OK?"].Value();
	if (ret)
	{
		m_dos["OPEN!"].Set(false);
		m_dos["CLOSE!"].Set(true);
	}
	return ret;
}

bool DoubleThrowValve::Open()
{
	bool ret = m_dis["OPEN_OK?"].Value();
	
	if (ret)
	{
		m_dos["CLOSE!"].Set(false);
		m_dos["OPEN!"].Set(true);
	}
	return ret;
}

void  DoubleThrowValve::IdleOutput()
{
	m_dos["CLOSE!"].Set(false);
	m_dos["OPEN!"].Set(false);
}
