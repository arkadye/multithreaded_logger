#pragma once

#include <string>

namespace Logging
{
	using LogLevel = int;

	template <typename T>
	class InputStream;

	namespace internal_types
	{
		using Message_t = std::pair<LogLevel, std::string>;

		class LevelSetter
		{
			template <typename T>
			friend class InputStream;
			LogLevel m_level;
		public:
			LevelSetter(LogLevel level) : m_level{ level } {}
			LevelSetter(const LevelSetter&) = default;
			LevelSetter& operator=(const LevelSetter&) = default;
		};

		class Flusher
		{};
	}

	// This is single threaded. It will act like a stream. When it is destroyed, it will pass all the messages it has
	// to its parent for printing. This object must not be shared between threads, but the logger they can from can be.
	template <typename Logger_t>
	class InputStream
	{
	private:
		Logger_t * m_parent;
		std::vector<internal_types::Message_t> m_messages;
		std::ostringstream m_current_message;
		LogLevel m_current_level;
	public:
		template <typename T>
		InputStream& operator<<(const T& input)
		{
			m_current_message << input;
			return *this;
		}

		template<>
		InputStream& operator<< <internal_types::LevelSetter>(const internal_types::LevelSetter& newLevel)
		{
			m_current_level = newLevel.m_level;
			return *this;
		}

		template <>
		InputStream& operator<< <internal_types::Flusher>(const internal_types::Flusher&)
		{
			std::string msg{ m_current_message.str() };
			if (!msg.empty())
			{
				m_messages.emplace_back(m_current_level, m_current_message.str());
				m_current_message = std::ostringstream{};
			}
			return *this;
		}

		InputStream(Logger_t* parent, LogLevel initialLevel)
			: m_parent{ parent }
			, m_current_level{ initialLevel }
		{}

		InputStream()
			: InputStream(nullptr, 0)
		{}

		InputStream(const InputStream&) = delete;
		InputStream(InputStream&& other)
		{
			*this = std::move(other);
		}

		InputStream& operator=(InputStream&& other)
		{
			m_parent = other.m_parent;
			other.m_parent = nullptr;
			m_messages = std::move(other.m_messages);
			other.m_messages = decltype(m_messages){};
			m_current_message = std::move(other.m_current_message);
			other.m_current_message = decltype(m_current_message){};
			m_current_level = other.m_current_level;
			other.m_current_level = 0;
			return *this;
		}

		~InputStream()
		{
			if (m_parent != nullptr)
			{
				*this << flush();
				m_parent->addMessages(std::move(m_messages));
			}
		}
	};
}