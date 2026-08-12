#pragma once
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QStringList>

enum class FileType {
    Unknown,
    Text,
    Image,
    Video,
    Audio,
    Docx,
    PPtx,
    PDF,
    Archive,
    Executable
};

struct fileData {
    QString filePath;
    FileType fileType;
    QDateTime lastModified;
    QStringList tags; // 用于存储文件的标签
};

class tempdata
{
public:
    static tempdata& instance(){
        static tempdata instance;
        return instance;
    };
    tempdata(const tempdata&) = delete;
    tempdata& operator=(const tempdata&) = delete;

    QVector<fileData> allFiles; // 存储所有文件的路径
private:
    tempdata()=default;
    ~tempdata()=default;
};