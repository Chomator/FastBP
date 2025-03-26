//Original idea by https://github.com/x64dbg/SlothBP

#include "plugin.h"
#include "resource.h"
#include "strUtil.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;



static void ParseJsonNode(const json& node, const std::string& currentCategory, std::vector<API>& apiList);
static bool LoadApis(const wchar_t* apiFile);
static bool SetupMenus();
static std::vector<size_t> FindApisByName(const std::vector<std::string>& apiNames, const std::string& category);
static void SetMultipleBreakpoints(const std::vector<size_t>& apiIndices, bool setBreakpoints);
bool ReloadConfig();
static void refreshStatus(CBTYPE type, const char* modulename);


// Recursively parse JSON structure
static void ParseJsonNode(const json& node, const std::string& currentCategory, std::vector<API>& apiList)
{
    if (node.is_object())
    {
        // Check if this node has an ALL tag
        auto allIt = node.find("ALL");
        if (allIt != node.end() && allIt->is_array())
        {
            // Store the ALL tag information
            AllTagInfo allInfo;
            allInfo.category = currentCategory;

            // Get the list of APIs this ALL tag controls
            for (const auto& item : *allIt)
            {
                if (item.is_string())
                {
                    allInfo.apiNames.push_back(item.get<std::string>());
                }
            }

            if (!allInfo.apiNames.empty())
            {
                allTags.push_back(allInfo);
            }
        }

        // Continue with normal processing
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            const std::string& key = it.key();
            if (key == "ALL")
                continue; // Skip ALL keys in normal processing

            const auto& value = it.value();

            // New category path
            std::string newCategory = currentCategory.empty() ? key : currentCategory + "/" + key;

            if (value.is_object())
            {
                // This is a category node, recurse deeper
                ParseJsonNode(value, newCategory, apiList);
            }
            else if (value.is_array() && value.size() >= 2)
            {
                // This is a leaf node with an API definition
                API api;
                api.bpName = key;                       // Menu display name
                api.apiName = value[0].get<std::string>(); // API to evaluate
                api.description = value[1].get<std::string>(); // Description
                api.category = currentCategory;          // Category for grouping

                apiList.push_back(api);
            }
        }
    }
}

static bool LoadApis(const wchar_t* apiFile)
{
    apis.clear();
    try
    {
        // Convert wide string to narrow string for fstream
        std::wstring wideFilePath(apiFile);
        std::string filePath(wideFilePath.begin(), wideFilePath.end());

        // Open the JSON file
        std::ifstream jsonFile(filePath);
        if (!jsonFile.is_open())
        {
            _plugin_logputs("[" PLUGIN_NAME "] Failed to open JSON file...");
            return false;
        }

        // Parse the JSON
        json apiJson;
        jsonFile >> apiJson;

        // Process the JSON recursively starting from the root
        ParseJsonNode(apiJson, "", apis);
        _plugin_logprintf("[" PLUGIN_NAME "] Loaded %d APIs!\n", int(apis.size()));

        return true;
    }
    catch (const json::parse_error& e)
    {
        _plugin_logputs("[" PLUGIN_NAME "] Json parsing ERROR!!!");
        return false;
    }
    catch (const std::exception& e)
    {
        _plugin_logputs("[" PLUGIN_NAME "] Json Loading ERROR!!!");
        return false;
    }
    catch (...)
    {
        _plugin_logputs("[" PLUGIN_NAME "] Unknown error occurred while loading APIs...");
        return false;
    }
}


