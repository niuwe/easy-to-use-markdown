/*
 * This project is licensed under the MIT license. For more information see the
 * LICENSE file.
 */
#pragma once

// -----------------------------------------------------------------------------

#include <regex>
#include <string>

#include "maddy/lineparser.h"

// -----------------------------------------------------------------------------

namespace maddy {

// -----------------------------------------------------------------------------

/**
 * BreakLineParser
 *
 * @class
 */
//class BreakLineParser : public LineParser
//{
//public:
//  /**
//   * Parse
//   *
//   * From Markdown: `text\r\n text`
//   *
//   * To HTML: `text<br> text`
//   *
//   * @method
//   * @param {std::string&} line The line to interpret
//   * @return {void}
//   */
//  void Parse(std::string& line) override
//  {
//    static std::regex re(R"((\r\n|\r))");
//    static std::string replacement = "<br>";
//
//    line = std::regex_replace(line, re, replacement);
//  }
//}; // class BreakLineParser

class BreakLineParser : public LineParser
{
public:
    void Parse(std::string& line) override
    {
        // 新的規則運算式：尋找行尾的兩個或更多空格
        static std::regex re(R"( {2,}$)");

        // 新的替換內容
        static std::string replacement = "<br>";

        // 我們只在找到匹配項時才進行替換
        if (std::regex_search(line, re)) {
            line = std::regex_replace(line, re, replacement);
        }
    }
}; // class BreakLineParser

// -----------------------------------------------------------------------------

} // namespace maddy
