#pragma once

#include <string>
#include "fileUnit/pptxunit.h"
#include "fileUnit/docxunit.h"
std::string inline processfile(const std::string filePath){
    if (filePath.substr(filePath.find_last_of(".") + 1) == "pptx") {
        return processPPTXFile(filePath);
    }
    else if (filePath.substr(filePath.find_last_of(".") + 1) == "docx") {
        return processDOCXFile(filePath);
    } 
    else {
        return "Unsupported file format";
    }
}