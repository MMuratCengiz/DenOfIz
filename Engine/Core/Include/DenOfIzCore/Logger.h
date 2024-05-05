#pragma once

#include "Common.h"
#include <fstream>
#include <queue>
#include <thread>
#include <boost/format.hpp>

namespace DenOfIz
{

    enum class LoggerType
    {
        File,
        Console
    };

    enum class Verbosity : int
    {
        Critical = 0,
        Warning = 1,
        Information = 2,
        Debug = 3
    };

    class Logger
    {

#ifdef DEBUG
        const Verbosity globalVerbosity = Verbosity::Information;
#else
	const Verbosity globalVerbosity = Verbosity::Warning;
#endif

        const std::string verbosityStrMap[ 4 ] = { "Critical", "Warning", "Information", "Debug" };

        LoggerType loggerType;

        std::fstream logStream;
        std::priority_queue<std::string> messageQueue;
        std::thread listener;

        bool running = true;

        explicit Logger( const LoggerType &loggerType );

    public:
        static Logger &Get()
        {
            static Logger instance(
#ifdef DEBUG
                LoggerType::Console
#else
				LoggerType::Console
#endif
                );

            return instance;
        }

        void Log( const Verbosity &verbosity, const std::string &component, const std::string &message );
        ~Logger();

    private:
        void FileLog( const std::string &message );
        void ConsoleLog( const std::string &message ) const;
        void LogListener();
    };

}

#define LOG(verbosity, component, message) DenOfIz::Logger::Get().Log(verbosity, component, message)
