// TextCleaner.cpp
#include "TextCleaner.h"
#include <QString>
#include <QRegularExpression>

namespace TextProcessor {

std::string cleanText(const std::string& text) {
    if (text.empty()) {
        return "";
    }
    
    QString ustr = QString::fromStdString(text);
    
    // 移除URL
    static const QRegularExpression urlPattern(R"(https?://\S+|www\.\S+)");
    ustr.replace(urlPattern, "");
    
    // 保留指定字符集
    static const QRegularExpression invalidPattern(
        QStringLiteral("[^\\x{4e00}-\\x{9fa5}a-zA-Z0-9，。！？；：\"\"''（）【】《》、 ]")
    );
    ustr.replace(invalidPattern, "");
    
    // 合并空白
    static const QRegularExpression whitespacePattern(R"(\s+)");
    ustr.replace(whitespacePattern, " ");
    ustr = ustr.trimmed();
    
    return ustr.toStdString();
}

std::string cleanFilename(const std::string& filename) {
    if (filename.empty()) {
        return "";
    }
    
    QString ustr = QString::fromStdString(filename);
    
    // 保留中英文、数字，其他转空格
    static const QRegularExpression pattern(R"([^\x{4e00}-\x{9fa5}a-zA-Z0-9])");
    ustr.replace(pattern, " ");
    
    // 合并空白
    static const QRegularExpression whitespacePattern(R"(\s+)");
    ustr.replace(whitespacePattern, " ");
    ustr = ustr.trimmed();
    
    return ustr.toStdString();
}

} // namespace TextProcessor