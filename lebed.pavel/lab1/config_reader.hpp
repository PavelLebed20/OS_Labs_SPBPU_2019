#ifndef __CONFIG_READER_H__
#define __CONFIG_READER_H__

#include "logger.hpp"


class ConfigReader
{
private:
	string file_name;
	str_map conf_values;
public:
	ConfigReader(const string& FileName) : file_name(FileName) {}
	ConfigReader() = default;

	void SetFile(const string& FileName)
	{
		file_name = FileName;
	}

	bool Parse() 
	{
		// clear old data
		conf_values.clear();
		
		ifstream conf_file(file_name);

		if (!conf_file.is_open() || conf_file.eof())
		{
			g_logger.LogError("Cannot open config file: " + file_name);
			return false;
		}
		
		string key, value;
		while (conf_file >> key >> value)
		{
			if (key.length() == 0)
				break;
		    
			if (conf_values.count(key) > 0)
			{
				g_logger.LogError("Config key repeats twice: " + key);
				conf_file.close();
				return false;
			}
			conf_values.insert({ key, value });
		}

		conf_file.close();
		return true;
	}

	bool GetAsInt(const string& Key, int &Res) const
	{
		auto value = conf_values.find(Key);
		if (value == conf_values.end())
		{
			g_logger.LogError(Key + " does not exists in config file!");
			return false;
		}
		try
		{
			Res = std::stoi(value->second);
		}
		catch (...)
		{
			g_logger.LogError(value->second + " cannot parse value as int!");
			return false;
		}
		return true;
	}

	bool GetAsStr(const string& Key, string &Res) const
	{
		auto value = conf_values.find(Key);
		if (value == conf_values.end())
		{
			g_logger.LogError(Key + " does not exists in config file!");
			return false;
		}
		Res = value->second;
		return true;
	}
};

#endif // __CONFIG_READER_H__
