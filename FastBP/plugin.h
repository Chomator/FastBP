#pragma once

#include "pluginmain.h"


//plugin data
#define PLUGIN_NAME "FastBP"
#define PLUGIN_VERSION 1

struct API
{
    std::string apiName;     // API function name or expression to evaluate
    std::string bpName;      // Display name in menu
    std::string category;    // Category for menu grouping
    std::string description; // Description of what the API does
    bool enabled = false;
};

enum MenuEntries
{
    MENU_SET_ALL_BASE = 10000,    // Base for set all menu entries
    MENU_CLEAR_ALL_BASE = 50000,  // Base for clear all menu entries
    MENU_CLEAR_ALL = 90000,       // Add this line for the Clear All BPs option
    MENU_RELOAD = 99999,
    MENU_ABOUT = 100000
};

// Structure to store ALL tag information
struct AllTagInfo
{
    std::string category;
    std::vector<std::string> apiNames;
    int menuId;
};

// Add this as a global variable
static std::vector<AllTagInfo> allTags;
static std::vector<API> apis;
static wchar_t apiFile[MAX_PATH];

//functions
bool pluginInit(PLUG_INITSTRUCT* initStruct);
bool pluginStop();
void pluginSetup();

