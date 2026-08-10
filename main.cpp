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

using namespace fluent;
using namespace fluent::collections;
using namespace fluent::navigation;

// ============================================================
// 导航委托：使用 dynamic_cast 获取 FluentElement
// ============================================================
class NavigationDelegate : public QStyledItemDelegate {
public:
    explicit NavigationDelegate(QObject* parent = nullptr) 
        : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override {
        
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        
        // ✅ 使用 dynamic_cast（不是 qobject_cast）
        // FluentElement 不是 QObject，但可以用 dynamic_cast
        FluentElement* themeHost = nullptr;
        QWidget* widget = qobject_cast<QWidget*>(parent());
        while (widget && !themeHost) {
            themeHost = dynamic_cast<FluentElement*>(widget);
            if (!themeHost) {
                widget = widget->parentWidget();
            }
        }
        
        // 如果找到了 FluentElement，使用主题颜色
        if (themeHost) {
            paintWithTheme(painter, option, index, themeHost);
        } else {
            // 降级方案：使用 QPalette
            paintWithPalette(painter, option, index);
        }
        
        painter->restore();
    }
    
    QSize sizeHint(const QStyleOptionViewItem& option, 
                   const QModelIndex& index) const override {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(1, 36);  // Gallery 的行高 kRouteHeight = 36
    }
    
private:
    void paintWithTheme(QPainter* painter, const QStyleOptionViewItem& option,
                        const QModelIndex& index, FluentElement* theme) const {
        
        const auto& colors = theme->themeColorsRef();
        const auto radius = theme->themeRadius();
        bool isSelected = option.state & QStyle::State_Selected;
        bool isHovered = option.state & QStyle::State_MouseOver;
        
        // ============================================================
        // 1. 绘制行背景（带圆角）
        // ============================================================
        QRectF bgRect = QRectF(option.rect).adjusted(4, 2, -12, -2);
        QColor bgColor = Qt::transparent;
        if (option.state & QStyle::State_Sunken)
            bgColor = colors.subtleTertiary;
        else if (isSelected || isHovered)
            bgColor = colors.subtleSecondary;
            
        if (bgColor.alpha() > 0) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);
            painter->drawRoundedRect(bgRect, radius.control, radius.control);
        }
        
        
        
        // ============================================================
        // 2. ★★★ 绘制矢量图标（直接用 QFont）★★★
        // ============================================================
        QString iconName = index.data(Qt::UserRole + 1).toString();
        if (!iconName.isEmpty()) {
            int iconSize = 16;
            QFont iconFont = Typography::Icons::font(iconSize);
            QString glyph = Typography::Icons::glyph(iconName);
            
            QRect iconRect(option.rect.left() + 19,
                          option.rect.center().y() - iconSize / 2,
                          iconSize, iconSize);
            
            painter->setFont(iconFont);
            painter->setPen(isSelected ? colors.textPrimary : colors.textSecondary);
            painter->drawText(iconRect, Qt::AlignCenter, glyph);
        }
        
        // ============================================================
        // 3. 绘制文本
        // ============================================================
        QString text = index.data(Qt::DisplayRole).toString();
        if (!text.isEmpty()) {
            QFont textFont = theme->themeFont(Typography::FontRole::Body).toQFont();
            textFont.setPixelSize(Typography::FontSize::Body);
            painter->setFont(textFont);
            painter->setPen(isSelected ? colors.textPrimary : colors.textSecondary);
            
            QRect textRect(option.rect.left() + 43,
                          option.rect.top(),
                          option.rect.width() - 55,
                          option.rect.height());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
    }
    
    void paintWithPalette(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const {
        // 降级方案：使用 QPalette
        QPalette pal = option.palette;
        bool isDark = pal.window().color().lightness() < 128;
        bool isSelected = option.state & QStyle::State_Selected;
        bool isHovered = option.state & QStyle::State_MouseOver;
        
        QColor accentColor = pal.highlight().color();
        QColor textPrimary = pal.text().color();
        QColor textSecondary = pal.text().color();
        textSecondary.setAlpha(153);
        
        QColor subtleSecondary = isDark 
            ? QColor(255, 255, 255, 30) 
            : QColor(0, 0, 0, 30);
        QColor subtleTertiary = isDark 
            ? QColor(255, 255, 255, 60) 
            : QColor(0, 0, 0, 60);
        
        qreal radius = 4.0;
        
        // 背景
        QRectF bgRect = QRectF(option.rect).adjusted(4, 2, -12, -2);
        QColor bgColor = Qt::transparent;
        if (option.state & QStyle::State_Sunken)
            bgColor = subtleTertiary;
        else if (isSelected || isHovered)
            bgColor = subtleSecondary;
            
        if (bgColor.alpha() > 0) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);
            painter->drawRoundedRect(bgRect, radius, radius);
        }
        
        // 选中指示器
        if (isSelected) {
            QRectF indicatorRect(option.rect.left() + 4, 
                                option.rect.center().y() - 7,
                                3, 14);
            painter->setPen(Qt::NoPen);
            painter->setBrush(accentColor);
            painter->drawRoundedRect(indicatorRect, 1.5, 1.5);
        }
        
        // 图标
        QString iconName = index.data(Qt::UserRole + 1).toString();
        if (!iconName.isEmpty()) {
            int iconSize = 16;
            QFont iconFont = Typography::Icons::font(iconSize);
            QString glyph = Typography::Icons::glyph(iconName);
            
            QRect iconRect(option.rect.left() + 19,
                          option.rect.center().y() - iconSize / 2,
                          iconSize, iconSize);
            
            painter->setFont(iconFont);
            painter->setPen(isSelected ? textPrimary : textSecondary);
            painter->drawText(iconRect, Qt::AlignCenter, glyph);
        }
        
        // 文本
        QString text = index.data(Qt::DisplayRole).toString();
        if (!text.isEmpty()) {
            QFont textFont = option.font;
            textFont.setPixelSize(14);
            painter->setFont(textFont);
            painter->setPen(isSelected ? textPrimary : textSecondary);
            
            QRect textRect(option.rect.left() + 43,
                          option.rect.top(),
                          option.rect.width() - 55,
                          option.rect.height());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
    }
};

