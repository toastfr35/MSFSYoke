#include <windows.h>
#include <stdio.h>
#include "SimConnect.h"
#include "MSFS_var.h"

HANDLE  hSimConnect = NULL;

enum DATA_DEFINE_ID {DEF_SET, DEF_GET};
enum EVENT_ID {EVENT_PARKING_BRAKES, EVENT_ENGINE_START_ALL, EVENT_ENGINE_STOP_1, EVENT_ENGINE_STOP_2, EVENT_AXIS_LEFT_BRAKE_SET, EVENT_AXIS_RIGHT_BRAKE_SET, 
               EVENT_PAUSE_ON, EVENT_PAUSE_OFF, EVENT_SIM_RATE_INCR, EVENT_SIM_RATE_DECR};
enum DATA_REQUEST_ID {REQUEST};

t_structControl yc;
static t_structControl p_yc;


struct t_recv_data
{
    double parking_brake_position;
	double engine_rpm;
	double brake_left;
	double brake_right;
	double on_ground;
};
t_recv_data recv_data;


/****************************
 *
 ****************************/
static int yc_has_changed() 
{
	int i;	
	for (i=0; i<last_var; i++) {
		if (p_yc[i] = yc[i]) {return 1;}
	}
	return 0;
}

/****************************
 *
 ****************************/
static void open_simconnect()
{
    HRESULT hr;
	int i;
	int size = 0;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "YOKE Control", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Flight Simulator!");   
        // Set up a data definition for the plane controls

        hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "YOKE X POSITION", "percent"); //roll
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "YOKE Y POSITION", "percent"); //pitch
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "RUDDER POSITION", "percent"); //rudder
        
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent"); //throttle1
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent"); //throttle2
		
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG MIXTURE LEVER POSITION:1", "percent"); //mixture1
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG MIXTURE LEVER POSITION:2", "percent"); //mixture2

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG PROPELLER LEVER POSITION:1", "percent"); //propeller1
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GENERAL ENG PROPELLER LEVER POSITION:2", "percent"); //propeller2

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "FLAPS HANDLE INDEX", "number"); //flaps

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "ELEVATOR TRIM POSITION", "percent"); //pitch_trim

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "GEAR HANDLE POSITION", "number"); //landing_gear

		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_SET, "WATER RUDDER HANDLE POSITION", "number"); //water_rudder

		// Queries
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "BRAKE PARKING POSITION", "number");
		//hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "GENERAL ENG RPM:1", "rpm");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "GENERAL ENG COMBUSTION:1", "bool");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "BRAKE LEFT POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "BRAKE RIGHT POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, DEF_GET, "SIM ON GROUND", "bool");


        // Events
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PARKING_BRAKES, "PARKING_BRAKES");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ENGINE_START_ALL, "ENGINE_AUTO_START");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ENGINE_STOP_1, "MAGNETO1_OFF");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ENGINE_STOP_2, "MAGNETO2_OFF");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_AXIS_LEFT_BRAKE_SET, "AXIS_LEFT_BRAKE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_AXIS_RIGHT_BRAKE_SET, "AXIS_RIGHT_BRAKE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_AXIS_RIGHT_BRAKE_SET, "AXIS_RIGHT_BRAKE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PAUSE_ON, "PAUSE_ON");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PAUSE_OFF, "PAUSE_OFF");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_SIM_RATE_INCR, "SIM_RATE_INCR");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_SIM_RATE_DECR, "SIM_RATE_DECR");

	} else {		
		printf ("Error: cound not connect\n");
		exit (1);		
	}			
}


/****************************
 *
 ****************************/
static void send_event(int event, DWORD dwEventData)
{
	SIMCONNECT_OBJECT_ID objectID = SIMCONNECT_OBJECT_ID_USER;
	SimConnect_TransmitClientEvent(	hSimConnect,
									objectID,
									event, // event ID from your enumeration
									dwEventData, // event DATA
									SIMCONNECT_GROUP_PRIORITY_HIGHEST, // required parameter
									SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY); // required
}




/****************************
 *
 ****************************/
unsigned int count = 0;
void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	//printf ("answer %d\n", pData->dwID);
    if (pData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE) {
        SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;
        if (pObjData->dwRequestID == REQUEST) {
            DWORD ObjectID = pObjData->dwObjectID;
            t_recv_data *val = (t_recv_data*)&pObjData->dwData;
			memcpy (&recv_data, val, sizeof (t_recv_data));
			//printf (">>recv_data.parking_brake_position = %f\n", recv_data.parking_brake_position);
			//printf (">>recv_data.engine_rpm = %f\n", recv_data.engine_rpm);
			//printf (">>recv_data.brake_left = %f        ", recv_data.brake_left);
			//printf ("recv_data.brake_right = %f\n", recv_data.brake_right);
			//printf (">>recv_data.on_ground = %f\n", recv_data.on_ground);
			//printf ("."); fflush (stdout);
			SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST, DEF_GET, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
			if(!(count%10)) printf ("*");
			count++;
			return;
        }
    }
	printf ("?");
}


