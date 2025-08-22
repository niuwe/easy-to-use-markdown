/*
 * This project is licensed under the MIT license. For more information see the
 * LICENSE file.
 */
#pragma once

// -----------------------------------------------------------------------------

#include <functional>
#include <memory>
#include <regex>
#include <string>

#include "maddy/blockparser.h"

// -----------------------------------------------------------------------------

namespace maddy {

// -----------------------------------------------------------------------------

/**
 * QuoteParser
 *
 * @class
 */
class QuoteParser : public BlockParser
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
  QuoteParser(
    std::function<void(std::string&)> parseLineCallback,
    std::function<std::shared_ptr<BlockParser>(const std::string& line)>
      getBlockParserForLineCallback
  )
    : BlockParser(parseLineCallback, getBlockParserForLineCallback)
    , isStarted(false)
    , isFinished(false)
  {}

  /**
   * IsStartingLine
   *
   * A quote starts with `>`.
   *
   * @method
   * @param {const std::string&} line
   * @return {bool}
   */
  static bool IsStartingLine(const std::string& line)
  {
    return !line.empty() && line[0] == '>';
  }

  /**
   * AddLine
   *
   * Adding a line which has to be parsed, with corrected nesting logic.
   *
   * @method
   * @param {std::string&} line
   * @return {void}
   */
  void AddLine(std::string& line) override
  {
    if (!this->isStarted)
    {
      this->result << "<blockquote>";
      this->isStarted = true;
    }

    // --- FIX START: Correct termination logic ---

    // If a line is not a quote, it's a signal to end the block.
    if (!IsStartingLine(line))
    {
      // If there's an active child, it must be terminated first.
      // We pass the non-quote line to it, which will trigger its own finish logic.
      if (this->childParser)
      {
        this->childParser->AddLine(line);
        // Now that the child has processed the end signal, we collect its COMPLETE result.
        this->result << this->childParser->GetResult().str();
        this->childParser = nullptr;
      }

      // Now, terminate the parent parser.
      this->result << "</blockquote>";
      this->isFinished = true;
      return;
    }

    // --- FIX END ---

    // The line is a valid quote line. Strip one level of ">".
    std::string content = line;
    if (content.length() > 1 && content[1] == ' ')
    {
      content.erase(0, 2); // Remove "> "
    }
    else
    {
      content.erase(0, 1); // Remove ">"
    }

    // If a child parser is already active, pass the stripped content to it.
    if (this->childParser)
    {
      this->childParser->AddLine(content);

      // After processing, check if the child has finished (e.g., went from `>>` to `>`).
      if (this->childParser->IsFinished())
      {
        this->result << this->childParser->GetResult().str();
        this->childParser = nullptr;
      }
    }
    else // No active child parser.
    {
      // Check if the stripped content is ALSO a quote, meaning we need to START nesting.
      if (IsStartingLine(content))
      {
        this->childParser = std::make_shared<QuoteParser>(
          this->parseLineCallback,
          this->getBlockParserForLineCallback
        );
        this->childParser->AddLine(content);
      }
      else // Not nested, just regular content for the current quote level.
      {
        this->parseLine(content);
        this->result << content << "<br/>";
      }
    }
  }

  /**
   * IsFinished
   *
   * @method
   * @return {bool}
   */
  bool IsFinished() const override { return this->isFinished; }

protected:
  bool isInlineBlockAllowed() const override { return true; }

  bool isLineParserAllowed() const override { return true; }

  void parseBlock(std::string& /*line*/) override {}

private:
  bool isStarted;
  bool isFinished;
}; // class QuoteParser

// -----------------------------------------------------------------------------

} // namespace maddy
