#include "abstractions/terminal/table.h"

#include <abstractions/errors.h>
#include <fmt/ranges.h>

#include <ranges>

namespace abstractions::terminal {

namespace {

std::string replicate(const std::string &str, int times) {
    std::vector<std::string> repeated(times, str);
    return fmt::format("{}", fmt::join(repeated, ""));
}

}  // namespace

Cell::Cell(const std::string &contents) :
    _contents{contents},
    _justification{TextJustification::Left},
    _padding{0} {}

Cell &Cell::Justify(TextJustification justify) {
    _justification = justify;
    return *this;
}

Cell &Cell::Padding(int padding) {
    _padding = padding;
    return *this;
}

int Cell::TotalLength() const {
    // Cell's must be at least '1' character wide.
    int full_length = _contents.size() + 2 * _padding;
    return full_length > 0 ? full_length : 1;
}

std::string Cell::PaddedContent() const {
    auto padding = replicate(" ", _padding);
    return fmt::format("{}{}{}", padding, _contents, padding);
}

Table::Table(const std::string &horz, const std::string &vert, const std::string &corner) :
    _outer_borders{true},
    _row_dividers{true},
    _horz_sep{horz},
    _vert_sep{vert},
    _corner{corner} {}

Table &Table::AddRow(const std::vector<std::string> &row) {
    // Input cannot be empty.
    abstractions_assert(row.empty() == false);

    // The table shouldn't be "jagged", so assert out if the provided row isn't
    // the same length as the first stored row.
    abstractions_assert(_rows.empty() || _rows.front().size() == row.size());

    // Construct the row with the default cell configuration.  The program can
    // update it at a later time before rendering.
    Row new_row;
    for (auto &str : row) {
        new_row.emplace_back(str);
    }
    _rows.push_back(new_row);
    return *this;
}

Table &Table::Justify(int column, TextJustification justification) {
    abstractions_assert(_rows.empty() || column < _rows.front().size());

    if (_rows.empty()) {
        return *this;
    }

    for (auto &row : _rows) {
        row[column].Justify(justification);
    }

    return *this;
}

Table &Table::Pad(int column, int padding) {
    abstractions_assert(_rows.empty() || column < _rows.front().size());

    if (_rows.empty()) {
        return *this;
    }

    for (auto &row : _rows) {
        row[column].Padding(padding);
    }

    return *this;
}

Table &Table::Pad(int padding) {
    for (auto &row : _rows) {
        for (auto &col : row) {
            col.Padding(padding);
        }
    }

    return *this;
}

Table &Table::RowDividers(bool show) {
    _row_dividers = show;
    return *this;
}

Table &Table::OuterBorders(bool show) {
    _outer_borders = show;
    return *this;
}

Table &Table::HorizontalSeparator(const std::string &sep) {
    _horz_sep = sep;
    return *this;
}

Table &Table::VerticalSeparator(const std::string &sep) {
    _vert_sep = sep;
    return *this;
}

Table &Table::CornerSymbol(const std::string &corner) {
    _corner = corner;
    return *this;
}

Cell &Table::GetCell(int r, int c) {
    abstractions_assert(r >= 0 && r < _rows.size());

    auto row = &_rows[r];
    abstractions_assert(c >= 0 && c < row->size());

    return (*row)[c];
}

std::vector<std::string> Table::Render() const {
    if (_rows.empty()) {
        return {};
    }

    std::vector<std::string> lines;
    const int num_rows = Rows();
    const int num_columns = Columns();

    // Figure out the column sizes.
    std::vector<int> column_sizes;
    for (int c = 0; c < num_columns; c++) {
        int max_col_size = 0;
        for (int r = 0; r < num_rows; r++) {
            int length = _rows[r][c].TotalLength();
            max_col_size = max_col_size < length ? length : max_col_size;
        }

        column_sizes.push_back(max_col_size);
    }

    // Generate the horizontal divider based on the column widths.
    std::string horz_divider;
    {
        std::vector<std::string> column_dividers;
        for (const auto &width : column_sizes) {
            column_dividers.push_back(replicate(_horz_sep, width));
        }

        horz_divider = fmt::format("{}{}{}", _corner, fmt::join(column_dividers, _corner), _corner);
    }

    if (_outer_borders) {
        lines.push_back(horz_divider);
    }

    for (int r = 0; r < num_rows; r++) {
        std::vector<std::string> columns;
        for (int c = 0; c < num_columns; c++) {
            const auto &col = _rows[r][c];

            std::string justification;
            switch (col._justification) {
                case TextJustification::Centre:
                    justification = "^";
                    break;
                case TextJustification::Left:
                    justification = "<";
                    break;
                case TextJustification::Right:
                    justification = ">";
                    break;
            }

            auto fmt_cell = fmt::format("{{:{}{}}}", justification, column_sizes[c]);
            auto cell = fmt::format(fmt::runtime(fmt_cell), col.PaddedContent());
            columns.push_back(cell);
        }

        std::string line;
        if (_outer_borders) {
            line = fmt::format("{}{}{}", _vert_sep, fmt::join(columns, _vert_sep), _vert_sep);
        } else {
            line = fmt::format("{}", fmt::join(columns, _vert_sep));
        }

        lines.push_back(line);

        if (_row_dividers && r != _rows.size() - 1) {
            lines.push_back(horz_divider);
        }
    }

    if (_outer_borders) {
        lines.push_back(horz_divider);
    }

    return lines;
}

void Table::Render(const Console &console) const {
    for (auto &line : Render()) {
        console.Print(line);
    }
}

int Table::Rows() const {
    return _rows.size();
}

int Table::Columns() const {
    return _rows.empty() ? 0 : _rows.front().size();
}

}  // namespace abstractions::terminal
