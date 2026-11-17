#ifndef DOCUMENT_TREE_HPP
#define DOCUMENT_TREE_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <stdexcept>
#include <format>

namespace docengine {

// Forward declarations of concrete composite elements
class HeadingNode;
class ParagraphNode;
class CodeBlockNode;
class SectionNode;
class DocumentRoot;

// =========================================================================
// 1. The Abstract Visitor Interface
// =========================================================================
class IDocumentVisitor {
public:
    virtual ~IDocumentVisitor() = default;

    virtual void visit(HeadingNode& heading) = 0;
    virtual void visit(ParagraphNode& paragraph) = 0;
    virtual void visit(CodeBlockNode& code_block) = 0;
    virtual void visit(SectionNode& section) = 0;
    virtual void visit(DocumentRoot& root) = 0;
};

// =========================================================================
// 2. The Abstract Composite Node Base
// =========================================================================
class DocumentNode {
public:
    virtual ~DocumentNode() = default;

    // The double-dispatch acceptance entry point
    virtual void accept(IDocumentVisitor& visitor) = 0;

    // Optional composite child operations (default no-op for leaves)
    virtual void add(std::unique_ptr<DocumentNode> /*child*/) {
        throw std::runtime_error("Cannot add child elements to a leaf node.");
    }
};

// =========================================================================
// 3. Leaf Element Nodes
// =========================================================================

class HeadingNode final : public DocumentNode {
public:
    HeadingNode(int level, std::string text)
        : m_level(level), m_text(std::move(text)) {}

    void accept(IDocumentVisitor& visitor) override {
        visitor.visit(*this); // Dispatches on HeadingNode & Visitor type
    }

    [[nodiscard]] int level() const noexcept { return m_level; }
    [[nodiscard]] const std::string& text() const noexcept { return m_text; }

private:
    int         m_level;
    std::string m_text;
};

class ParagraphNode final : public DocumentNode {
public:
    explicit ParagraphNode(std::string text) : m_text(std::move(text)) {}

    void accept(IDocumentVisitor& visitor) override {
        visitor.visit(*this);
    }

    [[nodiscard]] const std::string& text() const noexcept { return m_text; }

private:
    std::string m_text;
};

class CodeBlockNode final : public DocumentNode {
public:
    CodeBlockNode(std::string language, std::string code)
        : m_language(std::move(language)), m_code(std::move(code)) {}

    void accept(IDocumentVisitor& visitor) override {
        visitor.visit(*this);
    }

    [[nodiscard]] const std::string& language() const noexcept { return m_language; }
    [[nodiscard]] const std::string& code() const noexcept { return m_code; }

private:
    std::string m_language;
    std::string m_code;
};

// =========================================================================
// 4. Composite Container Nodes
// =========================================================================

class SectionNode : public DocumentNode {
public:
    explicit SectionNode(std::string title) : m_title(std::move(title)) {}

    void accept(IDocumentVisitor& visitor) override {
        visitor.visit(*this);
    }

    void add(std::unique_ptr<DocumentNode> child) override {
        m_children.push_back(std::move(child));
    }

    [[nodiscard]] const std::string& title() const noexcept { return m_title; }
    [[nodiscard]] const std::vector<std::unique_ptr<DocumentNode>>& children() const noexcept {
        return m_children;
    }

private:
    std::string m_title;
    std::vector<std::unique_ptr<DocumentNode>> m_children;
};

class DocumentRoot final : public SectionNode {
public:
    explicit DocumentRoot(std::string doc_title) : SectionNode(std::move(doc_title)) {}

    void accept(IDocumentVisitor& visitor) override {
        visitor.visit(*this);
    }
};

} // namespace docengine

#endif // DOCUMENT_TREE_HPP