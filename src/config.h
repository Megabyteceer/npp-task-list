#ifndef CONFIG_H
#define CONFIG_H

enum e_config_value
{
	_config_value_invalid = -1,
	_config_value_version,
	_config_value_keywords,
	k_config_value_count
};

enum e_config_load_result
{
	_config_load_success,
	_config_load_no_file,
	_config_load_file_failed, // defaul succeeded
	_config_load_default_failed,
	k_config_load_result_count
};

e_config_load_result load_config_file();
void unload_config_file();

const char * const *get_keyword_list(int *out_keyword_count);

enum
{
	k_max_keyword_length = 255,
};

#endif // CONFIG_H