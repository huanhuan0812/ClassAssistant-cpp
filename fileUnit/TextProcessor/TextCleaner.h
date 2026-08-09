// TextCleaner.h
#pragma once
#include <string>

namespace TextProcessor {

/**
 * 基础文本清洗
 */
std::string cleanText(const std::string& text);

/**
 * 清洗文件名：只保留中英文、数字，其他转空格
 */
std::string cleanFilename(const std::string& filename);

} // namespace TextProcessor