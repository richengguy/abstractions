#pragma once

#include <abstractions/terminal/console.h>
#include <fmt/format.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace abstractions::terminal {

/// @brief Specify text justification for a table
enum class TextJustification { Left, Right, Centre };

/// @brief A single table cell.
class Cell {
public:
    Cell(const std::string &contents);

    /// @brief Set the text justification.
    /// @param justify text justification
    /// @return A reference to `this` to allow for chaining.
    Cell &Justify(TextJustification justify);

    /// @brief Set the amount of padding around the cell.  The padding is
    ///     applied equally to both sides.
    /// @param padding characters of padding
    /// @return A reference to `this` to allow for chaining.
    Cell &Padding(int padding);

    /// @brief The length (or size) of the cell.
    int TotalLength() const;

    /// @brief The cell's content, with padding applied.
    std::string PaddedContent() const;

private:
    friend class Table;
    std::string _contents;
    TextJustification _justification;
    int _padding;
};

/// @brief Create a table that can be printed on a console.
class Table {
    using Row = std::vector<Cell>;

public:
    /// @brief Construct a new table.
    /// @param horz the horizonal separator
    /// @param vert the vertical separator
    /// @param corner the corner separator
    Table(const std::string &horz = "-", const std::string &vert = "|",
          const std::string &corner = "+");

    /// @brief Add a row to the table.
    /// @param row a vector of strings
    /// @return A reference to `this` so calls may be chained.
    Table &AddRow(const std::vector<std::string> &row);

    /// @brief Add a row to the table.
    /// @tparam ...Args argument types
    /// @param ...args The values that will make up the row.  They are all
    ///     automatically converted into strings using the `fmt` library.
    /// @return A reference to `this` so calls may be chained.
    template <typename... Args>
    Table &AddRow(Args... args) {
        std::vector<std::string> row;
        AddColumns(row, args...);
        AddRow(row);
        return *this;
    }

    /// @brief Set the text justification on a particular column.
    /// @note This is only applied to rows added *before* this method is called.
    /// @param column column index
    /// @param justify text justification
    /// @return A reference to `this` so calls may be chained.
    Table &Justify(int column, TextJustification justify);

    /// @brief Set the padding on a particular column.
    /// @note This is only applied to rows added *before* this method is called.
    /// @param column column index
    /// @param padding padding, in characters
    /// @return A reference to `this` so calls may be chained.
    Table &Pad(int column, int padding);

    /// @brief Set the padding for all columns in the table.
    /// @param padding padding, in characters
    /// @return A reference to `this` so calls may be chained.
    Table &Pad(int padding);

    /// @brief Show hide the dividers that go between rows.
    /// @param show visibility
    /// @return A reference to `this` so calls may be chained.
    Table &RowDividers(bool show);

    /// @brief Show/hide the table's outer borders.
    /// @param show visibility
    /// @return A reference to `this` so calls may be chained.
    Table &OuterBorders(bool show);

    /// @brief The character used for the horizontal, or row, separator.
    /// @param sep separator character
    /// @return A reference to `this` so calls may be chained.
    Table &HorizontalSeparator(const std::string &sep);

    /// @brief The character used for the vertical, or column, separator.
    /// @param sep separator character
    /// @return A reference to `this` so calls may be chained.
    Table &VerticalSeparator(const std::string &sep);

    /// @brief The character used for corners or where the separators meet.
    /// @param corner corner character
    /// @return A reference to `this` so calls may be chained
    Table &CornerSymbol(const std::string &corner);

    /// @brief Obtain the cell at a particular row or column.
    /// @param r row index
    /// @param c column index
    /// @return A reference to the cell.
    Cell &GetCell(int r, int c);

    /// @brief Render the table so that it can be printed.
    /// @return The formatted table as a set of strings.
    std::vector<std::string> Render() const;

    /// @brief Render the table to the given console.
    /// @param console The console the table will be output to.
    void Render(const Console &console) const;

    /// @brief The number of rows in the table.
    int Rows() const;

    /// @brief The number of columns in the table.
    int Columns() const;

private:
    template <typename T>
    void AddColumns(std::vector<std::string> &row, T arg) {
        row.push_back(fmt::format("{}", arg));
    }

    template <typename T, typename... Args>
    void AddColumns(std::vector<std::string> &row, T arg, Args... args) {
        row.push_back(fmt::format("{}", arg));
        AddColumns(row, args...);
    }

    bool _outer_borders;
    bool _row_dividers;
    std::string _horz_sep;
    std::string _vert_sep;
    std::string _corner;
    std::vector<Row> _rows;
};

}  // namespace abstractions::terminal
