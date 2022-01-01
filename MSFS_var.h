enum sim_var {
	roll, pitch, rudder,
	throttle1, throttle2,
	mixture1, mixture2,
	propeller1, propeller2,
	flaps,
	pitch_trim,
	landing_gear,
	water_rudder,
	last_var,

	brake_left, brake_right,
	
	misc,
	engine_starter1, engine_starter2,
	parking_brake,
	
	all_var,
};


typedef double t_structControl[all_var];

extern t_structControl yc;

