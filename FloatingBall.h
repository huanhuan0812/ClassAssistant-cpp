#ifndef FLOATINGBALL_H
#define FLOATINGBALL_H

#include <QWidget>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class FloatingBall : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)   // 透明度属性
    Q_PROPERTY(qreal scale READ scale WRITE setScale)         // 缩放属性

public:
    explicit FloatingBall(QWidget *parent = nullptr);

    // 显示动画（淡入 + 放大）
    void showWithAnimation();
    // 隐藏动画（淡出 + 缩小）
    void hideWithAnimation();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    // 属性的 getter/setter
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    qreal scale() const { return m_scale; }
    void setScale(qreal scale);

private:
    QPoint m_dragPosition;
    bool m_bDragging = false;

    qreal m_opacity = 1.0;
    qreal m_scale = 1.0;

    QParallelAnimationGroup *m_showAnimation;  // 显示动画组
    QParallelAnimationGroup *m_hideAnimation;  // 隐藏动画组
};

#endif // FLOATINGBALL_H