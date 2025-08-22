#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace markdown {

class Element {
public:
  Element() {}
  virtual ~Element() {}
  virtual void write(std::ostream &out) const = 0;
};

using ElementPtr = std::shared_ptr<Element>;

class Document {
public:
  Document() {}
  ~Document() {}
  void read(const std::string &in);
  void read(std::istream &in);
  void write(std::ostream &out) const;

private:
  std::list<ElementPtr> elements;
};

class Header : public Element {
public:
  Header(int level, const std::string &text);
  ~Header() {}
  void write(std::ostream &out) const;

private:
  int _level;
  std::string _text;
};

class HorizontalRule : public Element {
public:
  HorizontalRule() {}
  ~HorizontalRule() {}
  void write(std::ostream &out) const;
};

class Paragraph : public Element {
public:
  Paragraph(const std::string &text);
  ~Paragraph() {}
  void write(std::ostream &out) const;

private:
  std::string _text;
};

class CodeFence : public Element {
public:
  CodeFence(const std::string &text);
  ~CodeFence() {}
  void write(std::ostream &out) const;

private:
  std::string _text;
};

class BlockQuote : public Element {
public:
  BlockQuote(const std::string &text);
  ~BlockQuote() {}
  void write(std::ostream &out) const;

private:
  std::string _text;
};

class List : public Element {
public:
  enum Type { Ordered, Unordered };

  List(Type type);
  ~List() {}

  void read(const std::string &in);
  void write(std::ostream &out) const;
  Type type() const;

private:
  Type _type;
  std::vector<std::string> _items;
};

inline void Document::read(const std::string &in) {
  std::stringstream sin(in);
  read(sin);
}

inline void Document::read(std::istream &in) {
  std::string line, code, block;
  bool in_code = false, in_block = false;

  while (std::getline(in, line)) {
    if (in_code) {
      if (line.length() >= 3 && line.substr(0, 3) == "```") {
        elements.emplace_back(std::make_shared<CodeFence>(code));
        code.clear();
        in_code = false;
      } else {
        code += line + "\n";
      }
      continue;
    }

    if (in_block) {
      if (line.empty()) {
        elements.emplace_back(std::make_shared<BlockQuote>(block));
        block.clear();
        in_block = false;
      } else {
        block += line + "\n";
      }
      continue;
    }

    if (line.empty()) {
      continue;
    }

    if (line[0] == '#') {
      int level = 0;
      for (; level < line.length() && line[level] == '#'; ++level)
        ;
      elements.emplace_back(
          std::make_shared<Header>(level, line.substr(level + 1)));
    } else if (line.length() >= 3 && line.substr(0, 3) == "---") {
      elements.emplace_back(std::make_shared<HorizontalRule>());
    } else if (line.length() >= 3 && line.substr(0, 3) == "```") {
      in_code = true;
    } else if (line[0] == '>') {
      in_block = true;
      block = line.substr(1) + "\n";
    } else if (line.length() >= 2 && line.substr(0, 2) == "* ") {
      auto list = std::make_shared<List>(List::Unordered);
      list->read(line);
      elements.emplace_back(list);
    } else if (line.length() >= 2 && isdigit(line[0]) && line[1] == '.' &&
               line[2] == ' ') {
      auto list = std::make_shared<List>(List::Ordered);
      list->read(line);
      elements.emplace_back(list);
    } else {
      elements.emplace_back(std::make_shared<Paragraph>(line));
    }
  }

  if (in_code) {
    elements.emplace_back(std::make_shared<CodeFence>(code));
  } else if (in_block) {
    elements.emplace_back(std::make_shared<BlockQuote>(block));
  }
}

inline void Document::write(std::ostream &out) const {
  for (const auto &element : elements) {
    element->write(out);
  }
}

inline Header::Header(int level, const std::string &text)
    : _level(level), _text(text) {}

inline void Header::write(std::ostream &out) const {
  out << "<h" << _level << ">" << _text << "</h" << _level << ">" << std::endl;
}

inline void HorizontalRule::write(std::ostream &out) const {
  out << "<hr />" << std::endl;
}

inline void process_spans(std::string &text) {
  size_t pos = 0;
  while ((pos = text.find("`", pos)) != std::string::npos) {
    size_t end = text.find("`", pos + 1);
    if (end == std::string::npos) {
      break;
    }

    text.replace(pos, end - pos + 1,
                 "<code>" + text.substr(pos + 1, end - pos - 1) + "</code>");
    pos = end + strlen("</code>");
  }

  pos = 0;
  while ((pos = text.find("**", pos)) != std::string::npos) {
    size_t end = text.find("**", pos + 2);
    if (end == std::string::npos) {
      break;
    }

    text.replace(pos, end - pos + 2,
                 "<strong>" + text.substr(pos + 2, end - pos - 2) +
                     "</strong>");
    pos = end + strlen("</strong>");
  }

  pos = 0;
  while ((pos = text.find("*", pos)) != std::string::npos) {
    size_t end = text.find("*", pos + 1);
    if (end == std::string::npos) {
      break;
    }

    text.replace(pos, end - pos + 1,
                 "<em>" + text.substr(pos + 1, end - pos - 1) + "</em>");
    pos = end + strlen("</em>");
  }

  pos = 0;
  while ((pos = text.find("[", pos)) != std::string::npos) {
    size_t end_text = text.find("]", pos + 1);
    if (end_text == std::string::npos) {
      break;
    }

    size_t start_url = text.find("(", end_text + 1);
    if (start_url == std::string::npos) {
      break;
    }

    size_t end_url = text.find(")", start_url + 1);
    if (end_url == std::string::npos) {
      break;
    }

    std::string link_text = text.substr(pos + 1, end_text - pos - 1);
    std::string url = text.substr(start_url + 1, end_url - start_url - 1);

    text.replace(pos, end_url - pos + 1,
                 "<a href=\"" + url + "\">" + link_text + "</a>");
  }
}

inline Paragraph::Paragraph(const std::string &text) : _text(text) {}

inline void Paragraph::write(std::ostream &out) const {
  std::string text = _text;
  size_t pos = text.length() - 1;

  while (pos > 0 && isspace(text[pos])) {
    --pos;
  }

  if (pos < text.length() - 2) {
    text.replace(pos + 1, text.length() - pos - 1, "<br />\n");
  }

  process_spans(text);

  out << "<p>" << text << "</p>" << std::endl;
}

inline CodeFence::CodeFence(const std::string &text) : _text(text) {}

inline void CodeFence::write(std::ostream &out) const {
  out << "<pre><code>" << _text << "</code></pre>" << std::endl;
}

inline BlockQuote::BlockQuote(const std::string &text) : _text(text) {}

inline void BlockQuote::write(std::ostream &out) const {
  out << "<blockquote>" << _text << "</blockquote>" << std::endl;
}

inline List::List(Type type) : _type(type) {}

inline void List::read(const std::string &in) {
  if (in.length() >= 2 && in.substr(0, 2) == "* ") {
    _items.emplace_back(in.substr(2));
  } else if (in.length() >= 2 && isdigit(in[0]) && in[1] == '.' &&
             in[2] == ' ') {
    _items.emplace_back(in.substr(3));
  }
}

inline void List::write(std::ostream &out) const {
  const char *tag = _type == Ordered ? "ol" : "ul";

  out << "<" << tag << ">" << std::endl;
  for (auto item : _items) {
    process_spans(item);
    out << "<li>" << item << "</li>" << std::endl;
  }
  out << "</" << tag << ">" << std::endl;
}

inline List::Type List::type() const { return _type; }
}

#endif // MARKDOWN_H
