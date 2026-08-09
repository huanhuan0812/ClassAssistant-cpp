// FilenameSegmenter.cpp
#include "FilenameSegmenter.h"
#include "JiebaSingleton.h"
#include "TextCleaner.h"
#include "Utils.h"
#include <sstream>
#include <algorithm>

namespace TextProcessor {

std::vector<std::string> FilenameSegmenter::cutFilename(const std::string& filename, bool use_hmm) {
    auto& jieba = JiebaSingleton::getInstance();
    
    if (!jieba.isReady()) {
        return {"空文本"};
    }
    
    // 清洗文件名
    std::string cleaned = cleanFilename(filename);
    if (cleaned.empty()) {
        return {"空文本"};
    }
    
    // 分词
    std::vector<std::string> words;
    jieba.getJieba()->Cut(cleaned, words, use_hmm);
    
    // 过滤空词
    words.erase(
        std::remove_if(words.begin(), words.end(),
            [](const std::string& w) { return trim(w).empty(); }),
        words.end()
    );
    
    // 如果分词结果为空，按空格分割（兜底）
    if (words.empty() && !cleaned.empty()) {
        std::istringstream iss(cleaned);
        std::string token;
        while (iss >> token) {
            words.push_back(token);
        }
    }
    
    if (words.empty()) {
        return {"空文本"};
    }
    
    // 过滤停用词，保留有意义单字
    const auto& stopwords = jieba.getStopwords();
    const auto& meaningful_chars = jieba.getMeaningfulSingleChars();
    
    std::vector<std::string> filtered;
    filtered.reserve(words.size());
    
    for (const auto& word : words) {
        std::string trimmed = trim(word);
        if (trimmed.empty()) {
            continue;
        }
        
        if (stopwords.find(trimmed) != stopwords.end()) {
            continue;
        }
        
        if (trimmed.length() > 1 || 
            meaningful_chars.find(trimmed) != meaningful_chars.end()) {
            filtered.push_back(trimmed);
        }
    }
    
    if (filtered.empty()) {
        filtered.push_back("空文本");
    }
    
    return filtered;
}

std::string FilenameSegmenter::processFilename(const std::string& filepath, bool use_hmm) {
    // 提取文件名（不含扩展名）
    size_t lastSlash = filepath.find_last_of("/\\");
    std::string filename = (lastSlash == std::string::npos) 
                           ? filepath 
                           : filepath.substr(lastSlash + 1);
    
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos) {
        filename = filename.substr(0, lastDot);
    }
    
    std::vector<std::string> words = cutFilename(filename, use_hmm);
    return joinWords(words, " ");
}

} // namespace TextProcessor