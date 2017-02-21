#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// constants
const char *k_config_value_strings[k_config_value_count] = {
    "version=",
    "keywords=",
};

char default_config_v1[] = "version=\n1\n\nkeywords=\nTODO:";
#define default_config default_config_v1
const int k_current_version = 1;

const char *k_config_file_path = "./plugins/config/npp_task_list.cfg";

// globals
char **g_keyword_list = NULL;
int g_keyword_count = 0;
bool g_version_confirmed = false;

// private functions
static e_config_value g_current_config_value = _config_value_invalid;
static bool handle_string(const char *string);
static e_config_value lookup_config_value(const char *config_value_string);

// implementation
e_config_load_result load_config_file()
{
    e_config_load_result result = _config_load_success;
    bool success = true;

    unload_config_file();

    FILE *config_file = fopen(k_config_file_path, "rt");

    if (config_file != NULL) {
        const int k_buffer_size = 1024;
        char buffer[k_buffer_size];
        int bytes_read = 0;
        int buffer_space = k_buffer_size - 1;
        int bytes_to_read;
        char *current_position = buffer;
        int left_over_length = 0;

        do {
            bytes_to_read = buffer_space;
            bytes_read = static_cast<int>(fread(current_position, 1, bytes_to_read, config_file));
            current_position[bytes_read] = '\0';
            current_position = buffer; // reset to head
            char *newline_position = strchr(buffer, '\n');

            while (newline_position != NULL && success) {
                *newline_position = '\0';
                success &= handle_string(current_position);
                *newline_position = '\n';
                current_position = newline_position + 1;
                newline_position = strchr(current_position, '\n');
            };

            left_over_length = static_cast<int>(strlen(buffer) - (current_position - buffer));
            buffer_space = k_buffer_size - 1 - left_over_length;
            memmove(buffer, current_position, left_over_length);
            current_position = buffer + left_over_length;

            if (buffer_space == 0)
                success = false;
        } while (bytes_read == bytes_to_read && success);

        if (success && left_over_length != 0) {
            buffer[left_over_length] = '\0';
            success &= handle_string(buffer);
        }

        fclose(config_file);

        if (!g_version_confirmed)
            success = false;
    } else {
        success = false;
    }

    if (!success) {
        if (config_file != NULL)
            result = _config_load_file_failed;
        else
            result = _config_load_no_file;

        unload_config_file();
        success = true;

        char *current_position = default_config; // reset to head
        char *newline_position = strchr(current_position, '\n');

        while (newline_position != NULL && success) {
            *newline_position = '\0';
            success &= handle_string(current_position);
            *newline_position = '\n';
            current_position = newline_position + 1;
            newline_position = strchr(current_position, '\n');
        };

        if (success && static_cast<size_t>(current_position - default_config) < strlen(default_config))
            success &= handle_string(current_position);

        if (!success)
            result = _config_load_default_failed;
    }

    return result;
}

// NOTE: this is not thread safe
void unload_config_file()
{
    if (g_keyword_list != NULL) {
        for (int keyword_index = 0; keyword_index < g_keyword_count; keyword_index++) {
            if (g_keyword_list[keyword_index] != NULL) {
                free(g_keyword_list[keyword_index]);
                g_keyword_list[keyword_index] = NULL;
            }
        }

        free(g_keyword_list);
        g_keyword_list = NULL;
    }

    g_keyword_count = 0;
    g_current_config_value = _config_value_invalid;
    g_version_confirmed = false;
}

static bool handle_string(
    const char *string)
{
    bool result = false;

    if (g_current_config_value == _config_value_invalid) {
        g_current_config_value = lookup_config_value(string);

        if (g_current_config_value != _config_value_invalid)
            result = true;
    } else {
        size_t string_length = strlen(string);

        if (string_length == 0) { // double newline, reset state to invalid so we can look for a new config value
            g_current_config_value = _config_value_invalid;
            result = true;
        } else {
            switch (g_current_config_value) {
            case _config_value_version:
                {
                    if (g_version_confirmed) { // we've already seen a version value
                        result = false;
                    } else {
                        int version_number = atoi(string);

                        if (version_number == k_current_version) {
                            g_version_confirmed = true;
                            result = true;
                        }
                    }
                }
                break;
            case _config_value_keywords:
                {
                    if (string_length <= k_max_keyword_length) {
                        g_keyword_list = (char**)realloc(g_keyword_list, sizeof(char*)*(g_keyword_count + 1));
                        g_keyword_list[g_keyword_count] = (char*)malloc(string_length + 1);
                        memcpy(g_keyword_list[g_keyword_count], string, string_length + 1);
                        g_keyword_count++;
                        result = true;
                    } else {
                        result = false;
                    }
                }
                break;
            default:
                result = false;
                break;
            }
        }
    }

    return result;
}

static e_config_value lookup_config_value(
    const char *config_value_string)
{
    e_config_value result = _config_value_invalid;

    for (int value_index = 0; value_index < k_config_value_count; value_index++) {
        if (strcmp(config_value_string, k_config_value_strings[value_index]) == 0) {
            result = (e_config_value)value_index;
            break;
        }
    }

    return result;
}

const char * const * get_keyword_list(
    int *out_keyword_count)
{
    *out_keyword_count = g_keyword_count;
    return g_keyword_list;
}
