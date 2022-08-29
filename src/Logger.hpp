/**
* \file Logger.hpp
*
* \brief Defines the class to implement the logger.
*
* \author Randeep Singh
* \author Giulia Mazzilli
*/

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <ctime>
#include <cstring>
#include <filesystem>

/** Enum class to list different message priorities
* 	for the logger.
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

/** Class used to print on terminal and on file logging messages. */
class Logger
{
  public:

    /** priority setter
    *
    *	\param new_priority priority to set for the Logger
    */
    static void
    SetPriority(LogPriority new_priority)
    {
      get_instance().priority = new_priority;
    }

    /** Method to enable or disable Logger messages on terminal
    *
    * 	\param EnableTerminalOutput_ true to enable terminal output, false otherwise
    * 								 (defaults to true)
    */
    static void
    EnableTerminalOutput(bool EnableTerminalOutput_ = true)
    {
      get_instance().EnableTerminalOutputFlag = EnableTerminalOutput_;
    }

    /** Method to enable the printing of the Logger messages on file.
    *
    * 	\param EnableFileOutputFlag_ true to enable file output, false otherwise
    * 								 (defaults to true)
    *	\param filename_ Name of the file where to save output messages; mind that
    *					 eventual folders in the path must already exists. If the
    *					 filename is not provided, a new file is automatically generated
    *					 using the current date and time in the title. Moreover, the date
    *          and time may be appended also to the filename provided, to
    *          distinguish between different runs.
    */
    static void
    EnableFileOutput(bool EnableFileOutputFlag_ = true, const std::string& filename_ = "")
    {
      get_instance().EnableFileOutputFlag = EnableFileOutputFlag_;

      if(EnableFileOutputFlag_)
      {
        Logger& logger_instance = get_instance();
        std::time_t current_time = std::time(0);
        std::tm* timestamp = std::localtime(&current_time);
        char buffer[50];
        strftime(buffer, 50, "%y%m%d_%H%M%S", timestamp);
        std::filesystem::create_directory("logs/");
        std::string name = "logs/log" + filename_ + "_" + std::string(buffer) + ".txt";
        logger_instance.filepath = name;
        logger_instance.enable_file_output();
      }
    }

    /** print TracePriority messages */
    static void
    Trace(const std::string& message)
    {
      get_instance().log("[Trace]\t", TracePriority, message);
    }

    /** print DebugPriority messages */
    static void
    Debug(const std::string& message)
    {
      get_instance().log("[Debug]\t", DebugPriority, message);
    }

    /** print InfoPriority messages */
    static void
    Info(const std::string& message)
    {
      get_instance().log("[Info]\t", InfoPriority, message);
    }

    /** print WarnPriority messages */
    static void
    Warn(const std::string& message)
    {
      get_instance().log("[Warn]\t", WarnPriority, message);
    }

    /** print ErrorPriority messages */
    static void
    Error(const std::string& message)
    {
      get_instance().log("[Error]\t", ErrorPriority, message);
    }

    /** print CriticalPriority messages */
    static void
    Critical(const std::string& message)
    {
      get_instance().log("[Critical]\t", CriticalPriority, message);
    }

  private:

    /** Logger class consturctor */
    Logger() {}

    /** Delete copy constructor */
    Logger(const Logger&) = delete;

    /** Delete " = " operator */
    Logger& operator= (const Logger&) = delete;

    /** Logger class destructor
    *
    * 	It closes the output file if enabled.
    */
    ~Logger()
    {
      if(EnableFileOutputFlag)
      {
        free_file();
      }
    }

    /** Logger instance getter */
    static Logger&
    get_instance()
    {
      static Logger logger;
      return logger;
    }

    /** Method to print any Logger message on terminal and file, depending on the LogPriority
    *
    * 	\param message_priority_str String to describe the Logger priority level
    * 	\param message_priority Logger priority level (see LogPriority)
    * 	\param message Logging message to be printed
    */
    void
    log(const std::string& message_priority_str, LogPriority message_priority, const std::string& message)
    {
      if(priority <= message_priority)
      {
        std::time_t current_time = std::time(nullptr);
        std::tm* timestamp = std::localtime(&current_time);

        if(EnableTerminalOutputFlag)
        {
          std::scoped_lock lock(log_mutex);
          std::cout << std::put_time(timestamp, "%c") << "\t" << message_priority_str + message << std::endl;
        }

        if(EnableFileOutputFlag)
        {
          std::scoped_lock lock(log_mutex);
          file << std::put_time(timestamp, "%c") << "\t" << message_priority_str + message << std::endl;
        }
      }
    }

    /** Method to enable the file output by opening the file specified by
    *   Logger.filepath
    */
    void
    enable_file_output()
    {
      if(file)
      {
        file.close();
      }

      file.open(filepath);

      if(!file)
      {
        std::cout << "Logger: Failed to open file at " + filepath << std::endl;
      }
    }

    /** Method to close the output file */
    void
    free_file()
    {
      file.close();
    }

  private:

    /** priority of the Logger messages to be set at the beginning of the program */
    LogPriority priority = InfoPriority;

    /** object to make the Logger thread safe */
    std::mutex log_mutex;

    /** name of the file, if you want to save Logger messages on file */
    std::string filepath;

    /** file to write Logger messages */
    std::ofstream file;

    /** flag to decide whether to print logging messages on terminal or not */
    bool EnableTerminalOutputFlag = false;

    /** flag to decide whether to print logging messages on file or not */
    bool EnableFileOutputFlag = false;

};

#endif // SRC_LOGGER_HPP_
