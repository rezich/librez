#pragma once

#ifndef USE_PD_STRING_FORMAT
#ifdef _WINDLL
#define format_string(...) stbsp_sprintf(__VA_ARGS__)
#else
#define format_string(...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"") \
    stbsp_sprintf(__VA_ARGS__); \
    _Pragma("GCC diagnostic pop")
#endif
#else
#ifdef _WINDLL
#define format_string(...) pd->system->formatString(__VA_ARGS__)
#else
#define format_string(...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"") \
    pd->system->formatString(__VA_ARGS__); \
    _Pragma("GCC diagnostic pop")
#endif
#endif

#ifdef _WINDLL
#define LOG(...) pd->system->logToConsole(__VA_ARGS__)
#else
#define LOG(...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wdouble-promotion\"") \
    pd->system->logToConsole(__VA_ARGS__); \
    _Pragma("GCC diagnostic pop")
#endif

FORCE_INLINE PDMenuItem* sysmenu_add(const char* title, PDMenuItemCallbackFunction* callback, void* userdata) { return pd->system->addMenuItem(title, callback, userdata); }
FORCE_INLINE PDMenuItem* sysmenu_add_checkbox(const char* title, bool value, PDMenuItemCallbackFunction* callback, void* userdata) { return pd->system->addCheckmarkMenuItem(title, value, callback, userdata); }
FORCE_INLINE PDMenuItem* sysmenu_add_options(const char* title, const char** options, int options_count, PDMenuItemCallbackFunction* callback, void* userdata) { return pd->system->addOptionsMenuItem(title, options, options_count, callback, userdata); }
FORCE_INLINE       void  sysmenu_remove(PDMenuItem* menu_item) { pd->system->removeMenuItem(menu_item); }
FORCE_INLINE       void  sysmenu_remove_all() { pd->system->removeAllMenuItems(); }
FORCE_INLINE const char* sysmenu_get_title(PDMenuItem* menu_item) { return pd->system->getMenuItemTitle(menu_item); }
FORCE_INLINE       void  sysmenu_set_title(PDMenuItem* menu_item, const char* title) { pd->system->setMenuItemTitle(menu_item, title); }
FORCE_INLINE        int  sysmenu_get_value(PDMenuItem* menu_item) { pd->system->getMenuItemValue(menu_item); }
FORCE_INLINE       void  sysmenu_set_value(PDMenuItem* menu_item, int value) { pd->system->setMenuItemValue(menu_item, value); }
FORCE_INLINE       void* sysmenu_get_userdata(PDMenuItem* menu_item) { return pd->system->getMenuItemUserdata(menu_item); }
FORCE_INLINE       void  sysmenu_set_userdata(PDMenuItem* menu_item, void* userdata) { pd->system->setMenuItemUserdata(menu_item, userdata); }
FORCE_INLINE       void  sysmenu_set_image(LCDBitmap* bitmap, int x_offset) { pd->system->setMenuImage(bitmap, x_offset); }

FORCE_INLINE bool is_device_flipped()   { return pd->system->getFlipped(); }
FORCE_INLINE bool is_flashing_reduced() { return pd->system->getReduceFlashing(); }
