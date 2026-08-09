// TextSegmenter.h
#pragma once
#include <string>
#include <vector>

namespace TextProcessor {

/**
 * 文本分词器（使用Jieba单例）
 */
class TextSegmenter {
public:
    TextSegmenter() = default;
    ~TextSegmenter() = default;
    
    /**
     * 文本分词 - 返回词列表（已过滤停用词）
     */
    std::vector<std::string> cutWords(const std::string& text, bool use_hmm = false);
    
    /**
     * 处理文本：清洗 + 分词 + 过滤 + 合并
     */
    std::string processText(const std::string& text, bool use_hmm = false);
};

} // namespace TextProcessor