static bool SetupMenus()
{
    if (apis.empty())
        return false;

    std::map<std::string, int> categories;  // Using map for ordered categories

    // First create all the category menus and add "Set All" and "Clear All" at the beginning
    for (size_t i = 0; i < allTags.size(); i++)
    {
        auto& allInfo = allTags[i];

        // Find the menu ID for this category
        std::string categoryPath = allInfo.category;
        std::string currentPath;
        int parentMenu = hMenu;

        // Split the category path and create menu hierarchy
        std::vector<std::string> categoryParts = split(categoryPath, '/');
        for (size_t j = 0; j < categoryParts.size(); j++)
        {
            if (currentPath.empty())
                currentPath = categoryParts[j];
            else
                currentPath += "/" + categoryParts[j];

            if (!categories.count(currentPath))
            {
                int cMenu = _plugin_menuadd(parentMenu, categoryParts[j].c_str());
                if (cMenu == -1)
                {
                    _plugin_logprintf("[" PLUGIN_NAME "] Failed to add menu item for category %s\n", currentPath.c_str());
                    return false;
                }
                categories[currentPath] = cMenu;
            }
            parentMenu = categories[currentPath];
        }

        // Create "Set All" menu entry
        int setAllId = MENU_SET_ALL_BASE + static_cast<int>(i);
        if (!_plugin_menuaddentry(parentMenu, setAllId, "Set All"))
        {
            _plugin_logprintf("[" PLUGIN_NAME "] Failed to add Set All menu for %s\n", categoryPath.c_str());
        }

        // Create "Clear All" menu entry
        int clearAllId = MENU_CLEAR_ALL_BASE + static_cast<int>(i);
        if (!_plugin_menuaddentry(parentMenu, clearAllId, "Clear All"))
        {
            _plugin_logprintf("[" PLUGIN_NAME "] Failed to add Clear All menu for %s\n", categoryPath.c_str());
        }

        // Store the menu ID
        allInfo.menuId = static_cast<int>(i);
    }

    // Now add all the API entries
    for (size_t i = 0; i < apis.size(); i++)
    {
        const auto& api = apis[i];

        // Ensure the category path exists in the menu
        std::string categoryPath = api.category;
        std::string currentPath;
        int parentMenu = hMenu;

        // Split the category path and create menu hierarchy
        std::vector<std::string> categoryParts = split(categoryPath, '/');
        for (size_t j = 0; j < categoryParts.size(); j++)
        {
            if (currentPath.empty())
                currentPath = categoryParts[j];
            else
                currentPath += "/" + categoryParts[j];

            if (!categories.count(currentPath))
            {
                int cMenu = _plugin_menuadd(parentMenu, categoryParts[j].c_str());
                if (cMenu == -1)
                {
                    _plugin_logprintf("[" PLUGIN_NAME "] Failed to add menu item for category %s\n", currentPath.c_str());
                    return false;
                }
                categories[currentPath] = cMenu;
            }
            parentMenu = categories[currentPath];
        }

        // Add the API as menu entry
        auto hEntry = int(i);
        if (!_plugin_menuaddentry(parentMenu, hEntry, api.bpName.c_str()))
        {
            _plugin_logprintf("[" PLUGIN_NAME "] Failed to add menu entry for %s\n", api.bpName.c_str());
            return false;
        }
        _plugin_menuentrysetchecked(pluginHandle, hEntry, false);
    }

    // Add icon to the plugin menu
    auto hResInfo = FindResourceW(hInst, MAKEINTRESOURCEW(IDB_FAST), L"PNG");
    auto hResData = LoadResource(hInst, hResInfo);
    auto pData = LockResource(hResData);
    ICONDATA icon;
    icon.data = pData;
    icon.size = SizeofResource(hInst, hResInfo);
    _plugin_menuseticon(hMenu, &icon);

    // Add utility menu entries
    _plugin_menuaddentry(hMenu, MENU_CLEAR_ALL, "Clear All Breakpoints");
    _plugin_menuaddentry(hMenu, MENU_RELOAD, "Reload Config");
    _plugin_menuaddentry(hMenu, MENU_ABOUT, "About");
    return true;
}


// Helper function to find APIs by name
static std::vector<size_t> FindApisByName(const std::vector<std::string>& apiNames, const std::string& category)
{
    std::vector<size_t> result;

    // Find all matching APIs
    for (size_t i = 0; i < apis.size(); i++)
    {
        const auto& api = apis[i];

        // Check if this API belongs to the specified category or a subcategory
        if (api.category == category ||
            (api.category.length() > category.length() &&
                api.category.substr(0, category.length()) == category &&
                api.category[category.length()] == '/'))
        {
            // Check if the API name is in our list
            for (const auto& name : apiNames)
            {
                if (api.bpName == name)
                {
                    result.push_back(i);
                    break;
                }
            }
        }
    }

    return result;
}

// Helper function to set or clear breakpoints for multiple APIs
static void SetMultipleBreakpoints(const std::vector<size_t>& apiIndices, bool setBreakpoints)
{
    for (size_t index : apiIndices)
    {
        if (index >= apis.size())
            continue;

        auto& api = apis[index];

        if (DbgIsDebugging())
        {
            auto addr = Eval(api.apiName.c_str());
            if (addr)
            {
                auto bpType = DbgGetBpxTypeAt(addr);
                char cmd[256] = "";
                bool changed = false;

                if (setBreakpoints && !(bpType & bp_normal))
                {
                    sprintf_s(cmd, "bp %p", addr);
                    if (DbgCmdExecDirect(cmd))
                    {
                        api.enabled = true;
                        changed = true;
                    }
                }
                else if (!setBreakpoints && (bpType & bp_normal))
                {
                    sprintf_s(cmd, "bc %p", addr);
                    if (DbgCmdExecDirect(cmd))
                    {
                        api.enabled = false;
                        changed = true;
                    }
                }

                if (changed)
                {
                    // Update menu check state
                    _plugin_menuentrysetchecked(pluginHandle, int(index), api.enabled);

                    // Log the change
                    _plugin_logprintf("[" PLUGIN_NAME "] %s breakpoint %s: %s\n",
                        setBreakpoints ? "Set" : "Cleared",
                        api.bpName.c_str(),
                        api.description.c_str());
                }
            }
        }
    }
}

