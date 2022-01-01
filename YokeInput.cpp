#include <windows.h>
#include <stdio.h>
#include "MSFS_var.h"
#ifndef NOSIM
#include "SimConnect.h"
#endif


void ClearScreen()
{
  static int first = 1;
  HANDLE                     hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD                      count;
  DWORD                      cellCount;
  COORD                      homeCoords = { 0, 0 };

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  if (first) {
  /* Get the number of cells in the current buffer */
  if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
  cellCount = csbi.dwSize.X *csbi.dwSize.Y;

  /* Fill the entire buffer with spaces */
  if (!FillConsoleOutputCharacter(
    hStdOut,
    (TCHAR) ' ',
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Fill the entire buffer with the current colors and attributes */
  if (!FillConsoleOutputAttribute(
    hStdOut,
    csbi.wAttributes,
    cellCount,
    homeCoords,
    &count
    )) return;
	
    first = 0;
  }
  
  /* Move the cursor home */
  SetConsoleCursorPosition( hStdOut, homeCoords );
  }


void debug() {
	ClearScreen();
	
	printf ("Pitch = %f", yc[pitch]);
	printf ("     Roll = %f               \n", yc[roll]);

	printf ("Throttle = %f", yc[throttle1]);
	printf ("     Mixture = %f", yc[mixture1]);
	printf ("     Propeller = %f                \n", yc[propeller1]);
	
	printf ("Flaps = %f", yc[flaps]);
	printf ("     Trim = %f             \n", yc[pitch_trim]);

	if (yc[engine_starter1]) printf ("Engine on                        \n");
	if (yc[landing_gear]) printf ("Landing gear                 \n");
	if (yc[water_rudder]) printf ("Water rudder                 \n");
	if (yc[parking_brake]) printf ("Parking brake                  \n");
	if (yc[misc]) printf ("Misc                        \n");

	Sleep (10);
}



#ifndef NOSIM
void UpdateControls();
#endif

DWORD serial_task (void *);

int main(void)
{
#ifdef NOSIM
    CreateThread(0, 0, serial_task, 0, 0, NULL);
    while (1) debug();
#else
    CreateThread(0, 0, serial_task, 0, 0, NULL); // Serial receiver. Updates extern variables (yoke_X_position, ...)
    UpdateControls();
	while (1) {Sleep(1000);}
#endif
    return 0;
}
