
/**
* \file Logger.hpp
*
* \brief Defines the class to implement the logger.
*
* \author rando98
* \author giuliamazzeellee
*/

#ifndef LOGGER_HPP
#define LOGGER_HPP


#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <ctime>
#include <cstring>
#include <filesystem>

/** LogPriority class.
*
*	Enum class to list different priorities
* for the logger.
*/
enum LogPriority
{
	TracePriority = 0,
	DebugPriority = 1,
	InfoPriority = 2,
	WarnPriority = 3,
	ErrorPriority = 4,
	CriticalPriority = 5
};

/** Logger.
*	Class used to print in terminal and file useful logging messages.
*/
class Logger
{
private:

	/** priority of the logger to be set at the beginning of the program */
	LogPriority priority = InfoPriority;

	std::mutex log_mutex;

	/** name of the file, if you want to save Logger messages on file */
	std::string filepath;
	std::ofstream file;

	/** flag to decide whether to print logging on terminal or not */
	bool EnableTerminalOutputFlag = false;

	/** flag to decide whether to print logging on file or nor */
	bool EnableFileOutputFlag = false;

public:

	/** priority setter
	*		\param new_priority priority to set for the logger
	*/
	static void SetPriority(LogPriority new_priority)
	{
		get_instance().priority = new_priority;
	}

	/** Method to enable logger messages on terminal */
	static void EnableTerminalOutput(bool EnableTerminalOutput_ = true)
	{
		get_instance().EnableTerminalOutputFlag = EnableTerminalOutput_;
	}

	/** Method to enable the printing of the logger messages on file.
	*		\param EnableFileOutputFlag_ true to enable file output, false otherwise
	*		\param new_filepath 	Name of the file where to save output messages; mind that
	*													eventual folders in the path must already exists. If the
	*													filename is not provided, a new file is automatically generated
	*													using the current time.
	*/
	static void EnableFileOutput(bool EnableFileOutputFlag_ = true, const std::string& new_filepath="")
	{
		get_instance().EnableFileOutputFlag = EnableFileOutputFlag_;

		if(EnableFileOutputFlag_)
		{
			if(new_filepath != "")
			{
				Logger& logger_instance = get_instance();
				logger_instance.filepath = new_filepath;
				logger_instance.enable_file_output();
			}
			else
			{
				Logger& logger_instance = get_instance();

				std::time_t current_time = std::time(0);
				std::tm* timestamp = std::localtime(&current_time);

				char buffer[50];
				strftime(buffer, 50, "%y%m%d_%H%M%S", timestamp);

				std::filesystem::create_directory("logs/");

				std::string name = "logs/logInfo_" + std::string(buffer) + ".txt";

				logger_instance.filepath = name;

				logger_instance.enable_file_output();
			}
		}
	}

	/** print Trace priority messages */
	static void Trace(const std::string& message)
	{
		get_instance().log("[Trace]\t", TracePriority, message);
	}

	/** print Debug priority messages */
	static void Debug(const std::string& message)
	{
		get_instance().log("[Debug]\t", DebugPriority, message);
	}

	/** print Info priority messages */
	static void Info(const std::string& message)
	{
		get_instance().log("[Info]\t", InfoPriority, message);
	}

	/** print Warn priority messages */
	static void Warn(const std::string& message)
	{
		get_instance().log("[Warn]\t", WarnPriority, message);
	}

	/** print Error priority messages */
	static void Error(const std::string& message)
	{
		get_instance().log("[Error]\t", ErrorPriority, message);
	}

	/** print Critical priority messages */
	static void Critical(const std::string& message)
	{
		get_instance().log("[Critical]\t", CriticalPriority, message);
	}

private:

	Logger() {}

	Logger(const Logger&) = delete;
	Logger& operator= (const Logger&) = delete;

	~Logger()
	{
		if(EnableFileOutputFlag)
			free_file();
	}

	static Logger& get_instance()
	{
		static Logger logger;
		return logger;
	}

	void log(const std::string& message_priority_str, LogPriority message_priority, const std::string& message)
	{
		if (priority <= message_priority)
		{
			std::time_t current_time = std::time(nullptr);
			std::tm* timestamp = std::localtime(&current_time);

			std::scoped_lock lock(log_mutex);

			if(EnableTerminalOutputFlag)
			{
				std::cout << std::put_time(timestamp, "%c") << "\t" << message_priority_str + message << std::endl;
			}

			if(EnableFileOutputFlag)
			{
				file << std::put_time(timestamp, "%c") << "\t" << message_priority_str + message << std::endl;
			}
		}
	}

	void enable_file_output()
	{
		if(file)
		{
			file.close();
		}

		file.open(filepath);

		if (!file)
		{
			std::cout << "Logger: Failed to open file at " + filepath << std::endl;
		}
	}

	void free_file()
	{
		file.close();
	}

};

#endif /*LOGGER_HPP*/
