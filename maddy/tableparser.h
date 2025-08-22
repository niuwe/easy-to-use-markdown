/*
 * This project is licensed under the MIT license. For more information see the
 * LICENSE file.
 */
#pragma once

// -----------------------------------------------------------------------------

#include <functional>
#include <memory> // For std::shared_ptr
#include <regex>
#include <string>
#include <vector>
#include <sstream> // For std::stringstream

#include "maddy/blockparser.h"

// -----------------------------------------------------------------------------

namespace maddy {

// -----------------------------------------------------------------------------

/**
 * TableParser
 *
 * For more information, see the docs folder.
 *
 * @class
 */
class TableParser : public BlockParser
{
public:
  /**
   * ctor
   *
   * @method
   * @param {std::function<void(std::string&)>} parseLineCallback
   * @param {std::function<std::shared_ptr<BlockParser>(const std::string&
   * line)>} getBlockParserForLineCallback
   */
  TableParser(
    std::function<void(std::string&)> parseLineCallback,
    std::function<std::shared_ptr<BlockParser>(const std::string& line)>
      getBlockParserForLineCallback
  )
    : BlockParser(parseLineCallback, getBlockParserForLineCallback)
    , isStarted(false)
    , isFinished(false)
    , currentBlock(0)
    , currentRow(0)
  {}

  /**
   * IsStartingLine
   *
   * If the line has exact `|table>`, then it is starting the table.
   *
   * @method
   * @param {const std::string&} line
   * @return {bool}
   */
  static bool IsStartingLine(const std::string& line)
  {
    static std::string matchString("|table>");
    return line == matchString;
  }

  /**
   * AddLine
   *
   * Adding a line which has to be parsed.
   *
   * @method
   * @param {std::string&} line
   * @return {void}
   */
  void AddLine(std::string& line) override
  {
    if (!this->isStarted && line == "|table>")
    {
      this->isStarted = true;
      return;
    }

    if (this->isStarted)
    {
      if (line == "- | - | -" || line == "---|---" || line == "-|-|-" ) // Made separator more flexible
      {
        ++this->currentBlock;
        this->currentRow = 0;
        return;
      }

      if (line == "|<table")
      {
        static std::string emptyLine = "";
        this->parseBlock(emptyLine);
        this->isFinished = true;
        return;
      }

      if (this->table.size() < this->currentBlock + 1)
      {
        this->table.push_back(std::vector<std::vector<std::string>>());
      }
      this->table[this->currentBlock].push_back(std::vector<std::string>());

      // --- FIX START ---
      // Trim leading and trailing pipes to prevent empty columns
      std::string lineToSplit = line;
      if (!lineToSplit.empty() && lineToSplit.front() == '|')
      {
        lineToSplit.erase(0, 1);
      }
      if (!lineToSplit.empty() && lineToSplit.back() == '|')
      {
        lineToSplit.pop_back();
      }
      // --- FIX END ---

      std::string segment;
      std::stringstream streamToSplit(lineToSplit); // Use the trimmed string

      while (std::getline(streamToSplit, segment, '|'))
      {
        this->parseLine(segment); // This presumably handles inline markdown like **bold**
        this->table[this->currentBlock][this->currentRow].push_back(segment);
      }

      ++this->currentRow;
    }
  }

  /**
   * IsFinished
   *
   * A table ends with `|<table`.
   *
   * @method
   * @return {bool}
   */
  bool IsFinished() const override { return this->isFinished; }

protected:
  bool isInlineBlockAllowed() const override { return false; }

  bool isLineParserAllowed() const override { return true; }

  void parseBlock(std::string&) override
  {
    result << "<table>";

    bool hasHeader = false;
    bool hasFooter = false;
    bool isFirstBlock = true;
    uint32_t currentBlockNumber = 0;

    // A table has a header if it has more than one block (e.g., header and body)
    if (this->table.size() > 1)
    {
      hasHeader = true;
    }

    // A table has a footer if it has 3 or more blocks (header, body, footer)
    if (this->table.size() >= 3)
    {
      hasFooter = true;
    }

    for (const auto& block : this->table)
    {
      bool isInHeader = false;
      bool isInFooter = false;
      ++currentBlockNumber;

      if (hasHeader && isFirstBlock)
      {
        result << "<thead>";
        isInHeader = true;
      }
      else if (hasFooter && currentBlockNumber == this->table.size())
      {
        result << "<tfoot>";
        isInFooter = true;
      }
      else
      {
        result << "<tbody>";
      }

      for (const auto& row : block)
      {
        result << "<tr>";

        for (const auto& column : row)
        {
          if (isInHeader)
          {
            result << "<th>" << column << "</th>";
          }
          else
          {
            result << "<td>" << column << "</td>";
          }
        }

        result << "</tr>";
      }

      if (isInHeader)
      {
        result << "</thead>";
      }
      else if (isInFooter)
      {
        result << "</tfoot>";
      }
      else
      {
        result << "</tbody>";
      }

      isFirstBlock = false;
    }

    result << "</table>";
  }

private:
  bool isStarted;
  bool isFinished;
  uint32_t currentBlock;
  uint32_t currentRow;
  // 3D vector: table -> blocks (thead/tbody/tfoot) -> rows -> columns
  std::vector<std::vector<std::vector<std::string>>> table;
}; // class TableParser

// -----------------------------------------------------------------------------

} // namespace maddy
