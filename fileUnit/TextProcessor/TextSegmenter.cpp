// TextSegmenter.cpp
#include "TextSegmenter.h"
#include "JiebaSingleton.h"
#include "TextCleaner.h"
#include "Utils.h"
#include <algorithm>

namespace TextProcessor {

std::vector<std::string> TextSegmenter::cutWords(const std::string& text, bool use_hmm) {
    auto& jieba = JiebaSingleton::getInstance();
    
    if (!jieba.isReady() || text.empty()) {
        return {"空文本"};
    }
    
    // 分词
    std::vector<std::string> words;
    jieba.getJieba()->Cut(text, words, use_hmm);
    
    // 过滤空词
    words.erase(
        std::remove_if(words.begin(), words.end(),
            [](const std::string& w) { return trim(w).empty(); }),
        words.end()
    );
    
    // 过滤停用词
    const auto& stopwords = jieba.getStopwords();
    std::vector<std::string> filtered;
    filtered.reserve(words.size());
    
    for (const auto& word : words) {
        std::string trimmed = trim(word);
        if (!trimmed.empty() && stopwords.find(trimmed) == stopwords.end()) {
            filtered.push_back(trimmed);
        }
    }
    
    if (filtered.empty()) {
        filtered.push_back("空文本");
    }
    
    return filtered;
}

std::string TextSegmenter::processText(const std::string& text, bool use_hmm) {
    std::string cleaned = cleanText(text);
    std::vector<std::string> words = cutWords(cleaned, use_hmm);
    return joinWords(words, " ");
}

} // namespace TextProcessor