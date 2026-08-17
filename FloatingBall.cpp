#include "FloatingBall.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent)
{
    // ========== 窗口属性 ==========
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    // 固定大小（圆形）
    setFixedSize(70, 70);

    // ========== 阴影效果 ==========
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    // ========== 初始化位置（屏幕右下角） ==========
    QRect screenRect = QGuiApplication::primaryScreen()->geometry();
    move(screenRect.width() - width() - 30,
         screenRect.height() - height() - 80);

    // ========== 创建动画 ==========
    // 显示动画：透明度 0→1，缩放 0.3→1.0
    m_showAnimation = new QParallelAnimationGroup(this);

    QPropertyAnimation *opacityShow = new QPropertyAnimation(this, "opacity");
    opacityShow->setDuration(300);
    opacityShow->setStartValue(0.0);
    opacityShow->setEndValue(1.0);
    opacityShow->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *scaleShow = new QPropertyAnimation(this, "scale");
    scaleShow->setDuration(300);
    scaleShow->setStartValue(0.3);
    scaleShow->setEndValue(1.0);
    scaleShow->setEasingCurve(QEasingCurve::OutBack);  // 弹性效果

    m_showAnimation->addAnimation(opacityShow);
    m_showAnimation->addAnimation(scaleShow);

    // 隐藏动画：透明度 1→0，缩放 1.0→0.3
    m_hideAnimation = new QParallelAnimationGroup(this);

    QPropertyAnimation *opacityHide = new QPropertyAnimation(this, "opacity");
    opacityHide->setDuration(250);
    opacityHide->setStartValue(1.0);
    opacityHide->setEndValue(0.0);
    opacityHide->setEasingCurve(QEasingCurve::InCubic);

    QPropertyAnimation *scaleHide = new QPropertyAnimation(this, "scale");
    scaleHide->setDuration(250);
    scaleHide->setStartValue(1.0);
    scaleHide->setEndValue(0.3);
    scaleHide->setEasingCurve(QEasingCurve::InBack);

    m_hideAnimation->addAnimation(opacityHide);
    m_hideAnimation->addAnimation(scaleHide);

    // 隐藏动画结束后真正隐藏窗口
    connect(m_hideAnimation, &QParallelAnimationGroup::finished, this, &QWidget::hide);

    // 默认显示（初始状态为可见）
    setOpacity(1.0);
    setScale(1.0);
}

// ========== 公开方法 ==========
void FloatingBall::showWithAnimation()
{
    // 先重置到起始状态（透明 + 缩小）
    setOpacity(0.0);
    setScale(0.3);
    QWidget::show();  // 先显示（但透明看不见）
    m_showAnimation->start();
}

void FloatingBall::hideWithAnimation()
{
    if (isVisible()) {
        m_hideAnimation->start();
    }
}

// ========== 属性 setter/getter ==========
void FloatingBall::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();  // 触发重绘
}

void FloatingBall::setScale(qreal scale)
{
    m_scale = scale;
    // 缩放通过改变绘制大小实现，窗口本身大小不变
    update();
}

// ========== 绘制 ==========
void FloatingBall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 应用透明度
    painter.setOpacity(m_opacity);

    // 计算缩放后的绘制区域（居中缩放）
    int size = qRound(70 * m_scale);
    int x = (width() - size) / 2;
    int y = (height() - size) / 2;

    // 绘制渐变圆形
    QRadialGradient gradient(width()/2, height()/2, size/2);
    gradient.setColorAt(0, QColor(100, 200, 255, 230));
    gradient.setColorAt(0.7, QColor(30, 144, 255, 220));
    gradient.setColorAt(1, QColor(0, 80, 200, 200));

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(x, y, size, size);

    // 绘制高光
    painter.setBrush(QColor(255, 255, 255, 60));
    painter.drawEllipse(x + size*0.2, y + size*0.15, size*0.25, size*0.2);

    // 绘制图标文字
    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.setOpacity(m_opacity * 0.9);
    painter.drawText(QRect(x, y, size, size), Qt::AlignCenter, "⚡");
}

// ========== 拖拽逻辑 ==========
void FloatingBall::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bDragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bDragging = false;
        // 吸附边缘（可选）
        QRect screen = QGuiApplication::primaryScreen()->geometry();
        int x = pos().x();
        int y = pos().y();
        int margin = 10;

        if (x < margin) x = 0;
        else if (x + width() > screen.width() - margin)
            x = screen.width() - width();

        if (y < margin) y = 0;
        else if (y + height() > screen.height() - margin)
            y = screen.height() - height();

        move(x, y);
        event->accept();
    }
}