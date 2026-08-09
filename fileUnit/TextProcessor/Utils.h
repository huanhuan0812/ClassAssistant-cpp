// Utils.h
#pragma once
#include <string>
#include <vector>

namespace TextProcessor {

/**
 * 将词列表合并为空格分隔的字符串
 */
std::string joinWords(const std::vector<std::string>& words, const std::string& delimiter = " ");

/**
 * 去除字符串首尾空白
 */
std::string trim(const std::string& str);

} // namespace TextProcessor