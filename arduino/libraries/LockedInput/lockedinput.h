#ifndef LockedInput_h
#define LockedInput_h

#include <pin.h>
#include <pinio.h>

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class LockedInput {
  public :
    String name;
	String api;
    Pin lock;
    Pin button;
	Pin indicator;
	int value;
	int last;
    
    LockedInput(String _name, String _api, int _lock, int _button, int _indicator);
    
	int get();
    void update();
	void updated();
    void print();
};

#endif