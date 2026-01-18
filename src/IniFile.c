#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "IniFile.h"

// Helper function to trim whitespace from both ends of a string
static char* trim(char* str)
{
    char* end;

    // Trim leading whitespace
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) {
        return str;
    }

    // Trim trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator
    end[1] = '\0';

    return str;
}

// Helper function to create a new section
static IniSection_t* IniSection_create(const char* name)
{
    IniSection_t* section = (IniSection_t*)SDL_malloc(sizeof(IniSection_t));
    if (section == NULL) {
        return NULL;
    }

    SDL_memset(section, 0, sizeof(IniSection_t));
    strncpy(section->name, name, INI_MAX_SECTION_NAME - 1);
    section->name[INI_MAX_SECTION_NAME - 1] = '\0';
    section->entries = NULL;
    section->next = NULL;

    return section;
}

// Helper function to create a new entry
static IniEntry_t* IniEntry_create(const char* key, const char* value)
{
    IniEntry_t* entry = (IniEntry_t*)SDL_malloc(sizeof(IniEntry_t));
    if (entry == NULL) {
        return NULL;
    }

    SDL_memset(entry, 0, sizeof(IniEntry_t));
    strncpy(entry->key, key, INI_MAX_KEY_NAME - 1);
    entry->key[INI_MAX_KEY_NAME - 1] = '\0';
    strncpy(entry->value, value, INI_MAX_VALUE_LENGTH - 1);
    entry->value[INI_MAX_VALUE_LENGTH - 1] = '\0';
    entry->next = NULL;

    return entry;
}

// Add entry to section
static void IniSection_addEntry(IniSection_t* section, IniEntry_t* entry)
{
    if (section->entries == NULL) {
        section->entries = entry;
    } else {
        IniEntry_t* current = section->entries;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = entry;
    }
}

// Add section to INI file
static void IniFile_addSection(IniFile_t* ini, IniSection_t* section)
{
    if (ini->sections == NULL) {
        ini->sections = section;
    } else {
        IniSection_t* current = ini->sections;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = section;
    }
}

IniFile_t* IniFile_load(const char* filepath)
{
    FILE* file;
    char line[INI_MAX_LINE_LENGTH];
    IniFile_t* ini;
    IniSection_t* currentSection = NULL;

    file = fopen(filepath, "r");
    if (file == NULL) {
        printf("IniFile_load: Could not open file '%s'\n", filepath);
        return NULL;
    }

    ini = (IniFile_t*)SDL_malloc(sizeof(IniFile_t));
    if (ini == NULL) {
        fclose(file);
        return NULL;
    }

    SDL_memset(ini, 0, sizeof(IniFile_t));
    strncpy(ini->filepath, filepath, sizeof(ini->filepath) - 1);
    ini->sections = NULL;

    while (fgets(line, sizeof(line), file) != NULL) {
        char* trimmedLine = trim(line);

        // Skip empty lines and comments
        if (trimmedLine[0] == '\0' || trimmedLine[0] == ';' || trimmedLine[0] == '#') {
            continue;
        }

        // Check for section header [section]
        if (trimmedLine[0] == '[') {
            char* endBracket = strchr(trimmedLine, ']');
            if (endBracket != NULL) {
                *endBracket = '\0';
                char* sectionName = trim(trimmedLine + 1);
                
                currentSection = IniSection_create(sectionName);
                if (currentSection != NULL) {
                    IniFile_addSection(ini, currentSection);
                }
            }
        }
        // Check for key=value pair
        else if (currentSection != NULL) {
            char* equalSign = strchr(trimmedLine, '=');
            if (equalSign != NULL) {
                *equalSign = '\0';
                char* key = trim(trimmedLine);
                char* value = trim(equalSign + 1);

                // Remove quotes from value if present
                size_t valueLen = strlen(value);
                if (valueLen >= 2 && ((value[0] == '"' && value[valueLen - 1] == '"') ||
                                      (value[0] == '\'' && value[valueLen - 1] == '\''))) {
                    value[valueLen - 1] = '\0';
                    value++;
                }

                IniEntry_t* entry = IniEntry_create(key, value);
                if (entry != NULL) {
                    IniSection_addEntry(currentSection, entry);
                }
            }
        }
    }

    fclose(file);
    printf("IniFile_load: Loaded '%s'\n", filepath);
    return ini;
}

void IniFile_free(IniFile_t* ini)
{
    if (ini == NULL) {
        return;
    }

    IniSection_t* section = ini->sections;
    while (section != NULL) {
        IniSection_t* nextSection = section->next;

        IniEntry_t* entry = section->entries;
        while (entry != NULL) {
            IniEntry_t* nextEntry = entry->next;
            SDL_free(entry);
            entry = nextEntry;
        }

        SDL_free(section);
        section = nextSection;
    }

    SDL_free(ini);
}

IniSection_t* IniFile_getSection(IniFile_t* ini, const char* sectionName)
{
    if (ini == NULL || sectionName == NULL) {
        return NULL;
    }

    IniSection_t* section = ini->sections;
    while (section != NULL) {
        if (SDL_strcasecmp(section->name, sectionName) == 0) {
            return section;
        }
        section = section->next;
    }

    return NULL;
}

static IniEntry_t* IniSection_getEntry(IniSection_t* section, const char* key)
{
    if (section == NULL || key == NULL) {
        return NULL;
    }

    IniEntry_t* entry = section->entries;
    while (entry != NULL) {
        if (SDL_strcasecmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

const char* IniFile_getString(IniFile_t* ini, const char* sectionName, const char* key, const char* defaultValue)
{
    IniSection_t* section = IniFile_getSection(ini, sectionName);
    if (section == NULL) {
        return defaultValue;
    }

    IniEntry_t* entry = IniSection_getEntry(section, key);
    if (entry == NULL) {
        return defaultValue;
    }

    return entry->value;
}

int IniFile_getInt(IniFile_t* ini, const char* sectionName, const char* key, int defaultValue)
{
    const char* value = IniFile_getString(ini, sectionName, key, NULL);
    if (value == NULL) {
        return defaultValue;
    }

    return atoi(value);
}

float IniFile_getFloat(IniFile_t* ini, const char* sectionName, const char* key, float defaultValue)
{
    const char* value = IniFile_getString(ini, sectionName, key, NULL);
    if (value == NULL) {
        return defaultValue;
    }

    return (float)atof(value);
}

SDL_bool IniFile_getBool(IniFile_t* ini, const char* sectionName, const char* key, SDL_bool defaultValue)
{
    const char* value = IniFile_getString(ini, sectionName, key, NULL);
    if (value == NULL) {
        return defaultValue;
    }

    // Check for true values
    if (SDL_strcasecmp(value, "true") == 0 ||
        SDL_strcasecmp(value, "yes") == 0 ||
        SDL_strcasecmp(value, "on") == 0 ||
        SDL_strcasecmp(value, "1") == 0) {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

SDL_bool IniFile_hasSection(IniFile_t* ini, const char* sectionName)
{
    return IniFile_getSection(ini, sectionName) != NULL ? SDL_TRUE : SDL_FALSE;
}

SDL_bool IniFile_hasKey(IniFile_t* ini, const char* sectionName, const char* key)
{
    IniSection_t* section = IniFile_getSection(ini, sectionName);
    if (section == NULL) {
        return SDL_FALSE;
    }

    return IniSection_getEntry(section, key) != NULL ? SDL_TRUE : SDL_FALSE;
}
