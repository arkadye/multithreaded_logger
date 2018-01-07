#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>

#include "multithreaded_logging_types.h"

namespace Logging
{
	// Work with any compatible type.
	template <typename LevelType>
	inline internal_types::LevelSetter setlevel(LevelType level) {
		return internal_types::LevelSetter{ static_cast<LogLevel>(level) };
	}
	inline internal_types::Flusher flush() { return internal_types::Flusher{}; }


	// Handle logging from multiple threads. Log level can be set. Messages get printed if the current log level
	// is greater or equal to the message log level.
	class MultithreadedLogger
	{
	public:
		using Duration_t = std::chrono::milliseconds;
	private:
		friend class InputStream<MultithreadedLogger>;
		using Lock_t = std::lock_guard<std::mutex>;
		std::atomic<LogLevel> m_current_log_level;
		std::mutex m_data_lock;
		std::vector<internal_types::Message_t> m_messages;
		Duration_t m_tick_time;
		std::atomic_bool m_should_stop;
		std::thread m_printer_thread;
		std::ostream& m_target;

		void m_printer_function();
		void addMessages(std::vector<internal_types::Message_t> messages);

	public:
		MultithreadedLogger(Duration_t TickTime, std::ostream& target);
		MultithreadedLogger(const MultithreadedLogger&) = delete;
		MultithreadedLogger(MultithreadedLogger&&) = default;
		MultithreadedLogger& operator=(const MultithreadedLogger&) = delete;
		MultithreadedLogger& operator=(MultithreadedLogger&&) = default;
		~MultithreadedLogger();

		InputStream<MultithreadedLogger> getLogger(LogLevel initialLevel = 0);
		void start_printing();
		void stop_printing();

		template <typename T>
		InputStream<MultithreadedLogger> operator<<(const T& type);

		template <typename LevelType>
		void setLogLevel(LevelType type);
	};
}