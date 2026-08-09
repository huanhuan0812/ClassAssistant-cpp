#pragma once

#include <QWidget>
#include <FluentQt/FluentQt.h>

class SearchView: public QWidget , public fluent::FluentElement
{
    Q_OBJECT

public:
    explicit SearchView(QWidget *parent = nullptr);

signals:
};
