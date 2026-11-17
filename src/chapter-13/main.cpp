#include "DocumentTree.hpp"
#include "Visitors.hpp"
#include <iostream>

int main() {
    using namespace docengine;

    std::cout << "========================================================\n";
    std::cout << "Composite, Visitor and Observer Patterns in Action\n";
    std::cout << "========================================================\n\n";

    // 1. Construct the Composite Document Tree
    auto document = std::make_unique<DocumentRoot>("Modern C++ Systems Architecture");

    // Add Section 1
    auto sec1 = std::make_unique<SectionNode>("Memory Subsystems");
    sec1->add(std::make_unique<HeadingNode>(2, "The Stack and Dynamic Heap"));
    sec1->add(std::make_unique<ParagraphNode>(
        "The stack provides high-speed LIFO allocation managed via the RSP hardware register, "
        "whereas the heap accommodates dynamic allocations requiring manual or RAII lifecycle management."
    ));
    sec1->add(std::make_unique<CodeBlockNode>("cpp", "std::unique_ptr<int> ptr = std::make_unique<int>(42);"));
    document->add(std::move(sec1));

    // Add Section 2
    auto sec2 = std::make_unique<SectionNode>("Behavioural Patterns");
    sec2->add(std::make_unique<HeadingNode>(2, "Double Dispatch with Visitors"));
    sec2->add(std::make_unique<ParagraphNode>(
        "The Visitor pattern decouples algorithms from data structures, resolving the Expression Problem."
    ));
    document->add(std::move(sec2));

    // -------------------------------------------------------------------------
    // 2. Execute Visitor 1: Markdown Export
    // -------------------------------------------------------------------------
    std::cout << "--- 1. MARKDOWN EXPORT VISITOR ---\n";
    MarkdownExportVisitor md_visitor;
    document->accept(md_visitor);
    std::cout << md_visitor.get_result();

    // -------------------------------------------------------------------------
    // 3. Execute Visitor 2: HTML5 Export
    // -------------------------------------------------------------------------
    std::cout << "--- 2. HTML5 EXPORT VISITOR ---\n";
    HtmlExportVisitor html_visitor;
    document->accept(html_visitor);
    std::cout << html_visitor.get_result() << "\n";

    // -------------------------------------------------------------------------
    // 4. Execute Visitor 3: Document Analytics
    // -------------------------------------------------------------------------
    std::cout << "--- 3. METRICS AND ANALYTICS VISITOR ---\n";
    WordCountMetricsVisitor metrics;
    document->accept(metrics);
    std::cout << std::format("Total Words Counted : {}\n", metrics.total_words());
    std::cout << std::format("Headings Encountered: {}\n", metrics.heading_count());
    std::cout << std::format("Paragraphs Analyzed : {}\n", metrics.paragraph_count());
    std::cout << std::format("Code Blocks Found   : {}\n\n", metrics.code_block_count());

    return 0;
}