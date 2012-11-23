///////////////////////////////////////////////////////////////////////////////
// FILE:          device.h 
// PROJECT:       Devices 
// SUBSYSTEM:     Device Controller 
//-----------------------------------------------------------------------------
// DESCRIPTION:   definitions for Valve example for vacuum control system
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
// REVISIONS:     rev 0.0 March 25, 2009   compiles with gcc 4.0.1  
//                rev 1.0 March 25, 2009   add pthread mutexes around output
//                    points
//                rev 1.1 March 27, 2009   more comments
//
// NOTES:   
// I've put multiple classes into one header file, as this library is
// primarily for pedagogical purposes.
// This demonstrates a simple object model for the basis of an instrument
// or equipment control library. There are StateObjects which are the base class
// major components in the system, and IO points which are the bits for generic
// digital input/output. 'Devices' are StateObjects which contain maps of I/O
// points. Then I subclassed 'Device' for two particular types of valves,
// as described below.  The sample compiles but still needs a main test
// harness.    KH. March 27, 2009
//
// to reiterate:
// all in this header, for demo purposes are the following classes:
// StateObject - a base class for named objects which can accept commands and 
//      maintain a State
// IO - a base class for objects which map to named IO points in the system,
//      with the following specializations:
// 		DigitalInput
//		DigitalOutput
//		AnalogueInput
//		AnalougeOutput
// Device - StateObjects which contain maps of named IO points, and maintain
// 		a mechanical State
// 		these maps need to be setup in an object factory from a table like
//      the following:
// 		Device name,   IO attribute,  IO path,       IO type,    IO access,
// 		Valve1,        OPENED?,       Valve1OPENED?, bool,       ReadOnly	
// Valve - a single axis Device which accepts COMMAND_OPEN, COMMAND_CLOSE,
//      COMMAND_RESET, checks appropriate interlock IO, sends commands to
//      equipment and then waits for motion to complete, flags timeout if
//      interlock is not met within specified number seconds, flags timeout
//      if motion is not completed in specified number of seconds.
// SingleThrowValve - a Valve which can set CLOSE! output to an actuator
//       and can see CLOSED? sensor
// DoubleThrowValve - a Valve which can set CLOSE! and OPEN! outputs and can
//       see CLOSED? and OPENED? sensors

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <pthread.h>

#include "statedefinitions.h"
using namespace std;


class StateObject
{
public:
	StateObject( const string name):m_name(name), m_state(STATE_IDLE) {} ;
	virtual ~StateObject() {};
	string Name() const {return m_name;};
	int State() const {return m_state;};
	int Command() const {return m_command;}
	bool SetCommand( const int theCommand) 
	{
	m_command  = theCommand;
	return true;
	} ;
private:
        string m_name;
        int m_state;
        int m_command;
protected:
	 void SetState(const int theState) { m_state = theState;};
};

class IO
{
public:
	IO(const string theName):m_name(theName){};
	virtual ~IO() {};  // possible performance improvement: verify default dtor good enough
	string Name() const {return m_name;};
private:
	string m_name;
};

extern  bool s_defaultBoolValue;  // needed for default ctor in std::map
extern double  s_defaultDoubleValue;


class DigitalInput : public IO
{
private:
	const bool& m_iopoint;
	pthread_mutex_t m_mtx;
public:
	DigitalInput(const string theName, bool& iopoint):IO(theName), m_iopoint(iopoint) {};
	DigitalInput() : IO(""), m_iopoint(s_defaultBoolValue) {}; 
	~DigitalInput() {pthread_mutex_destroy(&m_mtx);};
	bool Value() const { return m_iopoint;};
};	

class DigitalOutput : public IO
{
private:
	bool& m_iopoint;
	pthread_mutex_t m_mtx;
public:
	DigitalOutput(const string theName, bool& iopoint):IO(theName), m_iopoint(iopoint)
		{pthread_mutex_init(&m_mtx, NULL);};
	DigitalOutput() : IO(""), m_iopoint(s_defaultBoolValue) {};
	~DigitalOutput() {};
	void Set(const bool value)
	{
	    pthread_mutex_lock(&m_mtx);
	    m_iopoint = value;
	    pthread_mutex_unlock(&m_mtx);
	};
	bool Value() const { return m_iopoint;};
};	

