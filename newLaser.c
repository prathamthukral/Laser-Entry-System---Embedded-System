#include "gpiolib_addr.h"
#include "gpiolib_reg.h"
#include "gpiolib_reg.c"
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <math.h>

extern int errno;
// public variables
FILE *logFile;
FILE *statsFile;

void readConfig(FILE *configFile, int *timeout, char *logFileName, char *statsFileName, int *statsInterval)
{
	//Loop counter
	int i = 0;

	//A char array to act as a buffer for the file
	char buffer[255];

	//The value of the timeout and statsInterval variable is set to zero at the start
	*timeout = 0;
	*statsInterval = 0;

	//This is a variable used to track which input we are currently looking
	//for (configfile, timeout, or logfile)
	int input = 0;

	//This will
	//fgets(buffer, 255, configFile);
	//This will check that the file can still be read from and if it can,
	//then the loop will check to see if the line may have any useful
	//information.
	while (fgets(buffer, 255, configFile) != NULL)
	{
		i = 0;
		//If the starting character of the string is a '#',
		//then we can ignore that line
		if (buffer[i] != '#')
		{
			while (buffer[i] != 0)
			{
				//This if will check the value of timeout
				if (buffer[i] == '=' && input == 0)
				{
					//The loop runs while the character is not null
					while (buffer[i] != 0)
					{
						//If the character is a number from 0 to 9
						if (buffer[i] >= '0' && buffer[i] <= '9')
						{
							//Move the previous digits up one position and add the
							//new digit
							*timeout = (*timeout * 10) + (buffer[i] - '0');
						}
						i++;
					}
					input++;
				}
				else if (buffer[i] == '=' && input == 1) //This will find the name of the log file
				{
					int j = 0;
					//Loop runs while the character is not a newline or null
					while (buffer[i] != 0 && buffer[i] != '\n')
					{
						//If the characters after the equal sign are not spaces or
						//equal signs, then it will add that character to the string
						if (buffer[i] != ' ' && buffer[i] != '=')
						{
							logFileName[j] = buffer[i];
							j++;
						}
						i++;
					}
					//Add a null terminator at the end
					logFileName[j] = 0;
					input++;
				}
				else if (buffer[i] == '=' && input == 2) //This will find the name of the stats file
				{
					int j = 0;
					//Loop runs while the character is not a newline or null
					while (buffer[i] != 0 && buffer[i] != '\n')
					{
						//If the characters after the equal sign are not spaces or
						//equal signs, then it will add that character to the string
						if (buffer[i] != ' ' && buffer[i] != '=')
						{
							statsFileName[j] = buffer[i];
							j++;
						}
						i++;
					}
					//Add a null terminator at the end
					statsFileName[j] = 0;
					input++;
				}
				else if (buffer[i] == '=' && input == 3)
				{
					//The loop runs while the character is not null
					while (buffer[i] != 0)
					{
						//If the character is a number from 0 to 9
						if (buffer[i] >= '0' && buffer[i] <= '9')
						{
							//Move the previous digits up one position and add the
							//new digit
							*statsInterval = (*statsInterval * 10) + (buffer[i] - '0');
						}
						i++;
					}
					input++;
				}
				else
				{
					i++;
				}
			}
		}
	}
}

//This function will get the current time using the gettimeofday function
void getTime(char *buffer)
{
	//Create a timeval struct named tv
	struct timeval tv;

	//Create a time_t variable named curtime
	time_t curtime;

	//Get the current time and store it in the tv struct
	gettimeofday(&tv, NULL);

	//Set curtime to be equal to the number of seconds in tv
	curtime = tv.tv_sec;

	//This will set buffer to be equal to a string that in
	//equivalent to the current date, in a month, day, year and
	//the current time in 24 hour notation.
	strftime(buffer, 30, "%m-%d-%Y  %T.", localtime(&curtime));
}

void setToOutput(GPIO_Handle gpio, int pinNumber)
{
	//Check that the gpio is functional
	if (gpio == NULL)
	{
		printf("The GPIO has not been intitialized properly \n");
		return;
	}

	//Check that we are trying to set a valid pin number
	if (pinNumber < 2 || pinNumber > 27)
	{
		printf("Not a valid pinNumer \n");
		return;
	}

	//This will create a variable that has the appropriate select
	//register number. For more information about the registers
	//look up BCM 2835.
	int registerNum = pinNumber / 10;

	//This will create a variable that is the appropriate amount that
	//the 1 will need to be shifted by to set the pin to be an output
	int bitShift = (pinNumber % 10) * 3;

	//This is the same code that was used in Lab 2, except that it uses
	//variables for the register number and the bit shift
	uint32_t sel_reg = gpiolib_read_reg(gpio, GPFSEL(registerNum));
	sel_reg |= 1 << bitShift;
	gpiolib_write_reg(gpio, GPFSEL(1), sel_reg);
}

/* You may want to create helper functions for the Hardware Dependent functions*/

