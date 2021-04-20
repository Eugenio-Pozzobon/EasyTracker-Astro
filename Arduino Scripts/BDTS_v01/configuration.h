//Define Variables

// Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
// Find yours here: http://www.magnetic-declination.com/
float declination = 0; //-39; //degree
#define gy_hz 5
#define MPU_FILTER

#define CALIBRATING_TIME 30 //seconds
#define G_GAIN 0.00875
#define AA 0.5


//Define code operations

//#define BLUETOOTH
//#define bt_hz 5
#define DEBUGSENSOR
//#define DEBUGTIMER
