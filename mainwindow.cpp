#include "mainwindow.h"

#include "SettingsView.h"
#include "OrganizeView.h"
#include "SearchView.h"
#include "FileView.h"

mainwindow::mainwindow(QWidget* parent)
    : fluent::windowing::Window(parent)
{
    setObjectName(QStringLiteral("mainWindow"));
    setWindowTitle(QStringLiteral("ClassAssistant"));
    setMinimumSize(800, 600);

    floatingBall = new FloatingBall(this);

    titlebar = this->titleBar();

    titleLabel = new fluent::textfields::Label(QStringLiteral("ClassAssistant"), titlebar);
    titleLabel->setFont(Typography::fontStyle(Typography::FontRole::Caption).toQFont());
    titlebar->setContentWidget(titleLabel);

    navView = new fluent::navigation::NavigationView(this);
    navView->setDisplayMode(fluent::navigation::NavigationView::DisplayMode::Left);
    navView->setExpandedPaneWidth(150);

    listView = new fluent::collections::ListView(navView);
    listView->setSelectionMode(fluent::collections::ListView::SelectionMode::Single);
    listView->setBackgroundVisible(false);
    listView->setBorderVisible(false);
    listView->setFontRole(Typography::FontRole::Body);
    listView->setSelectedIndicatorAnimationEnabled(true);

    listView->setItemDelegate(new NavigationDelegate(listView));

    listModel = new QStandardItemModel(listView);

    QList<QPair<QString, QString>> items = {
        {QStringLiteral("文件列表"), "ic_fluent_folder_16_regular"},
        {QStringLiteral("整理"), "ic_fluent_filter_12_regular"},
        {QStringLiteral("搜索"), "ic_fluent_search_12_regular"},
        {QStringLiteral("设置"), "ic_fluent_settings_20_regular"}
    };

    for (const auto& [text, iconName] : items) {
        auto* item = new QStandardItem(text);
        item->setData(iconName, Qt::UserRole + 1);
        listModel->appendRow(item);
    }

    listView->setModel(listModel);
    navView->setMainChromeWidget(listView);

    QVector<QWidget*> pages = {
        new FileView(navView),
        new OrganizeView(navView),
        new SearchView(navView),
        new SettingsView(navView)
    };

    for (int i = 0; i < pages.size(); ++i) {
        navView->contentHost()->insertPage(i, pages[i]);
    }
    navView->contentHost()->setCurrentIndex(0, 1, false);

    QObject::connect(listView, &fluent::collections::ListView::itemClicked,
        [this](int index) {
            // 直接调用，不通过信号槽机制
            navView->contentHost()->setCurrentIndex(index);
        });


    listView->setSelectedIndex(0);

    // 设置系统托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon/app.png"));
    trayIcon->show();

    QMenu* trayMenu = new QMenu(this);
    QAction* showAction = new QAction("Show", this);
    connect(showAction, &QAction::triggered, this, [this]() {
        this->show();
        this->raise();
        this->activateWindow();
        floatingBall->hideWithAnimation();  // 隐藏悬浮球
    });
    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, [this]() {
        qApp->quit();
    });
    trayMenu->addAction(showAction);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);



    this->setContentWidget(navView);
}

void mainwindow::closeEvent(QCloseEvent* event) {
    hide();
    //floatingBall->showWithAnimation();  // 显示悬浮球
    event->ignore();
}