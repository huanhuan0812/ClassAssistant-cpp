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
}