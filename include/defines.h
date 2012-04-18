/**
 * defines.h
 * 
 * This file contains some const variables.
 */

#ifndef DEFINES_H
#define DEFINES_H

#define VATSINATOR_VERSION	"0.1_alpha1"

// where to get all the data from
#define VATSIM_STATUS_URL	"http://status.vatsim.net/status.txt"
// if on mingw there is no PREFIX
#ifdef PREFIX
#define AIRPORTS_DB		PREFIX "/share/vatsinator/WorldAirports.db"
#define FIRS_DB			PREFIX "/share/vatsinator/WorldFirs.db"
#define VATSINATOR_DAT		PREFIX "/share/vatsinator/vatsinator.dat"
#else
#define AIRPORTS_DB		"WorldAirports.db"
#define FIRS_DB			"WorldFirs.db"
#define VATSINATOR_DAT		"vatsinator.dat"
#endif

// below/above these values user can't zoom the map
#define ZOOM_MINIMUM		3

// the range below which the mouse is in the object's
#define OBJECT_TO_MOUSE		0.03

// how far from the airport the pilot must be to be recognized as "departing"
// or "arrived"
#define PILOT_TO_AIRPORT	0.1

// how to recognize unavailable metars
#define METAR_NO_AVAIL		"No METAR available"

// for these values the GL's context rendering size reprezents
// (-1.0; 1.0) orthogonal ranges for both x and y axes
#define BASE_SIZE_WIDTH		800
#define BASE_SIZE_HEIGHT	600

/* Default values */

// Vatsim data refresh rate in minutes
#define REFRESH_RATE		3

// view opts
#define PILOTS_CHECKBOX		Qt::Checked
#define AIRPORTS_CHECKBOX	Qt::Checked
#define FIRS_CHECKBOX		Qt::Checked
#define UIRS_CHECKBOX		Qt::Checked
#define PILOTS_LABELS		(WHEN_HOVERED | AIRPORT_RELATED)
#define DISPLAY_AIRPORT_BOX	1

// colors
#define PILOTS_LABELS_COLOR		0, 0, 0
#define AIRPORTS_LABELS_COLOR		127, 127, 127
#define UNSTAFFED_FIR_BORDERS_COLOR	127, 127, 127
#define STAFFED_FIR_BORDERS_COLOR	176, 32, 32
#define STAFFED_UIR_BORDERS_COLOR	140, 219, 255
#define APPROACH_CIRCLE_COLOR		127, 0, 0
#define BACKGROUND_COLOR		255, 255, 255

#define LINES_COLOR		0.0, 0.192, 0.337, 0.8



#ifndef NO_DEBUG
#define DEBUG_NEW new(__FILE__, __LINE__)
#include "../include/MemoryTracker.h"
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

#endif // DEFINES_H