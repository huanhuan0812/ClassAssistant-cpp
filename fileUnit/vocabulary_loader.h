#ifndef VOCABULARY_LOADER_H
#define VOCABULARY_LOADER_H

#include <QVector>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

/**
 * @brief 词汇加载器 - 从 JSON 文件加载按频率排序的词汇表
 * 
 * 只读取 words_by_frequency 数组
 * <UNK> 是第0个，真正的词汇从1开始
 * 未匹配到返回 0
 */
class VocabularyLoader
{
public:
    VocabularyLoader() = default;
    ~VocabularyLoader() = default;

    /**
     * @brief 从 JSON 文件加载词汇数据
     * @param filePath JSON 文件路径
     * @return 成功返回 true，失败返回 false
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief 根据词查询序号（<UNK>为0，真正的词从1开始）
     * @param word 要查询的词
     * @return 序号，未找到返回 0
     */
    int getRank(const QString &word) const;

    /**
     * @brief 根据序号获取词
     * @param rank 序号（0表示<UNK>，1表示第一个真正的词）
     * @return 对应的词，如果序号无效返回空字符串
     */
    QString getWordByRank(int rank) const;

    /**
     * @brief 获取所有词（按原始顺序）
     * @return 所有词的 QVector
     */
    const QVector<QString>& getAllWords() const;

    /**
     * @brief 获取总词数
     * @return 词汇总数
     */
    int size() const;

    /**
     * @brief 检查词是否存在
     * @param word 要检查的词
     * @return 存在返回 true
     */
    bool contains(const QString &word) const;

    /**
     * @brief 检查是否已成功加载数据
     * @return 已加载返回 true
     */
    bool isValid() const { return !m_words.isEmpty(); }

    // ========== 批量操作 ==========

    /**
     * @brief 批量查询多个词的序号
     * @param words 要查询的词列表
     * @return 词 -> 序号 的哈希表
     */
    QHash<QString, int> getRanks(const QStringList &words) const;

    /**
     * @brief 获取前 N 个词（从序号1开始，不含<UNK>）
     * @param n 要获取的数量
     * @return 前 N 个词的列表
     */
    QStringList getTopWords(int n) const;

    /**
     * @brief 获取序号在 [startRank, endRank] 区间的词（从1开始）
     * @param startRank 起始序号（1-based）
     * @param endRank 结束序号（1-based）
     * @return 区间内的词列表
     */
    QStringList getWordsByRankRange(int startRank, int endRank) const;

    // ========== 调试与统计 ==========

    /**
     * @brief 打印统计信息到控制台
     */
    void printStatistics() const;

    /**
     * @brief 清空所有数据
     */
    void clear();

private:
    QVector<QString> m_words;        // 按频率排序的词列表（索引0对应<UNK>）
    QHash<QString, int> m_rankMap;   // 词 -> 序号（<UNK>为0，真正的词从1开始）
};

#endif // VOCABULARY_LOADER_H