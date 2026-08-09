#include "docxunit.h"
#include <docxcpp/document.hpp>

std::string processDOCXFile(const std::string filePath) {
    docxcpp::Document document(filePath);

    const auto paragraphs = document.paragraphs();
    std::string textContent;
    for (const auto& paragraph : paragraphs) {
        textContent += paragraph.text() + "\n";
    }
    return textContent;
}