#include <FluentQt/FluentQt.h>
#include <QApplication>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QLabel>
#include "SettingsView.h"
#include "OrganizeView.h"
#include "SearchView.h"
#include "FileView.h"
#include "settings.h"
#include "fileUnit/TextProcessor/JiebaSingleton.h"
#include "mainwindow.h"

using namespace fluent;
using namespace fluent::collections;
using namespace fluent::navigation;

// ============================================================
// 主函数
// ============================================================
int main(int argc, char* argv[]) {
    fluent::prepareHighDpiApplication();
    QApplication app(argc, argv);
    fluent::initializeResources();
    settings::instance().load();

    // 初始化 JiebaSingleton
    TextProcessor::JiebaSingleton::getInstance().initialize(
        settings::instance().dictPath,
        settings::instance().hmmPath,
        settings::instance().userDictPath,
        settings::instance().idfPath,
        settings::instance().stopWordsPath
    );

    app.setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());

    mainwindow window;
    window.show();
    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        settings::instance().save();
    });
    return app.exec();
}