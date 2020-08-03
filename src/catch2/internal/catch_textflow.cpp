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
            m_suffix = false;
            auto width = m_column.m_width - indent();
            m_end = m_pos;
            std::string const& current_line = m_column.m_string;
            if ( current_line[m_pos] == '\n' ) {
                ++m_end;
            }
            while ( m_end < current_line.size() && current_line[m_end] != '\n' ) {
                ++m_end;
            }

            if ( m_end < m_pos + width ) {
                m_len = m_end - m_pos;
            } else {
                size_t len = width;
                while ( len > 0 && !isBoundary( current_line, m_pos + len ) ) {
                    --len;
                }
                while ( len > 0 && isWhitespace( current_line[m_pos + len - 1] ) ) {
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

        std::string Column::iterator::addIndentAndSuffix(std::string const& plain) const {
            std::string ret;
            const auto desired_indent = indent();
            ret.reserve(desired_indent + plain.size() + m_suffix);
            ret.append(desired_indent, ' ');
            ret.append(plain);
            if (m_suffix) {
                ret.push_back('-');
            }

            return ret;
        }

        Column::iterator::iterator( Column const& column ): m_column( column ) {
            assert( m_column.m_width > m_column.m_indent );
            assert( m_column.m_initialIndent == std::string::npos ||
                    m_column.m_width > m_column.m_initialIndent );
            calcLength();
            if ( m_len == 0 ) {
                m_pos = m_column.m_string.size();
            }
        }

        Column::iterator& Column::iterator::operator++() {
            m_pos += m_len;
            std::string const& current_line = m_column.m_string;
            if ( m_pos < current_line.size() && current_line[m_pos] == '\n' ) {
                m_pos += 1;
            } else {
                while ( m_pos < current_line.size() &&
                        isWhitespace( current_line[m_pos] ) ) {
                    ++m_pos;
                }
            }

            if ( m_pos != current_line.size() ) {
                calcLength();
            }
            return *this;
        }

        Column::iterator Column::iterator::operator++(int) {
            iterator prev(*this);
            operator++();
            return prev;
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

        Column Spacer( size_t spaceWidth ) {
            Column ret{ "" };
            ret.width( spaceWidth );
            return ret;
        }

        std::string Columns::iterator::operator*() const {
            std::string row, padding;

            for ( size_t i = 0; i < m_columns.size(); ++i ) {
                auto width = m_columns[i].width();
                if ( m_iterators[i] != m_columns[i].end() ) {
                    std::string col = *m_iterators[i];
                    row += padding + col;
                    if ( col.size() < width )
                        padding = std::string( width - col.size(), ' ' );
                    else
                        padding = "";
                } else {
                    padding += std::string( width, ' ' );
                }
            }
            return row;
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

        Columns& Columns::operator+=(Column const& col) {
            m_columns.push_back(col);
            return *this;
        }

        Columns Columns::operator+(Column const& col) {
            Columns combined = *this;
            combined += col;
            return combined;
        }

} // namespace TextFlow
} // namespace Catch
