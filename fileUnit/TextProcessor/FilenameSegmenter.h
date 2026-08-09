// FilenameSegmenter.h
#pragma once
#include <string>
#include <vector>

namespace TextProcessor {

/**
 * 文件名分词器（使用Jieba单例）
 */
class FilenameSegmenter {
public:
    FilenameSegmenter() = default;
    ~FilenameSegmenter() = default;
    
    /**
     * 文件名分词 - 返回词列表
     */
    std::vector<std::string> cutFilename(const std::string& filename, bool use_hmm = false);
    
    /**
     * 处理文件名：从完整路径提取文件名 + 清洗 + 分词 + 过滤 + 合并
     */
    std::string processFilename(const std::string& filepath, bool use_hmm = false);
};

} // namespace TextProcessor