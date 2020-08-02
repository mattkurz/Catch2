#include <catch2/internal/catch_textflow.hpp>

#include <cstring>
#include <ostream>

namespace {
    bool isWhitespace( char c ) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    bool isBreakableBefore( char c ) {
        static const char chars[] = "[({<|";
        return std::memchr( chars, c, sizeof( chars ) - 1 ) != nullptr;
    }

    bool isBreakableAfter( char c ) {
        static const char chars[] = "])}>.,:;*+-=&/\\";
        return std::memchr( chars, c, sizeof( chars ) - 1 ) != nullptr;
    }

    bool isBoundary( std::string const& line, size_t at ) {
        assert( at > 0 );
        assert( at <= line.size() );

        return at == line.size() ||
               ( isWhitespace( line[at] ) && !isWhitespace( line[at - 1] ) ) ||
               isBreakableBefore( line[at] ) ||
               isBreakableAfter( line[at - 1] );
    }

} // namespace

namespace Catch {
    namespace TextFlow {

        void Column::iterator::calcLength() {
            assert( m_stringIndex < m_column.m_strings.size() );

            m_suffix = false;
            auto width = m_column.m_width - indent();
            m_end = m_pos;
            if ( line()[m_pos] == '\n' ) {
                ++m_end;
            }
            while ( m_end < line().size() && line()[m_end] != '\n' ) {
                ++m_end;
            }

            if ( m_end < m_pos + width ) {
                m_len = m_end - m_pos;
            } else {
                size_t len = width;
                while ( len > 0 && !isBoundary( line(), m_pos + len ) ) {
                    --len;
                }
                while ( len > 0 && isWhitespace( line()[m_pos + len - 1] ) ) {
                    --len;
                }

                if ( len > 0 ) {
                    m_len = len;
                } else {
                    m_suffix = true;
                    m_len = width - 1;
                }
            }
        }

        Column::iterator& Column::iterator::operator++() {
            m_pos += m_len;
            if ( m_pos < line().size() && line()[m_pos] == '\n' ) {
                m_pos += 1;
            } else {
                while ( m_pos < line().size() &&
                        isWhitespace( line()[m_pos] ) ) {
                    ++m_pos;
                }
            }

            if ( m_pos == line().size() ) {
                m_pos = 0;
                ++m_stringIndex;
            }
            if ( m_stringIndex < m_column.m_strings.size() ) {
                calcLength();
            }
            return *this;
        }

        std::ostream& operator<<(std::ostream& os, Column const& col) {
            bool first = true;
            for (auto line : col) {
                if (first) {
                    first = false;
                } else {
                    os << '\n';
                }
                os << line;
            }
            return os;
        }

        std::ostream& operator<<(std::ostream& os, Columns const& cols) {
            bool first = true;
            for (auto line : cols) {
                if (first) {
                    first = false;
                } else {
                    os << '\n';
                }
                os << line;
            }
            return os;
        }


        Columns Column::operator+(Column const& other) {
            Columns cols;
            cols += *this;
            cols += other;
            return cols;
        }

    } // namespace TextFlow
} // namespace Catch