/****************************
 *
 ****************************/
static void get_info()
{
	HRESULT hr;
	static int first = 1;
	if (first) {
		hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
		hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
		hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
		hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
		SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST, DEF_GET, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
		first = 0;
	}
	//Sleep(100);
	//printf ("A"); fflush (stdout);
	SwitchToThread();
	hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
	//printf ("B"); fflush (stdout);
	if (hr == E_FAIL) printf ("!"); fflush (stdout);
}


/****************************
 *
 ****************************/
static DWORD get_info_task(void *)
{
	while(1) {get_info();}
}


/****************************
 *
 ****************************/
static DWORD update_task(void *)
{
    HRESULT hr;
	int i;
	int skip_events = 0;
	int size = sizeof(double) * last_var;
	int freq = 100;
	int fast_sim = 0;
	float restore_roll;

    while(1) {
		get_info();
		if (yc_has_changed) {
			// update previous yc;
			for (i=0; i<last_var; i++) {p_yc[i] = yc[i];}

			if (recv_data.on_ground) {
				restore_roll = yc[roll];
				yc[rudder] = yc[roll];
				yc[roll] = 0.0;
			} else {
				restore_roll = 0.0;
				yc[rudder] = 0.0;
			}

			hr = SimConnect_SetDataOnSimObject (hSimConnect, DEF_SET, SIMCONNECT_OBJECT_ID_USER, 0, 0, size, (void*)yc);

			if (restore_roll) yc[roll] = restore_roll;

			if (skip_events) {
				skip_events--;
			} else {
				
				if (yc[parking_brake] != recv_data.parking_brake_position) {
					// Toggle parking brake
					//printf ("Toggle parking brake to be %f\n", yc[parking_brake]);
					send_event(EVENT_PARKING_BRAKES, 0);
					skip_events=freq;
				}

				if (yc[engine_starter1] && !recv_data.engine_rpm) {
					// Switch engine on
					//printf ("Engine on\n");
					send_event(EVENT_ENGINE_START_ALL, 0);
					skip_events=freq;
				}

				if (!yc[engine_starter1] && recv_data.engine_rpm) {
					// Switch engine off
					//printf ("Engine off\n");
					send_event(EVENT_ENGINE_STOP_1, 0);
					send_event(EVENT_ENGINE_STOP_2, 0);
					skip_events=freq;
				}

				if ( (yc[brake_left] == 1.0) && (recv_data.brake_left == 0.0) ) {
					// Brakes are off
				} else if (yc[parking_brake]) {
					// Parking brake is on, no need to apply brakes
				} else {
					// Apply brakes
					int val = (int)(32766 * (1.0-yc[brake_left])) - 16383;
					//printf ("Brake on %f -> %d\n", yc[brake_left], val);
					send_event(EVENT_AXIS_LEFT_BRAKE_SET, val);
					send_event(EVENT_AXIS_RIGHT_BRAKE_SET, val);
					skip_events=freq;
				}

				if (yc[misc]) {
					if (!fast_sim) {
						printf ("Faster\n");
						fast_sim = 1;
						send_event(EVENT_SIM_RATE_INCR, 0);
						send_event(EVENT_SIM_RATE_INCR, 0);
						send_event(EVENT_SIM_RATE_INCR, 0);
						send_event(EVENT_SIM_RATE_INCR, 0);
						send_event(EVENT_SIM_RATE_INCR, 0);
						send_event(EVENT_SIM_RATE_INCR, 0);
					}
				} else {
					if (fast_sim) {
						printf ("Slower\n");
						fast_sim = 0;
						send_event(EVENT_SIM_RATE_DECR, 0);
						send_event(EVENT_SIM_RATE_DECR, 0);
						send_event(EVENT_SIM_RATE_DECR, 0);
						send_event(EVENT_SIM_RATE_DECR, 0);
						send_event(EVENT_SIM_RATE_DECR, 0);
						send_event(EVENT_SIM_RATE_DECR, 0);
					}
				}


			}
        } 
		Sleep(1000/freq);
    }
}


/****************************
 *
 ****************************/
void UpdateControls()
{
	open_simconnect();

	CreateThread(0, 0, update_task, 0, 0, NULL); 
	//CreateThread(0, 0, get_info_task, 0, 0, NULL); 
}