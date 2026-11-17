#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "DocumentTree.hpp"
#include <sstream>
#include <numeric>

namespace docengine {

// =========================================================================
// Visitor 1: Markdown Serialiser
// =========================================================================
class MarkdownExportVisitor final : public IDocumentVisitor {
public:
    void visit(HeadingNode& heading) override {
        m_ss << std::string(heading.level(), '#') << " " << heading.text() << "\n\n";
    }

    void visit(ParagraphNode& paragraph) override {
        m_ss << paragraph.text() << "\n\n";
    }

    void visit(CodeBlockNode& code_block) override {
        m_ss << "```" << code_block.language() << "\n"
             << code_block.code() << "\n```\n\n";
    }

    void visit(SectionNode& section) override {
        m_ss << "<!-- Section: " << section.title() << " -->\n";
        for (const auto& child : section.children()) {
            child->accept(*this); // Recursive composite traversal
        }
    }

    void visit(DocumentRoot& root) override {
        m_ss << "% Document: " << root.title() << "\n\n";
        for (const auto& child : root.children()) {
            child->accept(*this);
        }
    }

    [[nodiscard]] std::string get_result() const {
        return m_ss.str();
    }

private:
    std::stringstream m_ss;
};

// =========================================================================
// Visitor 2: HTML5 Semantic Serialiser
// =========================================================================
class HtmlExportVisitor final : public IDocumentVisitor {
public:
    void visit(HeadingNode& heading) override {
        m_ss << std::format("<h{0}>{1}</h{0}>\n", heading.level(), heading.text());
    }

    void visit(ParagraphNode& paragraph) override {
        m_ss << std::format("<p>{}</p>\n", paragraph.text());
    }

    void visit(CodeBlockNode& code_block) override {
        m_ss << std::format("<pre><code class=\"language-{}\">{}</code></pre>\n",
                            code_block.language(), code_block.code());
    }

    void visit(SectionNode& section) override {
        m_ss << "<section>\n";
        m_ss << std::format("  <h2>{}</h2>\n", section.title());
        for (const auto& child : section.children()) {
            child->accept(*this);
        }
        m_ss << "</section>\n";
    }

    void visit(DocumentRoot& root) override {
        m_ss << "<!DOCTYPE html>\n<html>\n<head>\n";
        m_ss << std::format("  <title>{}</title>\n", root.title());
        m_ss << "</head>\n<body>\n";
        for (const auto& child : root.children()) {
            child->accept(*this);
        }
        m_ss << "</body>\n</html>\n";
    }

    [[nodiscard]] std::string get_result() const {
        return m_ss.str();
    }

private:
    std::stringstream m_ss;
};

// =========================================================================
// Visitor 3: Analytics & Word Counter Metrics
// =========================================================================
class WordCountMetricsVisitor final : public IDocumentVisitor {
public:
    void visit(HeadingNode& heading) override {
        count_words(heading.text());
        ++m_heading_count;
    }

    void visit(ParagraphNode& paragraph) override {
        count_words(paragraph.text());
        ++m_paragraph_count;
    }

    void visit(CodeBlockNode& code_block) override {
        count_words(code_block.code());
        ++m_code_block_count;
    }

    void visit(SectionNode& section) override {
        count_words(section.title());
        for (const auto& child : section.children()) {
            child->accept(*this);
        }
    }

    void visit(DocumentRoot& root) override {
        count_words(root.title());
        for (const auto& child : root.children()) {
            child->accept(*this);
        }
    }

    [[nodiscard]] std::size_t total_words() const noexcept { return m_word_count; }
    [[nodiscard]] std::size_t heading_count() const noexcept { return m_heading_count; }
    [[nodiscard]] std::size_t paragraph_count() const noexcept { return m_paragraph_count; }
    [[nodiscard]] std::size_t code_block_count() const noexcept { return m_code_block_count; }

private:
    void count_words(std::string_view text) {
        std::stringstream ss{std::string(text)};
        std::string word;
        while (ss >> word) {
            ++m_word_count;
        }
    }

    std::size_t m_word_count{0};
    std::size_t m_heading_count{0};
    std::size_t m_paragraph_count{0};
    std::size_t m_code_block_count{0};
};

} // namespace docengine

#endif // VISITORS_HPP