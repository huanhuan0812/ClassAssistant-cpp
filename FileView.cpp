#include "FileView.h"
#include "components/collections/ListView.h"
#include "settings.h"
#include "tempdata.h"
#include <QtGui/qevent.h>
#include <QFile>
#include <QtWidgets/qboxlayout.h>

class TransparentScrollView : public fluent::scrolling::ScrollView {
public:
    explicit TransparentScrollView(QWidget* parent = nullptr)
        : fluent::scrolling::ScrollView(parent)
    {
        applyTransparentSurface();
    }

protected:
    void onThemeUpdated() override
    {
        fluent::scrolling::ScrollView::onThemeUpdated();
        applyTransparentSurface();
    }

private:
    void applyTransparentSurface()
    {
        setAutoFillBackground(false);
        QWidget* area = viewport();
        if (!area)
            return;

        area->setAutoFillBackground(false);
        area->setAttribute(Qt::WA_TranslucentBackground, false);
        area->setAttribute(Qt::WA_OpaquePaintEvent, false);
        QPalette palette = area->palette();
        palette.setColor(QPalette::Window, Qt::transparent);
        palette.setColor(QPalette::Base, Qt::transparent);
        area->setPalette(palette);
        area->update();
    }
};

class RecentFileCard : public fluent::layout::Card {
public:
    explicit RecentFileCard(QWidget* parent = nullptr,QString filePath = "")
        : fluent::layout::Card(parent)
    {
        m_filePath = filePath;
    }
private:
    QString m_filePath;

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) {
            // 处理点击事件，例如打开文件
            qDebug() << "Clicked on file: " << m_filePath;
        }
        if (event->button() == Qt::RightButton) {
            // 处理右键点击事件，例如显示上下文菜单
            qDebug() << "Right-clicked on file: " << m_filePath;
        }
        
        fluent::layout::Card::mousePressEvent(event);
    }

protected:
    void doubleClickEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton) {
            // 处理双击事件，例如打开文件
            qDebug() << "Double-clicked on file: " << m_filePath;
        }
        
        fluent::layout::Card::mouseDoubleClickEvent(event);
    }
    
};

FileView::FileView(QWidget *parent)
    : QWidget{parent} , fluent::FluentElement{}
{
    TransparentScrollView* scrollView = new TransparentScrollView(this);
    scrollView->setFrameShape(QFrame::NoFrame);
    scrollView->setObjectName(QStringLiteral("fileScrollView"));
    scrollView->setHorizontalScrollMode(fluent::scrolling::ScrollView::ScrollMode::Disabled);
    scrollView->setVerticalScrollMode(fluent::scrolling::ScrollView::ScrollMode::Auto);
    scrollView->setVerticalScrollBarVisibility(fluent::scrolling::ScrollView::ScrollBarVisibility::Auto);
    scrollView->setZoomMode(fluent::scrolling::ScrollView::ZoomMode::Disabled);
    scrollView->setWidgetResizable(true);

    QWidget *contentWidget = new QWidget(scrollView);
    contentWidget->setObjectName(QStringLiteral("fileContentWidget"));
    contentWidget->setAutoFillBackground(false);
    contentWidget->setStyleSheet(QStringLiteral("#fileContentWidget { background: transparent; }"));

    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    maxFileCount = settings::instance().maxRecentFilsCnt;
    
    for (int i = 0; i < tempdata::instance().allFiles.size() && i < maxFileCount; ++i) {
        QString filePath = tempdata::instance().allFiles.at(i).filePath;
        if (QFile::exists(filePath)) {
            recentFiles.append(filePath);
            RecentFileCard* card = new RecentFileCard(contentWidget, filePath);
            card->setObjectName(QStringLiteral("recentFileCard"));
            layout->addWidget(card);
        }
        
    }

    fluent::collections::ListView *listView = new fluent::collections::ListView(contentWidget);
    listView->setObjectName(QStringLiteral("recentFileListView"));

    listView->setCanReorderItems(false);
    listView->setDragEnabled(true);
    listView->setDragDropMode(QAbstractItemView::DragDrop);
    listView->setAcceptDrops(true);


    contentWidget->setLayout(layout);

    

    scrollView->setWidget(contentWidget);

    this->setLayout(new QVBoxLayout(this)); 
    this->layout()->addWidget(scrollView);
}