//This function should initialize the GPIO pins
//laser1Count will be how many times laser 1 is broken (the left laser).
//laser2Count will be how many times laser 2 is broken (the right laser).
//numberIn will be the number  of objects that moved into the room.
//numberOut will be the number of objects that moved out of the room.

void errorMessage(int errorCode)
{
	fprintf(logFile, "An error occured; the error code was %d \n", errorCode);
}

//This function should accept the diode number (1 or 2) and output
//a 0 if the laser beam is not reaching the diode, a 1 if the laser
//beam is reaching the diode or -1 if an error occurs.
int laserDiodeStatus(GPIO_Handle gpio, int diodeNumber)
{
	if (gpio == NULL)
	{
		errorMessage(1);
		return -1;
	}

	uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));
	if (diodeNumber == 1)
	{
		if (level_reg & (1 << 13))
			return 1;
		else
			return 0;
	}
	else
	{
		if (level_reg & (1 << 18))
			return 1;
		else
			return 0;
	}
}

//as what was done in Lab 2 to make the pin output 3.3V
void outputOn(GPIO_Handle gpio, int pinNumber)
{
	gpiolib_write_reg(gpio, GPSET(0), 1 << pinNumber);
}

//This function will make an output pin turn off. It is the same
//as what was done in Lab 2 to make the pin turn off
void outputOff(GPIO_Handle gpio, int pinNumber)
{
	gpiolib_write_reg(gpio, GPCLR(0), 1 << pinNumber);
}

GPIO_Handle initializeGPIO()
{
	//This is the same initialization that was done in Lab 2
	GPIO_Handle gpio;
	gpio = gpiolib_init_gpio();
	if (gpio == NULL)
	{
		perror("Could not initialize GPIO");
	}
	return gpio;
}

// output functions:
#define PRINT_MSG(file, time, programName, str)                \
	do                                                         \
	{                                                          \
		fprintf(file, "%s : %s : %s", time, programName, str); \
		fflush(file);                                          \
	} while (0)

#define OUTPUT_MSG(file, laser1Count, laser2Count, numberIn, numberOut)                                                                                                                                                            \
	do                                                                                                                                                                                                                             \
	{                                                                                                                                                                                                                              \
		fprintf(file, "Laser 1 was broken %d times \nLaser 2 was broken %d times \n%d objects entered the room \n%d objects exitted the room\n________________________________\n", laser1Count, laser2Count, numberIn, numberOut); \
		fflush(file);                                                                                                                                                                                                              \
	} while (0)

void dbg(int i)
{
	printf("\nTEST: %d\n", i);
}

