#pragma once

#include <QWidget>
#include <FluentQt/FluentQt.h>

class FileView: public QWidget , public fluent::FluentElement
{
    Q_OBJECT

public:
    explicit FileView(QWidget *parent = nullptr);

signals:

private:
    int maxFileCount = 50; // 最大文件数量限制
    QVector<QString> recentFiles; // 存储最近文件的路径
};
