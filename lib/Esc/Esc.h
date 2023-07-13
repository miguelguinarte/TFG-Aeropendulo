/*
 * Electronic Speed Controller (ESC) - Library
 *
 *
*/

#ifndef ESC_H
#define ESC_H

#include "Arduino.h"
#include <ESP32Servo.h>		// Including the Servo library

#define MOTOR1_ESC_PIN          (GPIO_NUM_13) 
#define MOTOR2_ESC_PIN          (GPIO_NUM_12) 
#define MOTORS_MIN_SPEED        (1012) 
#define MOTORS_MAX_SPEED 		(1996)
#define MOTORS_MIN_SPEED_CTRL   (1200) 
#define MOTORS_MAX_SPEED_CTRL   (1600)
#define SPEED_CTRL              (1400)
#define MOTORS_ARM_SPEED        (500) 



class ESC
{
	public:
		ESC(byte ESC_pin, int outputMin = 1000, int outputMax = 2000, int armVal = 500);
		~ESC();
		void calib(void);
		void arm(void);
		void stop(void);
		void speed(int ESC_val);
		void setCalibrationDelay(uint32_t calibration_delay);
		void setStopPulse(uint32_t stop_pulse);

	private:
	// < Local attributes >
		// Hardware
		byte oPin;			// ESC output Pin

		// Calibration
		int oMin = 1000; 
		int oMax = 2000;
		int oESC = 1000;
		int oArm = 500;
		uint32_t calibrationDelay = 8000;	// Calibration delay (milisecond)
		uint32_t stopPulse = 500;	// Stop pulse (microseconds)
		Servo myESC;		// create servo object to control an ESC
};

#endif

/* Electronic Speed Controller (ESC) - Library */