// main function
int main(const int argc, const char *const argv[])
{
	GPIO_Handle gpio = initializeGPIO();

	// initial values
	int laser1Count = 0;
	int laser2Count = 0;
	int numberIn = 0;
	int numberOut = 0;

	enum states
	{
		START,
		INCOMING,
		INTERMEDIATE,
		INCOMING_PLUS,
		OUTGOING,
		OUTGOING_PLUS,
		RIGHT_END
	};
	enum states s = START;

	time_t startTime = time(NULL);
	//Create a char array that will be used to hold the time values
	char time[30];
	//Create a string that contains the program name
	const char *argName = argv[0];

	//These variables will be used to count how long the name of the program is
	int i = 0;
	int namelength = 0;

	while (argName[i] != 0)
	{
		namelength++;
		i++;
	}

	char programName[namelength];

	i = 0;
	//Copy the name of the program without the ./ at the start
	//of argv[0]
	while (argName[i + 2] != 0)
	{
		programName[i] = argName[i + 2];
		i++;
	}

	//Create a file pointer named configFile
	FILE *configFile;
	//Set configFile to point to the newLaser.cfg file. It is
	//set to read the file.
	configFile = fopen("/home/pi/newLaser.cfg", "r");
	//Output a warning message if the file cannot be openned
	if (!configFile)
	{
		perror("The config file could not be opened");
		return -1;
	}

	//Declare the variables that will be passed to the readConfig function
	int timeout;
	char logFileName[80] = "/home/pi/logFile.log";
	char statsFileName[80] = "/home/pi/statsFile.log";
	int statsInterval;

	//Call the readConfig function to read from the config file
	readConfig(configFile, &timeout, logFileName, statsFileName, &statsInterval);

	//Close the configFile now that we have finished reading from it
	fclose(configFile);

	//Create a new file pointer to point to the log file
	//Set it to point to the file from the config file and make it append to
	//the file when it writes to it.
	logFile = fopen(logFileName, "w");
	statsFile = fopen(statsFileName, "w");

	//This line uses the ioctl function to set the time limit of the watchdog
	//timer to 15 seconds. The time limit can not be set higher that 15 seconds
	//so please make a note of that when creating your own programs.
	//If we try to set it to any value greater than 15, then it will reject that
	//value and continue to use the previously set time limit
	const int DEFAULT_TIMEOUT = 3;
	if (timeout > 15)
	{
		timeout = DEFAULT_TIMEOUT;
	}

	//This variable will be used to access the /dev/watchdog file, similar to how
	//the GPIO_Handle works
	int watchdog;

	//We use the open function here to open the /dev/watchdog file. If it does
	//not open, then we output an error message. We do not use fopen() because we
	//do not want to create a file if it doesn't exist
	if ((watchdog = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0)
	{
		fprintf(logFile, "Error: Couldn't open watchdog device! %d\n", watchdog);
		return -1;
	}

	//Get the current time
	getTime(time);
	//Log that the watchdog file has been opened
	PRINT_MSG(logFile, time, programName, "The Watchdog file has been opened\n\n");

	ioctl(watchdog, WDIOC_SETTIMEOUT, &timeout);
	//Get the current time
	getTime(time);
	//Log that the Watchdog time limit has been set
	PRINT_MSG(logFile, time, programName, "The Watchdog time limit has been set\n\n");

	//The value of timeout will be changed to whatever the current time limit of the
	//watchdog timer is
	ioctl(watchdog, WDIOC_GETTIMEOUT, &timeout);

	//This print statement will confirm to us if the time limit has been properly
	//changed. The \n will create a newline character similar to what endl does.

	fprintf(logFile, "The watchdog timeout is %d seconds.\n\n", timeout);
	int count1 = 0;
	int count2 = 0;
	while (1)
	{
		usleep(100);

		switch (s)
		{
		case START:
			if (!laserDiodeStatus(gpio, 1))
			{
				s = INCOMING;
				laser1Count++;
			}
			else if (!laserDiodeStatus(gpio, 2))
			{
				s = OUTGOING;
				laser2Count++;
			}
			break;
		case INCOMING:
			if (laserDiodeStatus(gpio, 1))
				s = START;
			else if (!laserDiodeStatus(gpio, 2))
			{
				s = INTERMEDIATE;
				laser2Count++;
			}
			break;
		case RIGHT_END:
			if (!laserDiodeStatus(gpio, 2))
			{
				s = OUTGOING_PLUS;
				laser2Count++;
			}
			else if (laserDiodeStatus(gpio, 1))
			{
				s = START;
				numberOut++;
			}
			break;
		case INTERMEDIATE:
			if (laserDiodeStatus(gpio, 1))
				s = INCOMING_PLUS;
			else if (laserDiodeStatus(gpio, 2))
				INCOMING;
			break;
		case INCOMING_PLUS:
			if (!laserDiodeStatus(gpio, 1))
			{
				s = INTERMEDIATE;
				laser1Count++;
			}
			else if (laserDiodeStatus(gpio, 2))
			{
				s = START;
				numberIn++;
			}
			break;
		case OUTGOING_PLUS:
			if (laserDiodeStatus(gpio, 2))
				s = RIGHT_END;
			else if (laserDiodeStatus(gpio, 1))
				s = OUTGOING;
			break;
		case OUTGOING:
			if (laserDiodeStatus(gpio, 2))
				s = START;
			else if (!laserDiodeStatus(gpio, 1))
			{
				s = OUTGOING_PLUS;
				laser1Count++;
			}
			break;
		default:
			printf("Default");
			break;
		}

		unsigned int numSecondsCountDog = count1 * 100;
		unsigned int numSecondsCountStats = count2 * 100;
		unsigned int timeStatsInterval = statsInterval * pow(10, 6);
		unsigned int timeWatchdogInterval = (timeout - 1) * pow(10, 6); // off by 1 so that the watchdog is kicked 1 sec before the timeout
		count1++;
		count2++;

		ioctl(watchdog, WDIOC_KEEPALIVE, 0);
		// output msg every minute
		if (numSecondsCountDog >= timeWatchdogInterval)
		{
			count1 = 0;
			ioctl(watchdog, WDIOC_KEEPALIVE, 0);
			getTime(time);
			//Log that the Watchdog was kicked
			PRINT_MSG(logFile, time, programName, "The Watchdog was kicked\n\n");
			OUTPUT_MSG(statsFile, laser1Count, laser2Count, numberIn, numberOut);
		}

		if (numSecondsCountStats >= timeStatsInterval)
		{
			count2 = 0;
			OUTPUT_MSG(statsFile, laser1Count, laser2Count, numberIn, numberOut);
		}
	}

	//Writing a V to the watchdog file will disable to watchdog and prevent it from
	//resetting the system
	write(watchdog, "V", 1);
	getTime(time);
	//Log that the Watchdog was disabled
	PRINT_MSG(logFile, time, programName, "The Watchdog was disabled\n\n");

	// Close the watchdog file so that it is not accidentally tampered with
	close(watchdog);
	getTime(time);
	//Log that the Watchdog was closed
	PRINT_MSG(logFile, time, programName, "The Watchdog was closed\n\n");

	//Free the gpio pins
	gpiolib_free_gpio(gpio);
	getTime(time);
	//Log that the GPIO pins were freed
	PRINT_MSG(logFile, time, programName, "The GPIO pins have been freed\n\n");

	//Return to end the program
	return 0;
}
