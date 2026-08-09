#include "pptxunit.h"
#include <pptx/pptx.hpp>

std::string processPPTXFile(const std::string filePath) {
    try {
        pptx::Presentation pres=pptx::Presentation::open(filePath);
        std::string textContent;

        for (const auto& slide : pres.slides()) {
            for (const auto& shape : slide.shapes()) {
                for(const auto& paragraph : shape.paragraphs()) {
                    for (const auto& run : paragraph.runs()) {
                        textContent += run.text();
                    }
                    textContent += "\n"; // Add a newline after each paragraph
                }
            }
        }

        return textContent;
    } catch (const std::exception& e) {
        return "Error processing PPTX file: " + std::string(e.what());
    }
}