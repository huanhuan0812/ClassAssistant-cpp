// Utils.cpp
#include "Utils.h"
#include <sstream>

namespace TextProcessor {

std::string joinWords(const std::vector<std::string>& words, const std::string& delimiter) {
    if (words.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) {
            oss << delimiter;
        }
        oss << words[i];
    }
    return oss.str();
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

} // namespace TextProcessor