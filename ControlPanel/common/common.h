#pragma once

#include <stdbool.h>

#define MAX_PRODUCT_PARAMS_COUNT        3
#define MAX_SPEED_PARAMS_COUNT          5
#define RPM_TO_VOLTAGEE_DIVIDE_FACTOR   150

#define SYSTEM_SETTINGS_FILE "./settings.bin"

#pragma pack(1)

enum SpeedProfiles {
    eSPEED_ONE = 0,
    eSPEED_TWO,
    eSPEED_THREE,
    eSPEED_FOUR,
    eSPEED_FIVE
};

enum RunStates {
    eSTATE_STARTED,
    eSTATE_STOPPED,
    eSTATE_UNKNOWN
};

enum ControlPanelScreens {
    eRUN_SCREEN = 0,
    eSETTINGS_SCREEN,
    eINPUT_TEXT_SCREEN,
    eUNKWON_SCREEN
};

struct SpeedParams
{
    int erpm;
    float crpm;
    float color;
};

struct ProductParams
{
    char name[32];
    SpeedParams params[MAX_SPEED_PARAMS_COUNT];
};

struct ControlPanelConfig
{
    bool link_state;
    int product_idx;
    int speed_idx;
    struct ProductParams m_products[MAX_PRODUCT_PARAMS_COUNT];
};