// Don't forget to clear allTags in ReloadConfig
bool ReloadConfig()
{
    bool ret = true;

    // Clear the menu
    if (!_plugin_menuclear(hMenu))
    {
        _plugin_logputs("[" PLUGIN_NAME "] Failed to clear menu...");
        return false;
    }

    // Clear allTags
    allTags.clear();

    // Load the API file
    if (!LoadApis(apiFile))
    {
        _plugin_logprintf("[" PLUGIN_NAME "] Failed to reload API file!\n");
        _plugin_menuaddentry(hMenu, MENU_RELOAD, "Reload Config");
        _plugin_menuaddentry(hMenu, MENU_ABOUT, "About");
        ret = false;
    }
    else
    {
        // Setup the menu items for new config
        if (!SetupMenus())
        {
            _plugin_logputs("[" PLUGIN_NAME "] Menu setup failed after reload...");
            _plugin_menuaddentry(hMenu, MENU_RELOAD, "Reload Config");
            _plugin_menuaddentry(hMenu, MENU_ABOUT, "About");
            ret = false;
        }
        else {
            _plugin_menuaddentry(hMenu, MENU_RELOAD, "Reload Config");
            _plugin_menuaddentry(hMenu, MENU_ABOUT, "About");
        }
    }

    return ret;
}

static void refreshStatus(CBTYPE type, const char* modulename)
{
    if (apis.empty())
        return;
    if (!modulename)
        return;
    if (type == CB_STOPDEBUG)
    {
        // Check for any invalid entries to remove
        for (size_t i = 0; i < apis.size(); ++i)
        {
            auto& api = apis[i];
            auto addr = Eval(api.apiName.c_str());
            auto oldenabled = api.enabled;
            api.enabled = addr ? (DbgGetBpxTypeAt(addr) & bp_normal) != 0 : false;
            if (api.enabled != oldenabled) //only waste GUI time if an update is needed
                _plugin_menuentrysetchecked(pluginHandle, int(i), api.enabled);
        }
        return;
    }

    for (size_t i = 0; i < apis.size(); ++i)
    {
        //determine if we care about this module.
        auto modnameList = split(apis[i].apiName, ':');
        auto modname = modnameList.at(0);
        if (strstr(modulename, modname.c_str()) == NULL)
        {
            //skip this module, we don't need to incur cycles to resolve it
            continue;
        }
        else
        {
            auto& api = apis[i];
            auto addr = Eval(api.apiName.c_str());
            auto oldenabled = api.enabled;
            api.enabled = addr ? (DbgGetBpxTypeAt(addr) & bp_normal) != 0 : false;
            if (api.enabled != oldenabled) //only waste GUI time if an update is needed
                _plugin_menuentrysetchecked(pluginHandle, int(i), api.enabled);
        }
    }
}

PLUG_EXPORT void CBCREATEPROCESS(CBTYPE cbType, PLUG_CB_CREATEPROCESS* info)
{
    refreshStatus(cbType, info->DebugFileName);
}

PLUG_EXPORT void CBLOADDLL(CBTYPE cbType, PLUG_CB_LOADDLL* info)
{
    refreshStatus(cbType, info->modname);
}

PLUG_EXPORT void CBSTOPDEBUG(CBTYPE cbType, PLUG_CB_STOPDEBUG* info)
{
    refreshStatus(cbType, nullptr);
}

