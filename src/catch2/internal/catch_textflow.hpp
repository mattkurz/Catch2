#ifndef CATCH_CLARA_TEXTFLOW_HPP_INCLUDED
#define CATCH_CLARA_TEXTFLOW_HPP_INCLUDED

#include <catch2/internal/catch_console_width.hpp>

#include <cassert>
#include <string>
#include <vector>


namespace Catch {
namespace TextFlow {

class Columns;

class Column {
	std::string m_string;
	size_t m_width = CATCH_CONFIG_CONSOLE_WIDTH - 1;
	size_t m_indent = 0;
	size_t m_initialIndent = std::string::npos;

public:
	class iterator {
		friend Column;

		struct EndTag {};

		Column const& m_column;
		bool m_stringEnded = false;
		size_t m_pos = 0;

		size_t m_len = 0;
		size_t m_end = 0;
		bool m_suffix = false;

		iterator(Column const& column, EndTag)
			: m_column(column),
			m_stringEnded(true) {}

		void calcLength();

		auto indent() const -> size_t {
            auto initial = m_pos == 0 && !m_stringEnded
                               ? m_column.m_initialIndent
                               : std::string::npos;
            return initial == std::string::npos ? m_column.m_indent : initial;
        }

		std::string addIndentAndSuffix(std::string const& plain) const {
			return std::string(indent(), ' ') + (m_suffix ? plain + "-" : plain);
		}

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = std::string;
		using pointer = value_type * ;
		using reference = value_type & ;
		using iterator_category = std::forward_iterator_tag;

		explicit iterator(Column const& column) : m_column(column) {
			assert(m_column.m_width > m_column.m_indent);
			assert(m_column.m_initialIndent == std::string::npos || m_column.m_width > m_column.m_initialIndent);
			calcLength();
			if (m_len == 0) {
				m_stringEnded = true;
			}
		}

		auto operator *() const -> std::string {
			assert(!m_stringEnded);
			assert(m_pos <= m_end);
			return addIndentAndSuffix(m_column.m_string.substr(m_pos, m_len));
		}

		iterator& operator++();
		iterator operator++(int);

		auto operator ==(iterator const& other) const -> bool {
			return
				m_pos == other.m_pos &&
				m_stringEnded == other.m_stringEnded &&
				&m_column == &other.m_column;
		}
		auto operator !=(iterator const& other) const -> bool {
			return !operator==(other);
		}
	};
	using const_iterator = iterator;

	explicit Column( std::string const& text ): m_string( text ) {}

	auto width(size_t newWidth) -> Column& {
		assert(newWidth > 0);
		m_width = newWidth;
		return *this;
	}
	auto indent(size_t newIndent) -> Column& {
		m_indent = newIndent;
		return *this;
	}
	auto initialIndent(size_t newIndent) -> Column& {
		m_initialIndent = newIndent;
		return *this;
	}

    size_t width() const { return m_width; }
    iterator begin() const { return iterator( *this ); }
	iterator end() const { return { *this, iterator::EndTag{} }; }

	friend std::ostream& operator<<( std::ostream& os, Column const& col );

	Columns operator + (Column const& other);
};

//! Creates a column that serves as an empty space of specific width
Column Spacer( size_t spaceWidth );

class Columns {
	std::vector<Column> m_columns;

public:

	class iterator {
		friend Columns;
		struct EndTag {};

		std::vector<Column> const& m_columns;
		std::vector<Column::iterator> m_iterators;
		size_t m_activeIterators;

		iterator(Columns const& columns, EndTag)
			: m_columns(columns.m_columns),
			m_activeIterators(0) {
			m_iterators.reserve(m_columns.size());

			for (auto const& col : m_columns)
				m_iterators.push_back(col.end());
		}

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = std::string;
		using pointer = value_type * ;
		using reference = value_type & ;
		using iterator_category = std::forward_iterator_tag;

		explicit iterator(Columns const& columns)
			: m_columns(columns.m_columns),
			m_activeIterators(m_columns.size()) {
			m_iterators.reserve(m_columns.size());

			for (auto const& col : m_columns)
				m_iterators.push_back(col.begin());
		}

		auto operator ==(iterator const& other) const -> bool {
			return m_iterators == other.m_iterators;
		}
		auto operator !=(iterator const& other) const -> bool {
			return m_iterators != other.m_iterators;
		}
		std::string operator*() const;
		iterator& operator++() {
			for (size_t i = 0; i < m_columns.size(); ++i) {
				if (m_iterators[i] != m_columns[i].end())
					++m_iterators[i];
			}
			return *this;
		}
		iterator operator++(int) {
			iterator prev(*this);
			operator++();
			return prev;
		}
	};
	using const_iterator = iterator;

	iterator begin() const { return iterator(*this); }
	iterator end() const { return { *this, iterator::EndTag() }; }

	Columns& operator+= (Column const& col);
	Columns operator+ (Column const& col);

	friend std::ostream& operator<< (std::ostream& os, Columns const& cols);
};

}
}
#endif // CATCH_CLARA_TEXTFLOW_HPP_INCLUDED
