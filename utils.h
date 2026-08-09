#pragma once

#include <QString>
#include <QFont>
#include <QPainter>
#include <QRect>
#include <QtGlobal>

#include <FluentQt/FluentQt.h>
#include <QtGui/qicon.h>

using namespace fluent;

/**
 * @brief 图标工具类 - 用于绘制 Fluent 字体图标
 * 
 * 使用方法:
 * @code
 *   // 在 paint 方法中
 *   IconUtils::drawIcon(painter, "ic_fluent_folder_16_regular", 
 *                       QRect(10, 10, 16, 16), QColor(0, 0, 0));
 * @endcode
 */
class IconUtils {
public:
    /**
     * @brief 获取图标字体
     * @param size 字体大小（像素）
     * @return QFont 图标字体对象
     */
    static QFont getIconFont(int size = 16);
    
    /**
     * @brief 获取图标的 Unicode 字符
     * @param iconName 图标名称（如 "ic_fluent_folder_16_regular"）
     * @return QString 对应的字符
     */
    static QString getIconGlyph(const QString& iconName);
    
    /**
     * @brief 在指定矩形中绘制图标
     * @param painter QPainter 对象
     * @param iconName 图标名称
     * @param rect 绘制区域
     * @param color 图标颜色
     * @param alignment 对齐方式（默认居中）
     */
    static void drawIcon(QPainter* painter, 
                         const QString& iconName,
                         const QRect& rect,
                         const QColor& color,
                         Qt::Alignment alignment = Qt::AlignCenter);
    
    /**
     * @brief 在指定位置绘制图标（自动计算居中位置）
     * @param painter QPainter 对象
     * @param iconName 图标名称
     * @param x 左上角 X 坐标
     * @param y 左上角 Y 坐标
     * @param size 图标大小
     * @param color 图标颜色
     */
    static void drawIcon(QPainter* painter,
                         const QString& iconName,
                         int x, int y,
                         int size,
                         const QColor& color);
    
    /**
     * @brief 生成 QIcon 对象
     * @param iconName 图标名称
     * @param color 图标颜色
     * @param size 图标大小（默认 16）
     * @return QIcon 对象
     */

    static QIcon generateIcon(const QString& iconName, const QColor& color, int size = 16) {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        drawIcon(&painter, iconName, QRect(0, 0, size, size), color);
        return QIcon(pixmap);
    }

    static QPixmap generatePixmap(const QString& iconName, 
                               const QColor& color, 
                               int size = 16) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    drawIcon(&painter, iconName, QRect(0, 0, size, size), color);
    return pixmap;
    }


private:
    IconUtils() = delete;  // 禁止实例化
};