#include "vocabulary_loader.h"

bool VocabularyLoader::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "VocabularyLoader: 无法打开文件:" << filePath;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "VocabularyLoader: JSON 解析失败:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "VocabularyLoader: 根节点不是对象";
        return false;
    }

    QJsonObject root = doc.object();

    // 读取 words_by_frequency 数组
    if (!root.contains("words_by_frequency") || !root["words_by_frequency"].isArray()) {
        qWarning() << "VocabularyLoader: 缺少 words_by_frequency 数组";
        return false;
    }

    QJsonArray wordsArray = root["words_by_frequency"].toArray();
    m_words.clear();
    m_words.reserve(wordsArray.size());
    m_rankMap.clear();
    m_rankMap.reserve(wordsArray.size());

    for (int i = 0; i < wordsArray.size(); ++i) {
        QString word = wordsArray[i].toString();
        m_words.append(word);
        m_rankMap.insert(word, i);  // 序号 = 数组索引（<UNK>为0）
    }

    qDebug() << "VocabularyLoader: 加载成功! 共" << m_words.size() << "个词";
    qDebug() << "  <UNK> 的序号为 0，未匹配返回 0";
    return true;
}

int VocabularyLoader::getRank(const QString &word) const
{
    return m_rankMap.value(word, 0);  // 未找到返回 0
}

QString VocabularyLoader::getWordByRank(int rank) const
{
    if (rank < 0 || rank >= m_words.size()) {
        return QString();
    }
    return m_words[rank];
}

const QVector<QString>& VocabularyLoader::getAllWords() const
{
    return m_words;
}

int VocabularyLoader::size() const
{
    return m_words.size();
}

bool VocabularyLoader::contains(const QString &word) const
{
    return m_rankMap.contains(word);
}

QHash<QString, int> VocabularyLoader::getRanks(const QStringList &words) const
{
    QHash<QString, int> result;
    result.reserve(words.size());
    for (const QString &word : words) {
        result.insert(word, getRank(word));
    }
    return result;
}

QStringList VocabularyLoader::getTopWords(int n) const
{
    QStringList result;
    // 从索引1开始（跳过<UNK>）
    int start = 1;
    int end = qMin(n, m_words.size() - 1);
    int count = end - start + 1;
    if (count <= 0) {
        return result;
    }
    result.reserve(count);
    for (int i = start; i <= end; ++i) {
        result.append(m_words[i]);
    }
    return result;
}

QStringList VocabularyLoader::getWordsByRankRange(int startRank, int endRank) const
{
    QStringList result;
    int start = qMax(1, startRank);
    int end = qMin(endRank, m_words.size() - 1);

    if (start > end) {
        return result;
    }

    result.reserve(end - start + 1);
    for (int rank = start; rank <= end; ++rank) {
        result.append(m_words[rank]);
    }
    return result;
}

void VocabularyLoader::printStatistics() const
{
    qDebug() << "========== 词汇统计 ==========";
    qDebug() << "总词数:" << m_words.size();
    qDebug() << "<UNK> 的序号为 0，未匹配返回 0";
    if (!m_words.isEmpty()) {
        qDebug() << "前10个高频词（从序号1开始）:";
        int count = qMin(10, m_words.size() - 1);
        for (int i = 1; i <= count; ++i) {
            qDebug() << QString("  序号%1: %2").arg(i).arg(m_words[i]);
        }
    }
    qDebug() << "================================";
}

void VocabularyLoader::clear()
{
    m_words.clear();
    m_rankMap.clear();
}