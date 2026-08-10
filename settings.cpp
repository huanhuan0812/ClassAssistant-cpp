#include "settings.h"
#include <QSettings>

void settings::load() {
    // 读取设置文件
    QSettings qsettings("./settings.ini", QSettings::IniFormat);
    maxRecentFilsCnt = qsettings.value("maxRecentFilesCnt", 50).toInt();
    modelPath = qsettings.value("modelPath", "").toString();
    fileVocabPath = qsettings.value("fileVocabPath", "").toString();
    textVocabPath = qsettings.value("textVocabPath", "").toString();
    mixMapPath = qsettings.value("mixMapPath", "").toString();
    coursewareFolderPath = qsettings.value("coursewareFolderPath", "").toString();
    jiebaIdleTimeoutSeconds = qsettings.value("jiebaIdleTimeoutSeconds", 300).toInt(); // 默认5分钟
    dictPath = qsettings.value("jieba.dict.utf8", "").toString().toStdString();
    hmmPath = qsettings.value("hmm_model.utf8", "").toString().toStdString();
    userDictPath = qsettings.value("user.dict.utf8", "").toString().toStdString();
    idfPath = qsettings.value("idf.utf8", "").toString().toStdString();
    stopWordsPath = qsettings.value("stop_words.utf8", "").toString().toStdString();
}

void settings::save() {
    // 保存设置文件
    QSettings qsettings("./settings.ini", QSettings::IniFormat);
    qsettings.setValue("maxRecentFilesCnt", maxRecentFilsCnt);
    qsettings.setValue("modelPath", modelPath);
    qsettings.setValue("fileVocabPath", fileVocabPath);
    qsettings.setValue("textVocabPath", textVocabPath);
    qsettings.setValue("mixMapPath", mixMapPath);
    qsettings.setValue("coursewareFolderPath", coursewareFolderPath);
    qsettings.setValue("jiebaIdleTimeoutSeconds", jiebaIdleTimeoutSeconds); // 保存Jieba闲置超时秒数
    qsettings.setValue("jieba.dict.utf8", QString::fromStdString(dictPath));
    qsettings.setValue("hmm_model.utf8", QString::fromStdString(hmmPath));
    qsettings.setValue("user.dict.utf8", QString::fromStdString(userDictPath));
    qsettings.setValue("idf.utf8", QString::fromStdString(idfPath));
    qsettings.setValue("stop_words.utf8", QString::fromStdString(stopWordsPath));
}