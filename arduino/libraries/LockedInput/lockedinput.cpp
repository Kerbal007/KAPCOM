#define ANALOG 1
#define DIGITAL 0
#define NULL 0

#include <pin.h>
#include <lockedinput.h>

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// =================================
//	Constructors and Destructors
// =================================

LockedInput::LockedInput(String _name, String _api, int _lock, int _button, int _indicator, String _format) : lock(_name + " Locked", "", _lock, DIGITAL, INPUT_PULLUP), button(_name + " Button", "", _button, DIGITAL, INPUT_PULLUP), indicator(_name + " Indicator", "", _indicator, DIGITAL, OUTPUT) {
	// Set variables
	name		= _name;
	api			= _api;
	format		= _format;
	
	// Set default value, update from hardware, set initial value of previous
	update();
	last_value = value;
}

// ===================
//	Public Methods
// ===================

int LockedInput::get() {
	// Update the hardware and return the object value
	
	update();
	return value;
}

String LockedInput::toString() {
	if (format == "Value") {
		return String(value);
	}
	
	if (format == "Toggle") {
		if (value == 1) {
			return "None";
		} else {
			return "";
		}
	}
	
	if (format == "TrueFalse") {
		if (value == 0) {
			return "False";
		} else {
			return "True";
		}
	}
	
	if (format == "True") {
		if (value == 1) {
			return "True";
		} else {
			return "";
		}
	}
	
	if (format == "False") {
		if (value == 0) {
			return "False";
		} else {
			return "";
		}
	}
	
	Serial.println("Unexpected format \"" + format + "\" in object \"" + name + "\"");
	return "Error";
}

void LockedInput::update() {
	// Update the object value based on the hardware Pins
	lock.update();
	button.update();
	
	int isLocked = lock.get();
	indicator.set(isLocked);

	if (isLocked == LOW) {
		value = 0;
	} else {
		value = button.get();
	}
}

bool LockedInput::changed() {
	// Return true if the value has changed since last changed()
	bool is_updated = (last_value != value);
	last_value = value;
	return is_updated;
}

void LockedInput::print() {
	// Print the name of the pin and the value
	// Does not force a hardware refresh
	
	Serial.println(name + ": " + (String)value);
	lock.print();
	button.print();
	indicator.print();
}