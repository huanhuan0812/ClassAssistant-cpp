#pragma  once

#include <QString>

//单例设置数据
class settings
{
public:
    static settings& instance(){
        static settings instance;
        return instance;
    };
    settings(const settings&) = delete;
    settings& operator=(const settings&) = delete;

    void load();
    void save();

    QString modelPath,fileVocabPath,textVocabPath,mixMapPath;
    QString coursewareFolderPath;
    int maxRecentFilsCnt=50;
    std::string dictPath,hmmPath,userDictPath,idfPath,stopWordsPath;

    int jiebaIdleTimeoutSeconds=300; // Jieba闲置超时秒数，默认5分钟
private:
    settings()=default;
    ~settings()=default;

    
};