/**
 *  @file logging.hpp
 *  @brief Class and #defines for the logger.
 */

#pragma once

#include "engine/os.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define INITLOGGER(logFile, vbLevel) phx::Logger::get()->init( logFile, vbLevel )
#define DESTROYLOGGER()              phx::Logger::get()->destroy()

#define LERROR(message)              phx::Logger::get()->logMessage( __FILE__, __LINE__, "", message, phx::LogVerbosity::ERROR)
#define LINFO(message)               phx::Logger::get()->logMessage( __FILE__, __LINE__, "", message, phx::LogVerbosity::INFO)

#ifdef PHX_DEBUG
	#define LDEBUG(message)          phx::Logger::get()->logMessage( __FILE__, __LINE__, "", message, phx::LogVerbosity::DEBUG)
	#define LWARNING(message)        phx::Logger::get()->logMessage( __FILE__, __LINE__, "", message, phx::LogVerbosity::WARNING)
#else
	#define LDEBUG(message)
	#define LWARNING(message)
#endif

// These are here for "backward compatability" aka I can't be arsed & some people may prefer the shorter version
// (at the risk of conflicts)
#define ERROR(message)   LERROR(message)
#define INFO(message)    LINFO(message)
#define DEBUG(message)   LDEBUG(message)
#define WARNING(message) LWARNING(message)

namespace phx
{
	class Console
	{
	public:
		/**
		 * @enum  phx::Console::Color
		 * @brief The fixed list colors that text can be set to in the console. 
		 */
		enum class Color
		{
			RED         = 0,
			GREEN       = 1,
			BLUE        = 2,
			YELLOW      = 3,
			WHITE       = 4,
			BLACK       = 5,
			DARK_YELLOW = 6,
			DARK_RED    = 7,
			DARK_GREEN  = 8,
			DARK_BLUE   = 9,
			MAGENTA     = 10
		};

		/**
		 * @brief Set the text color for any preceding text (stdout). The color will remain as set here until it is changed by another `setTextColor` call.
		 * @param[in] The color to set any preceding text to.
		 */
		static void setTextColor(const Color& color);
	private:
	};

	/**
	 * @enum	phx::LogVerbosity
	 * @brief	This is what will define whether certain messages are low enough a verbosity to be outputted.
	 */
	enum class LogVerbosity
	{
		ERROR	= 0, ///< ERROR Is for critical messages that result in FAILURES
		WARNING = 1, ///< WARNING More verbose logging, for errors that are not FATAL, but are errors none-the-less
		INFO	= 2, ///< INFO For important information which can aid finding problems quickly
		DEBUG	= 3, ///< DEBUG For absolutely everything, like entering a function to exiting the function.
	};

	class Logger
	{
	public:
		/**
		 * @brief Fetches the static instance of the Logger (yes it's a singleton...)
		 * @return A pointer to the static instance of the logger. Don't try to `delete` or `free` this pointer, please.
		 */
		static Logger* get();

		/**
		 * @brief Initialise the Logger, open file and set the initial logging verbosity level.
		 * @param[in] logFile The file in which log messages should be outputted to, as well as the console.
		 * @param[in] vbLevel The initial logging verbosity, dictates what should and shouldn't be outputted, by referring to LogVerbosity.
		 */
		void init(std::string logFile, LogVerbosity vbLevel);

		/**
		 * @brief Destroy the logger, by closing the log file.
		 */
		void destroy();

		/**
		 * @brief logMessage is to actually log the message to the console and the file opened by init();
		 * @param errorFile     The file from which the error is occurring
		 * @param lineNumber    The line from which the error is occurring
		 * @param message       The actual message to be logged.
		 * @param verbosity     The verbosity of the message, dictates whether the message is outputted or not.
		 */
		void logMessage(std::string errorFile, int lineNumber, std::string subSectors, std::string message, LogVerbosity verbosity);

	private:
		Logger() {}
		~Logger() {}

		/// @brief String for the file to log to.
		std::string m_logFile;

		/// @brief The file handle for the logging file, opened by init(), and closed by destroy().
		std::ofstream m_logFileHandle;

		/// @brief The initial logging level that is referred to, to dictate what gets logged and what doesn't.
		LogVerbosity m_vbLevel;

		/// @brief The lookup table for ENUM, so something can get outputted like "[ERROR]"
		const char* LogVerbosityLookup[4];

		/// @brief The previous message that has been logged.
		std::string m_prevMessage;

		/// @brief The number of times the current message has tried to be logged
		size_t m_currentDuplicates;
	};

}