class AnalogueInput : public IO
{
private:
	const double& m_iopoint;
public:
	AnalogueInput(const string theName, const double& iopoint):
		IO(theName), m_iopoint(iopoint) {};
	AnalogueInput() : IO(""), m_iopoint(s_defaultDoubleValue) {};
	~AnalogueInput(){};
	double Value() const {return m_iopoint;};
};	

class AnalogueOutput : public IO
{
private:
	double& m_iopoint;
	pthread_mutex_t m_mtx;
public:
	AnalogueOutput(const string theName, double& iopoint):
		IO(theName), m_iopoint(iopoint) 
	    {pthread_mutex_init(&m_mtx, NULL);};
	AnalogueOutput() : IO(""), m_iopoint(s_defaultDoubleValue) {};
	~AnalogueOutput(){pthread_mutex_destroy(&m_mtx);};
	void Set(const double value)
	{
		pthread_mutex_lock(&m_mtx); // for demo purpose, block until we can write
		// todo need trylock until a timeout expires.
		m_iopoint = value;
		pthread_mutex_unlock(&m_mtx);
	}
};	


class Device : public StateObject
{
public:

    Device( const string name, const string serno, map<string, DigitalInput> dis,
		map<string, DigitalOutput> dos, map<string, AnalogueInput> ais, map<string,
		AnalogueOutput> aos):StateObject(name), m_serno(serno), m_dis(dis), m_dos(dos),
        m_ais(ais), m_aos(aos)     {}; 
	virtual ~Device() {};
	bool Ready() const;
	int ErrorStatus() const;
	int WarningStatus() const;
	virtual int DoProcessCallBacks();  // called when data changes
	virtual int DoProcessTimeouts();   // called periodically to check completion motions
	virtual bool Update(); // returns false in case command is issued in invalid state 
protected:
	map<string, DigitalInput> m_dis;
	map<string, DigitalOutput> m_dos;
	map<string, AnalogueInput> m_ais;
	map<string, AnalogueOutput> m_aos;
	string m_serno;
};

class Valve : public Device // a binary motion device with 1 or 2 commands, 
//1 or 2 sensors and some external material flow or pressure constraints
{
public:
	Valve( Device& baseDevice): Device(baseDevice), m_motionStartTime(0.),
	m_motionTimeOut(DEMO_TIMEOUT) {};
	virtual ~Valve() {};
	// these must be over-ridden depending on number of commands and sensors
	virtual bool InMotion()= 0;  // position sensor(s) do not match asserted command(s)
	virtual bool IsOpened() = 0; // opened position is reported
	virtual bool IsClosed() = 0; // closed position is reported
	virtual bool Close() = 0;  // returns true if interlock was satisfied and command was issued to hardware
	virtual bool Open() = 0; // returns true if interlock was satisfied and command was issued
	virtual double MotionStartTime(void) { return m_motionStartTime;};
	virtual int DoProcessCallBacks();  // called when data changes
	virtual int DoProcessTimeouts();   // called periodically to check completion of requested motions
	virtual bool Update();
	
protected:	
	double m_motionStartTime;
	double m_motionTimeOut;
	double m_waitStartTime;
	virtual void  IdleOutput();  // turn off outputs in case of motion timeout 
    virtual bool InvalidSensorState() {return false;}  //true if hardware sets conflicting outputs
};

class SingleThrowValve : public Valve // a single output actuator
{
public:
	SingleThrowValve(Valve& baseValve ): Valve(baseValve) {} ;
	virtual ~SingleThrowValve() {};
	bool InMotion();
	bool IsOpened(); 
	bool IsClosed();
	virtual bool Close();
	virtual bool Open();
protected:
    virtual void  IdleOutput();  // turn off outputs in case of motion timeout

};

class DoubleThrowValve : public Valve  // like a slot valve or a gate valve
{
public:
	DoubleThrowValve (Valve& baseValve): Valve(baseValve) {};
	virtual ~DoubleThrowValve() {};
	bool InMotion();
	bool IsOpened(); 
	bool IsClosed();
	virtual bool Close(); 
	virtual bool Open();
private:
	bool InvalidSensorState();
protected:
    virtual void  IdleOutput();  // turn off outputs in case of motion timeout
};