PLUG_EXPORT void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    // Check if this is a regular API menu entry
    if (info->hEntry >= 0 && info->hEntry < int(apis.size()))
    {
        auto& api = apis[info->hEntry];
        if (DbgIsDebugging())
        {
            auto addr = Eval(api.apiName.c_str());
            if (addr)
            {
                auto bpType = DbgGetBpxTypeAt(addr);
                char cmd[256] = "";
                if (api.enabled) //already enabled -> try to disable
                {
                    if (bpType & bp_normal) //enabled in the debuggee -> try to disable
                    {
                        sprintf_s(cmd, "bc %p", addr);
                        if (DbgCmdExecDirect(cmd))
                            api.enabled = false;
                    }
                    else //already disabled in the debuggee -> nothing to do
                    {
                        _plugin_logputs("Breakpoint already disabled...");
                        api.enabled = false;
                    }
                }
                else //not yet enabled -> try to enable
                {
                    if (bpType & bp_normal) //already enabled in debuggee -> nothing to do
                    {
                        _plugin_logputs("Breakpoint already enabled...");
                        api.enabled = true;
                    }
                    else
                    {
                        sprintf_s(cmd, "bp %p", addr);
                        if (DbgCmdExecDirect(cmd))
                            api.enabled = true;
                    }
                }

                // Show description in log when enabling
                if (api.enabled && !api.description.empty())
                    _plugin_logprintf("[" PLUGIN_NAME "] %s: %s\n", api.bpName.c_str(), api.description.c_str());
            }
            else
                _plugin_logprintf("[" PLUGIN_NAME "] Failed to resolve api %s (%s)...\n", api.bpName.c_str(), api.apiName.c_str());
        }
        else
            _plugin_logputs("[" PLUGIN_NAME "] Not debugging...");
        _plugin_menuentrysetchecked(pluginHandle, info->hEntry, api.enabled);
    }
    // Check if this is a Set All menu entry
    else if (info->hEntry >= MENU_SET_ALL_BASE && info->hEntry < MENU_SET_ALL_BASE + int(allTags.size()))
    {
        int tagIndex = info->hEntry - MENU_SET_ALL_BASE;
        if (tagIndex >= 0 && tagIndex < int(allTags.size()))
        {
            const auto& allInfo = allTags[tagIndex];
            auto apiIndices = FindApisByName(allInfo.apiNames, allInfo.category);

            if (!apiIndices.empty())
            {
                _plugin_logprintf("[" PLUGIN_NAME "] Setting all breakpoints for %s...\n", allInfo.category.c_str());
                SetMultipleBreakpoints(apiIndices, true);
            }
            else
            {
                _plugin_logprintf("[" PLUGIN_NAME "] No APIs found for ALL tag in %s\n", allInfo.category.c_str());
            }
        }
    }
    // Check if this is a Clear All menu entry
    else if (info->hEntry >= MENU_CLEAR_ALL_BASE && info->hEntry < MENU_CLEAR_ALL_BASE + int(allTags.size()))
    {
        int tagIndex = info->hEntry - MENU_CLEAR_ALL_BASE;
        if (tagIndex >= 0 && tagIndex < int(allTags.size()))
        {
            const auto& allInfo = allTags[tagIndex];
            auto apiIndices = FindApisByName(allInfo.apiNames, allInfo.category);

            if (!apiIndices.empty())
            {
                _plugin_logprintf("[" PLUGIN_NAME "] Clearing all breakpoints for %s...\n", allInfo.category.c_str());
                SetMultipleBreakpoints(apiIndices, false);
            }
            else
            {
                _plugin_logprintf("[" PLUGIN_NAME "] No APIs found for ALL tag in %s\n", allInfo.category.c_str());
            }
        }
    }

    // Check if this is a Clear All
    else if (info->hEntry == MENU_CLEAR_ALL)
    {
        if (DbgIsDebugging())
        {
            _plugin_logprintf("[" PLUGIN_NAME "] Clearing all breakpoints...\n");

            // Clear all breakpoints for all APIs
            std::vector<size_t> allApiIndices;
            for (size_t i = 0; i < apis.size(); i++)
            {
                auto& api = apis[i];
                if (api.enabled)
                {
                    allApiIndices.push_back(i);
                }
            }

            // Use existing function to clear breakpoints
            SetMultipleBreakpoints(allApiIndices, false);
        }
        else
        {
            _plugin_logputs("[" PLUGIN_NAME "] Not debugging...");
        }
    }

    // Handle other menu entries
    else if (info->hEntry == MENU_RELOAD)
    {
        if (!ReloadConfig())
            MessageBoxW(GuiGetWindowHandle(), L"Error Loading config", L"Error", MB_ICONERROR);
    }
    else if (info->hEntry == MENU_ABOUT)
        MessageBoxW(GuiGetWindowHandle(), L"A X64DBG plugin set breakpoints quickly\n\nMore info: https://github.com/Chomator/FastBP/", L"FastBP", MB_ICONINFORMATION);
    else
        _plugin_logprintf("[" PLUGIN_NAME "] Unknown menu entry %d...\n", info->hEntry);
}

#define EXPAND(x) L ## x
#define DOWIDEN(x) EXPAND(x)

bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    GetModuleFileNameW(hInst, apiFile, _countof(apiFile));
    auto l = wcsrchr(apiFile, L'\\');
    if (l)
        *l = L'\0';
    wcsncat_s(apiFile, L"\\" DOWIDEN(PLUGIN_NAME) L".json", _TRUNCATE); // Changed extension to .json

    std::ifstream testFile(apiFile);
    if (!testFile.is_open())
    {
        _plugin_logprintf("[" PLUGIN_NAME "] API file not found!\n");
        return false;
    }

    if (!LoadApis(apiFile))
    {
        return false;
    }
    return true;
}

bool pluginStop()
{
    return true;
}

void pluginSetup()
{
    if (!SetupMenus())
        _plugin_menuclear(hMenu);


}
