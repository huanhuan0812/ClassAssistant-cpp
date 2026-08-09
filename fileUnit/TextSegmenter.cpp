// TextSegmenter.cpp
#include "TextSegmenter.h"
#include <QString>
#include <QRegularExpression>
#include <sstream>
#include <iostream>

namespace TextProcessor {

// ==================== cleanText 实现 ====================
std::string cleanText(const std::string& text) {
    if (text.empty()) {
        return "";
    }
    
    // 将std::string转为QString
    QString ustr = QString::fromStdString(text);
    
    // 1. 移除URL
    // 使用静态正则对象提高性能
    static const QRegularExpression urlPattern(R"(https?://\S+|www\.\S+)");
    ustr.replace(urlPattern, "");
    
    // 2. 保留指定字符集（中英文、数字、标点、空格）
    // Qt使用 \x{4e00} 表示Unicode字符，范围用 \x{4e00}-\x{9fa5}
    static const QRegularExpression invalidPattern(
        QStringLiteral("[^\\x{4e00}-\\x{9fa5}a-zA-Z0-9，。！？；：\"\"''（）【】《》、 ]")
    );
    ustr.replace(invalidPattern, "");
    
    // 3. 合并空白
    static const QRegularExpression whitespacePattern(R"(\s+)");
    ustr.replace(whitespacePattern, " ");
    
    // 去除首尾空白
    ustr = ustr.trimmed();
    
    return ustr.toStdString();
}

// ==================== cleanFilename 实现 ====================
std::string cleanFilename(const std::string& filename) {
    if (filename.empty()) {
        return "";
    }
    
    QString ustr = QString::fromStdString(filename);
    
    // 保留中英文、数字，其他转空格（与Python版本一致）
    static const QRegularExpression pattern(R"([^\x{4e00}-\x{9fa5}a-zA-Z0-9])");
    ustr.replace(pattern, " ");
    
    // 合并空白
    static const QRegularExpression whitespacePattern(R"(\s+)");
    ustr.replace(whitespacePattern, " ");
    
    // 去除首尾空白
    ustr = ustr.trimmed();
    
    return ustr.toStdString();
}

// ==================== joinWords 实现 ====================
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

// ==================== TextSegmenter 实现 ====================

TextSegmenter::TextSegmenter(
    const std::string& dict_path,
    const std::string& hmm_path,
    const std::string& user_dict_path,
    const std::string& idf_path,
    const std::string& stop_word_path
) {
    // 检查所有词典文件是否存在
    std::vector<std::pair<std::string, std::string>> files = {
        {"jieba.dict.utf8", dict_path},
        {"hmm_model.utf8", hmm_path},
        {"user.dict.utf8", user_dict_path},
        {"idf.utf8", idf_path},
        {"stop_words.utf8", stop_word_path}
    };
    
    for (const auto& [name, path] : files) {
        std::ifstream file(path);
        if (!file.is_open()) {
            init_error_ = "词典文件不存在: " + path + " (" + name + ")";
            initialized_ = false;
            std::cerr << "TextSegmenter 错误: " << init_error_ << std::endl;
            return;
        }
    }
    
    try {
        jieba_ = std::make_unique<cppjieba::Jieba>(
            dict_path, hmm_path, user_dict_path, idf_path, stop_word_path
        );
        initialized_ = true;
    } catch (const std::exception& e) {
        init_error_ = "Jieba 初始化失败: " + std::string(e.what());
        initialized_ = false;
        std::cerr << "TextSegmenter 错误: " << init_error_ << std::endl;
        jieba_ = nullptr;
        return;
    }
    
    initStopwords();
    initMeaningfulSingleChars();
}

void TextSegmenter::initStopwords() {
    // 与Python版本保持一致的停用词
    std::vector<std::string> stop_words = {
        "的", "了", "是", "我", "你", "他", "她", "它", "我们", "你们", "他们",
        "这", "那", "有", "在", "不", "和", "与", "就", "都", "而", "及", "或",
        "一个", "这个", "那个", "那些", "这些", "这里", "那里", "然后", "因为",
        "所以", "但是", "如果", "虽然", "然而", "并且", "或者"
    };
    stopwords_.insert(stop_words.begin(), stop_words.end());
}

void TextSegmenter::initMeaningfulSingleChars() {
    // 有意义的单字（与Python版本一致）
    std::vector<std::string> chars = {
        "圆", "力", "氧", "氢", "碳", "钠", "酸", "碱", "盐",
        "电", "光", "声", "热", "诗", "词", "歌", "曲", "数",
        "方", "程", "函", "数", "角", "形", "体", "积"
    };
    meaningful_single_chars_.insert(chars.begin(), chars.end());
}

std::string TextSegmenter::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> TextSegmenter::doSegment(const std::string& text, bool use_hmm) {
    std::vector<std::string> words;
    if (jieba_ == nullptr || text.empty()) {
        return words;
    }
    
    // 使用cppjieba分词
    jieba_->Cut(text, words, use_hmm);
    
    // 过滤空词
    words.erase(
        std::remove_if(words.begin(), words.end(),
            [](const std::string& w) { 
                return w.empty() || w.find_first_not_of(" \t\n\r") == std::string::npos; 
            }),
        words.end()
    );
    
    return words;
}

std::vector<std::string> TextSegmenter::filterStopwords(const std::vector<std::string>& words) {
    std::vector<std::string> filtered;
    filtered.reserve(words.size());
    
    for (const auto& word : words) {
        std::string trimmed = trim(word);
        if (trimmed.empty()) {
            continue;
        }
        
        // 检查是否在停用词表中
        if (stopwords_.find(trimmed) == stopwords_.end()) {
            filtered.push_back(trimmed);
        }
    }
    
    return filtered;
}

std::vector<std::string> TextSegmenter::cutWords(const std::string& text, bool use_hmm) {
    // 1. 分词
    std::vector<std::string> words = doSegment(text, use_hmm);
    
    // 2. 过滤停用词
    std::vector<std::string> filtered = filterStopwords(words);
    
    // 3. 如果过滤后为空，返回特殊标记
    if (filtered.empty()) {
        filtered.push_back("空文本");
    }
    
    return filtered;
}

std::string TextSegmenter::processText(const std::string& text, bool use_hmm) {
    // 1. 清洗文本
    std::string cleaned = cleanText(text);
    
    // 2. 分词 + 过滤
    std::vector<std::string> words = cutWords(cleaned, use_hmm);
    
    // 3. 合并为空格分隔的字符串
    return joinWords(words, " ");
}

// ==================== FilenameSegmenter 实现 ====================

FilenameSegmenter::FilenameSegmenter(TextSegmenter* text_segmenter)
    : text_segmenter_(text_segmenter) {
}

std::string FilenameSegmenter::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> FilenameSegmenter::filterStopwordsForFilename(const std::vector<std::string>& words) {
    if (text_segmenter_ == nullptr || !text_segmenter_->isReady()) {
        return words;
    }
    
    const auto& stopwords = text_segmenter_->getStopwords();
    const auto& meaningful_chars = text_segmenter_->getMeaningfulSingleChars();
    
    std::vector<std::string> filtered;
    filtered.reserve(words.size());
    
    for (const auto& word : words) {
        std::string trimmed = trim(word);
        if (trimmed.empty()) {
            continue;
        }
        
        // 检查是否在停用词表中
        if (stopwords.find(trimmed) != stopwords.end()) {
            continue;
        }
        
        // 保留长度>1的词，或是有意义的单字
        if (trimmed.length() > 1 || 
            meaningful_chars.find(trimmed) != meaningful_chars.end()) {
            filtered.push_back(trimmed);
        }
    }
    
    // 如果过滤后为空，返回特殊标记
    if (filtered.empty()) {
        filtered.push_back("空文本");
    }
    
    return filtered;
}

std::vector<std::string> FilenameSegmenter::cutFilename(const std::string& filename, bool use_hmm) {
    std::vector<std::string> empty_result;
    empty_result.push_back("空文本");
    
    if (text_segmenter_ == nullptr || !text_segmenter_->isReady()) {
        return empty_result;
    }
    
    cppjieba::Jieba* jieba = text_segmenter_->getJieba();
    if (jieba == nullptr) {
        return empty_result;
    }
    
    // 1. 清洗文件名
    std::string cleaned = cleanFilename(filename);
    if (cleaned.empty()) {
        return empty_result;
    }
    
    // 2. 分词
    std::vector<std::string> words;
    jieba->Cut(cleaned, words, use_hmm);
    
    // 过滤空词
    words.erase(
        std::remove_if(words.begin(), words.end(),
            [](const std::string& w) { 
                return w.empty() || w.find_first_not_of(" \t\n\r") == std::string::npos; 
            }),
        words.end()
    );
    
    // 3. 特殊处理：如果分词结果为空，尝试按字符切分（兜底）
    if (words.empty() && !cleaned.empty()) {
        // 按空格分割（如果清洗后还有空格）
        std::istringstream iss(cleaned);
        std::string token;
        while (iss >> token) {
            words.push_back(token);
        }
    }
    
    // 4. 如果还是为空，返回特殊标记
    if (words.empty()) {
        return empty_result;
    }
    
    // 5. 过滤停用词并保留有意义单字
    return filterStopwordsForFilename(words);
}

std::string FilenameSegmenter::processFilename(const std::string& filepath, bool use_hmm) {
    // 1. 从完整路径中提取文件名（不含扩展名）
    size_t lastSlash = filepath.find_last_of("/\\");
    std::string filename = (lastSlash == std::string::npos) 
                           ? filepath 
                           : filepath.substr(lastSlash + 1);
    
    // 2. 移除扩展名
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos) {
        filename = filename.substr(0, lastDot);
    }
    
    // 3. 分词 + 过滤
    std::vector<std::string> words = cutFilename(filename, use_hmm);
    
    // 4. 合并为空格分隔的字符串
    return joinWords(words, " ");
}

} // namespace TextProcessor