// ============================================================
// 主函数
// ============================================================
int main(int argc, char* argv[]) {
    fluent::prepareHighDpiApplication();
    QApplication app(argc, argv);
    fluent::initializeResources();
    settings::instance().load();

    // 初始化 JiebaSingleton
    auto& jieba = TextProcessor::JiebaSingleton::getInstance();
    jieba.setDictPaths(
        settings::instance().dictPath,
        settings::instance().hmmPath,
        settings::instance().userDictPath,
        settings::instance().idfPath,
        settings::instance().stopWordsPath
    );

    app.setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());

    fluent::windowing::Window window;
    window.setWindowTitle(QStringLiteral("ClassAssistant"));
    window.resize(800, 600);

    auto* titleBar = window.titleBar();
    QLabel* titleLabel = new QLabel(QStringLiteral("ClassAssistant"), titleBar);
    titleLabel->setFont(Typography::fontStyle(Typography::FontRole::Caption).toQFont());
    titleBar->setContentWidget(titleLabel);

    auto* navView = new NavigationView(&window);
    navView->setDisplayMode(NavigationView::DisplayMode::Left);
    navView->setExpandedPaneWidth(150);

    auto* listView = new ListView(navView);
    listView->setSelectionMode(ListView::SelectionMode::Single);
    listView->setBackgroundVisible(false);
    listView->setBorderVisible(false);
    listView->setFontRole(Typography::FontRole::Body);
    listView->setSelectedIndicatorAnimationEnabled(true);
    
    // ★ 使用自定义委托
    listView->setItemDelegate(new NavigationDelegate(listView));
    
    auto* model = new QStandardItemModel(listView);
    
    QList<QPair<QString, QString>> items = {
        {QStringLiteral("文件列表"), "ic_fluent_folder_16_regular"},
        {QStringLiteral("整理"), "ic_fluent_filter_12_regular"},
        {QStringLiteral("搜索"), "ic_fluent_search_12_regular"},
        {QStringLiteral("设置"), "ic_fluent_settings_20_regular"}
    };
    
    for (const auto& [text, iconName] : items) {
        auto* item = new QStandardItem(text);
        item->setData(iconName, Qt::UserRole + 1);
        model->appendRow(item);
    }
    
    listView->setModel(model);
    navView->setMainChromeWidget(listView);

    QVector<QWidget*> pages = {
        new FileView(navView),
        new OrganizeView(navView),
        new SearchView(navView),
        new SettingsView(&window)
    };

    for (int i = 0; i < pages.size(); ++i) {
        navView->contentHost()->insertPage(i, pages[i]);
    }
    navView->contentHost()->setCurrentIndex(0, 1, false);

    QObject::connect(listView, &ListView::itemClicked,
        [navView](int index) {
            navView->contentHost()->setCurrentIndex(index, 1, true);
        });

    listView->setSelectedIndex(0);

    window.setContentWidget(navView);
    window.show();
    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        settings::instance().save();
    });
    return app.exec();
}