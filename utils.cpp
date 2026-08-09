#include "utils.h"
#include <QDebug>

// 实现
QFont IconUtils::getIconFont(int size) {
    return Typography::Icons::font(size);
}

QString IconUtils::getIconGlyph(const QString& iconName) {
    if (iconName.isEmpty()) {
        return QString();
    }
    return Typography::Icons::glyph(iconName);
}

void IconUtils::drawIcon(QPainter* painter, 
                         const QString& iconName,
                         const QRect& rect,
                         const QColor& color,
                         Qt::Alignment alignment) {
    if (!painter || iconName.isEmpty() || rect.isEmpty()) {
        return;
    }
    
    QString glyph = getIconGlyph(iconName);
    if (glyph.isEmpty()) {
        qWarning() << "Icon not found:" << iconName;
        return;
    }
    
    // 计算图标大小（通常取矩形的最小边，但不要超过矩形）
    int size = qMin(rect.width(), rect.height());
    if (size <= 0) return;
    
    // 获取图标字体
    QFont iconFont = getIconFont(size);
    
    // 保存 painter 状态
    painter->save();
    
    // 设置字体和颜色
    painter->setFont(iconFont);
    painter->setPen(color);
    
    // 计算绘制位置（居中）
    QRect textRect = rect;
    if (alignment & Qt::AlignCenter) {
        // 已经居中
    } else if (alignment & Qt::AlignLeft) {
        textRect.setLeft(rect.left());
        textRect.setRight(rect.left() + size);
    } else if (alignment & Qt::AlignRight) {
        textRect.setLeft(rect.right() - size);
        textRect.setRight(rect.right());
    }
    
    if (alignment & Qt::AlignTop) {
        textRect.setTop(rect.top());
        textRect.setBottom(rect.top() + size);
    } else if (alignment & Qt::AlignBottom) {
        textRect.setTop(rect.bottom() - size);
        textRect.setBottom(rect.bottom());
    }
    
    // 绘制文字
    painter->drawText(textRect, alignment, glyph);
    
    // 恢复 painter 状态
    painter->restore();
}

void IconUtils::drawIcon(QPainter* painter,
                         const QString& iconName,
                         int x, int y,
                         int size,
                         const QColor& color) {
    QRect rect(x, y, size, size);
    drawIcon(painter, iconName, rect, color);
}