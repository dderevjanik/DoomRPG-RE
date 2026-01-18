#ifndef INIFILE_H__
#define INIFILE_H__

#include <SDL.h>

#define INI_MAX_LINE_LENGTH 256
#define INI_MAX_SECTION_NAME 64
#define INI_MAX_KEY_NAME 64
#define INI_MAX_VALUE_LENGTH 128

// Forward declarations
struct IniSection_s;
struct IniEntry_s;

// Key-value entry
typedef struct IniEntry_s
{
    char key[INI_MAX_KEY_NAME];
    char value[INI_MAX_VALUE_LENGTH];
    struct IniEntry_s* next;
} IniEntry_t;

// Section containing entries
typedef struct IniSection_s
{
    char name[INI_MAX_SECTION_NAME];
    IniEntry_t* entries;
    struct IniSection_s* next;
} IniSection_t;

// Main INI file structure
typedef struct IniFile_s
{
    IniSection_t* sections;
    char filepath[256];
} IniFile_t;

// INI file functions
IniFile_t* IniFile_load(const char* filepath);
void IniFile_free(IniFile_t* ini);

// Get values from INI file
const char* IniFile_getString(IniFile_t* ini, const char* section, const char* key, const char* defaultValue);
int IniFile_getInt(IniFile_t* ini, const char* section, const char* key, int defaultValue);
float IniFile_getFloat(IniFile_t* ini, const char* section, const char* key, float defaultValue);
SDL_bool IniFile_getBool(IniFile_t* ini, const char* section, const char* key, SDL_bool defaultValue);

// Check if section/key exists
SDL_bool IniFile_hasSection(IniFile_t* ini, const char* section);
SDL_bool IniFile_hasKey(IniFile_t* ini, const char* section, const char* key);

// Get section by name
IniSection_t* IniFile_getSection(IniFile_t* ini, const char* section);

#endif
