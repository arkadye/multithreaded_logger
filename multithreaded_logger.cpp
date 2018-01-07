#include "multithreaded_logger.h"

using namespace Logging;

void MultithreadedLogger::m_printer_function()
{
	while (!m_should_stop)
	{
		std::this_thread::sleep_for(m_tick_time);
		Lock_t guard{ m_data_lock };
		for (auto&& message : m_messages)
		{
			if (message.first <= m_current_log_level)
			{
				m_target << std::move(message.second);
			}
		}
		m_messages.clear();
	}
}

void MultithreadedLogger::addMessages(std::vector<internal_types::Message_t> messages)
{
	if (messages.empty())
		return;
	Lock_t guard{ m_data_lock };
	for (auto&& message : messages)
	{
		m_messages.emplace_back(std::move(message));
	}
}


MultithreadedLogger::MultithreadedLogger(MultithreadedLogger::Duration_t TickTime, std::ostream& target)
	: m_current_log_level{ 0 }
	, m_tick_time{ TickTime }
	, m_target{ target }
{
	start_printing();
}

MultithreadedLogger::~MultithreadedLogger()
{
	stop_printing();
}

InputStream<MultithreadedLogger> MultithreadedLogger::getLogger(LogLevel initialLevel)
{
	return InputStream<MultithreadedLogger>(this, initialLevel);
}

void MultithreadedLogger::start_printing()
{
	m_should_stop = false;
	m_printer_thread = std::thread{ &MultithreadedLogger::m_printer_function,this };
}

void MultithreadedLogger::stop_printing()
{
	m_should_stop = true;
	m_printer_thread.join();
}

template <typename T>
InputStream<MultithreadedLogger> MultithreadedLogger::operator<<(const T& type)
{
	return std::move((getLogger() << type));
}

template <typename LevelType>
void MultithreadedLogger::setLogLevel(LevelType type)
{
	m_current_log_level = static_cast<LogLevel>(type);
}