#pragma once
#include <QStringList>

class tempdata
{
public:
    static tempdata& instance(){
        static tempdata instance;
        return instance;
    };
    tempdata(const tempdata&) = delete;
    tempdata& operator=(const tempdata&) = delete;

    QStringList recentFiles; // 存储最近文件的路径
private:
    tempdata()=default;
    ~tempdata()=default;
};