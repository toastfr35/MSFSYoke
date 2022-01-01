#include <windows.h>
#include "stdio.h"
#include "math.h"
#include "MSFS_var.h"

#ifdef NOSIM
t_structControl yc;
#endif

#define DATA_LENGTH 16

typedef struct t_filter_data {
	double data[DATA_LENGTH];
	unsigned int index;
} t_filter_data;

t_filter_data data_roll = {.index = 0};
t_filter_data data_pitch = {.index = 0};
t_filter_data data_throttle = {.index = 0};
t_filter_data data_mixture = {.index = 0};
t_filter_data data_propeller = {.index = 0};

double filter (t_filter_data * data, double value)
{
	int i;
	double total = 0;
	double v;
	//printf ("  %d     \n", data->index);
	data->data[data->index++] = value;
	if ((data->index) > DATA_LENGTH) data->index = 0;
	for (i=0; i<DATA_LENGTH; i++) total += data->data[i];
	v = total / DATA_LENGTH;
	return round(v);
}


void set_pitch (double v)
{
	yc[pitch] = filter(&data_pitch,v);
}

void set_roll (double v)
{
	yc[roll] = filter(&data_roll,v);
}

void set_trim (double v)
{
	yc[pitch_trim] = v/3;
}

void set_flaps (double v)
{
	yc[flaps] = v;
}

void set_propeller (double v)
{
	yc[propeller1] = filter(&data_propeller, 100.0 * v);
	yc[propeller2] = yc[propeller1];

	yc[brake_left]  = v;
	yc[brake_right] = yc[brake_left];
}

void set_mixture (double v)
{
	yc[mixture1] = filter(&data_mixture, 100.0 * v);
	yc[mixture2] = yc[mixture1];
}

void set_throttle (double v)
{
	yc[throttle1] = filter(&data_throttle, 100.0 * v);
	yc[throttle2] = yc[throttle1];
}

void set_switches (int v)
{
	// top row
	yc[landing_gear] = (v & 0x1)?1:0;
	yc[water_rudder] = (v & 0x2)?1:0;
	yc[parking_brake] = (v & 0x4)?1.0:0.0;

	// bottom row
	yc[engine_starter1] = (v & 0x10)?1:0;
	yc[engine_starter2] = (v & 0x10)?1:0;
	
	yc[misc] = (v & 0x20)?1:0;

}



/************************
 * Process one value
 ************************/
 static void process_value (char A, char B, double val)
 {
	switch (A) {
		case 'Y' : // Yoke
			switch (B) {
				case 'P' : 				
					set_pitch(val);
					break;
				case 'R' :
					set_roll (val);
					break;									
			}		
			break;
		
		case 'T' : // Trim
		    set_trim (val);
			break;
		
		case 'R' : // Flaps
		    set_flaps (val);
			break;
		
		case 'S' : // Switches
		    set_switches ((int) val);
			break;
		
		case 'P' : // Linear potentiometers
		    switch (B) {
				case 'R': 
					set_propeller (val);
					break;
				case 'C':
					set_mixture (val);
					break;
				case 'L' :
					set_throttle (val);
					break;
			}
			break;
		
		default:
			printf ("Error %c %c\n", A, B);
			exit(1);
	} 
 }


/**************/
/* Serial COM */
/**************/

HANDLE hSerial;
 
static void serial_init()
{
	// Declare variables and structures
	DCB dcb = {0};
	COMMTIMEOUTS timeouts = {0};

	// Open the highest available serial port number
	hSerial = CreateFile("\\\\.\\COM3", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hSerial == INVALID_HANDLE_VALUE) {printf ("Error: Could not open serial port COM3 for reading.\n"); exit (1);}

	// Set device parameters (1000000 baud, 1 start bit, 1 stop bit, no parity)
	dcb.DCBlength = sizeof(dcb);
	if (GetCommState(hSerial, &dcb) == 0) {CloseHandle(hSerial); printf ("Err2\n"); exit(1);}

	dcb.BaudRate = 1000000;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;

	if(SetCommState(hSerial, &dcb) == 0) {CloseHandle(hSerial); printf ("Err3\n"); exit(1);}

	// Set COM port timeout settings
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if(SetCommTimeouts(hSerial, &timeouts) == 0) {CloseHandle(hSerial); printf ("Err4\n"); exit(1);}
	
}

static void serial_receive_bytes (unsigned char * buf, unsigned int * size)
{
	DWORD received;
	ReadFile( hSerial ,  buf , *size, &received , NULL);
	*size = received;
}

static void serial_close()
{
	if (CloseHandle(hSerial) == 0) {exit(1);}
}



static char buf[256];
static unsigned int size = 0;
static unsigned int idx = 0;

static void get_next_buffer() 
{
	size=255;
	serial_receive_bytes ((unsigned char *)buf, &size);
	buf[size]=0;		
	idx = 0;
}

static char get_next_char() 
{
	while (idx == size) {
		get_next_buffer();
	}
	return buf[idx++];
}


/*************/
/* main loop */
/*************/

DWORD serial_task (void *) 
{
	char c;
	char A,B;
	char val[64];
	double valf;
	int i;
	
	serial_init();
	
    //skip some noise
    for (i=0;i<1000;i++) c = get_next_char();	

	while (1) {
		
		// Move to the next value
		while (c != ',') {c = get_next_char();}
		c = get_next_char();
		while ((c==10) || (c==13)) {c = get_next_char();}
		
		// Read the name
		A = c;
		B = get_next_char();
		
		// Read the '=' sign
		c = get_next_char();
		if (c != '=') {printf ("Error '%c' (%s)\n", c, buf);}
		
		// Read the value
		i = 0;
		while (1) {
			c = get_next_char();
            if (c != ',') {
				val[i++] = c;
		    } else {
				val[i] = 0;
				break;
			}
		}
		valf = atof (val);
		
		//printf ("%c %c = '%f'\n", A, B, valf);		
		process_value (A, B, valf);
	}
	return 0;
}
