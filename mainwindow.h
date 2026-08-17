#pragma once

#include "components/collections/ListView.h"
#include "components/windowing/TitleBar.h"
#include <FluentQt/FluentQt.h>
#include <QStandardItemModel>
#include <QSystemTrayIcon>
#include "FloatingBall.h"

using namespace fluent;

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


class mainwindow : public fluent::windowing::Window {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);

    ~mainwindow() override = default;
private:
    fluent::windowing::TitleBar* titlebar;
    fluent::textfields::Label* titleLabel;
    fluent::navigation::NavigationView* navView;
    fluent::collections::ListView* listView;

    QSystemTrayIcon* trayIcon;

    QStandardItemModel* listModel;

    FloatingBall* floatingBall;  // 悬浮球实例
protected:
    void closeEvent(QCloseEvent* event) override;
};