#pragma once

#include <QWidget>
#include <FluentQt/FluentQt.h>

class FileView: public QWidget , public fluent::FluentElement
{
    Q_OBJECT

public:
    explicit FileView(QWidget *parent = nullptr);

signals:
};
