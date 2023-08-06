#pragma once

#include <stdbool.h>

#define MAX_PROFILE_COUNT   3

enum SpeedProfiles {
    eSPEED_PROF_LOAD,
    eSPEED_PROF_LOW,
    eSPEED_PROF_MEDIUM,
    eSPEED_PROF_FULL,
    eSPEED_PROF_FLANK
};

enum RunStates {
    eSTATE_STARTED,
    eSTATE_STOPPED,
    eSTATE_UNKNOWN
};

enum UnitTypes {
    eUNIT_MM = 0,
    eUNIT_CM,
    eUNIT_INCH,
    eUNIT_UNKNOWN
};

struct ProfileParams
{
    char diameter[32];
    enum UnitTypes unittype;
    int maxerpm;
    float crpmfactor;
    float colorfactor;
};
