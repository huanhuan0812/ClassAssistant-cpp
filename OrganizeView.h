#pragma once

#include <QWidget>
#include <FluentQt/FluentQt.h>

class OrganizeView: public QWidget , public fluent::FluentElement
{
    Q_OBJECT

public:
    explicit OrganizeView(QWidget *parent = nullptr);

signals:
};
