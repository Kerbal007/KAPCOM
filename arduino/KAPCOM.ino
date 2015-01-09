#define ANALOG 1
#define DIGITAL 0
#define NULL 0

#include <new.cpp>
#include <iterator>
#include <vector>

#include <LedControl.h>

#include <pin.h>
#include <lockedinput.h>
#include <joy.h>
#include <display.h>
#include <ArduinoJson.h>

using namespace std;

// Global Serial Reading
String readLine = "";
String nextLine = "";

// Create calibration inputs
LockedInput calibrate("Calibrate", "calibrate", A11, A12, A13);

// Create global LedControl
int display_count = 5;
LedControl lc(7, 5, 6, display_count);

// Create vectors and iterators
vector<Joy> joys;
vector<Joy>::iterator joy;

vector<Pin> buttons;
vector<Pin>::iterator button;

vector<Pin> indicators;
vector<Pin>::iterator indicator;
   
vector<LockedInput> locks;
vector<LockedInput>::iterator lock;
/*
vector<Bargraph> bargraphs;
vector<Bargraph>::const_iterator bargraph;

vector<Display> displays;
vector<Display>::const_iterator display;
*/

void(* reset) (void) = 0;

void serialEvent() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    
    if (c == '\n' || c == 10 || c == 13) {
      readLine = nextLine;
      nextLine = "";
      
      if (readLine == "RESTART") {
        Serial.println("Resetting...");
        delay(1000); 
        reset();
      }
    } else {
      nextLine += c;
    }
  }
}

void setup() {
  // Initialize displays
  for(int i=0; i<display_count; i++) {
    lc.shutdown(i,false); // Enable display
    lc.setIntensity(i,15); // Set brightness level (0 is min, 15 is max)
    lc.clearDisplay(i);
  }
  
  joys.reserve(2);
  buttons.reserve(20);
  indicators.reserve(20);
  locks.reserve(2);
  /*
  bargraphs.reserve(5);
  displays.reserve(5);
  */
  
  // Load joysticks into vector
  joys.push_back(Joy("J0", A0, A1, A2));
  joys.push_back(Joy("J1", A4, A5, A6));
  
  // Load buttons into vector.*
  buttons.push_back(Pin("Soft", "?", A3, DIGITAL, INPUT_PULLUP));
  buttons.push_back(Pin("Trim", "?", A7, DIGITAL, INPUT_PULLUP));
   
  // Load locked inputs into vector
  locks.push_back(LockedInput("Stage", "stage", A11, A12, A13));
  locks.push_back(LockedInput("Abort", "abort", A8, A9, A10));
  /*
  displays.push_back(Display("Ap", "vessel_apoapsis", lc, 8, 1));
  displays.push_back(Display("Pe", "vessel_periapsis", lc, 8, 2));
  displays.push_back(Display("Alt", "vessel_altitude", lc, 8, 3));
  displays.push_back(Display("Vel", "vessel_velocity", lc, 8, 4));
  displays.push_back(Display("Rad", "vessel_asl_height", lc, 4, 5, 0, 3, " "));
  displays.push_back(Display("Inc", "vessel_inclination", lc, 4, 5, 4, 3, " "));
  */
  // Start the serial connection
  Serial.begin(9600);
  // Report online status
  Serial.println("ONLINE");
  // Receive configuration
  // configure();

  // Report calibration status
  Serial.println("CALIBRATING");

  // Calibrate until the staging button is pressed
  calibrate.indicator.set(HIGH);
  
  for (joy=joys.begin(); joy!=joys.end(); joy++) {
    joy->recalibrate();
    //joy->print();
  }

  while (calibrate.button.get() == LOW) {
    for(joy=joys.begin(); joy!=joys.end(); joy++) {
      joy->calibrate();
    }
  }

  calibrate.indicator.set(LOW);

  // Report ready status
  Serial.println("READY");
}

void configure() {
  StaticJsonBuffer<512> jsonBuffer;
  char* json; // = readLine.c_str();

  // Parse JSON
JsonObject& configuration = jsonBuffer.parseObject(json);
  if (!configuration.success()) {
    Serial.println("JSON Parsing failed.");
    reset();
  }

}

void poll() {
  // Poll all configured hardware 
  
  for (joy=joys.begin(); joy!=joys.end(); joy++) {
    joy->update();
  }
  
  for (button=buttons.begin(); button!=buttons.end(); button++) {
    button->update();
  }
   
  for (lock=locks.begin(); lock!=locks.end(); lock++) {
    lock->update();
  }
   
  /*
  for (bargraph=bargraphs.begin(); bargraph!=bargraphs.end(); bargraph++) {
    bargraph.update();
  }
  
  for (display=displays.begin(); display!=displays.end(); display++) {
    display->update();
  }
  */
}

void processInput() {
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& input = jsonBuffer.createObject();
  String yaw, pitch, roll, x, y, z, sixdof;
  
  // Aggregate fly-by-wire data
  
  for (joy=joys.begin(); joy!=joys.end(); joy++) {
    if (joy->name == "J0") {
      yaw = String(joy->X);
      pitch = String(joy->Y);
      roll = String(joy->Z);
    } else if (joy->name == "J1") {
      x = String(joy->X);
      y = String(joy->Y);
      z = String(joy->Z);
    }
  }
  sixdof=yaw + ", " + pitch + ", " + roll + ", " + x + ", " + y + ", " + z;
  input["toggle_fbw"] = "1";
  input["six_dof"] = sixdof.c_str();
  
  for (button=buttons.begin(); button!=buttons.end(); button++) {
   if (button->updated()) {
     input.add(button->api.c_str(), button->get());
   }
  }
  
  for (lock=locks.begin(); lock!=locks.end(); lock++) {
   if (lock->updated()) {
     input.add(lock->api.c_str(), lock->get());
   }
  }
  
  input.remove("?");
  
  // Send fly-by-wire data
  input.printTo(Serial);
  Serial.println("");
}

void processOutput() {
  StaticJsonBuffer<512> jsonBuffer;
  char* json; // = readLine;

  // Parse received telemetry
  JsonObject& output = jsonBuffer.parseObject(json);
  if (!output.success()) {
    Serial.println("JSON Parsing failed.");
    reset();
  }

  // Update instrumentation panels
  /*
  for (bargraph=bargraphs.begin(); bargraph!=bargraphs.end(); bargraph++) {
    bargraph.set(??);
  }
  */ /*
  for (display=displays.begin(); display!=displays.end(); display++) {
    display->set("1234.5678");
  }
  */
}



void loop() {
  delay(1000);
  
  //Serial.println(readLine);
  if (readLine != "") {
    // Wait for telemetry data
    //processOutput();
  }
  
  // Poll hardware for inputs
  poll();
  processInput();
  
  if (readLine != "") { 
    // Send fly-by-wire data
    processInput();
    
    readLine = "";
  }
 
}

