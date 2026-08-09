#include "SettingsView.h"
#include "components/layout/Card.h"
#include "components/scrolling/ScrollView.h"
#include "components/textfields/Label.h"
#include "design/Typography.h"
#include <FluentQt/FluentQt.h>
#include <QtCore/qnamespace.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPainter>
#include "settings.h"
#include "utils.h"

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

SettingsView::SettingsView(QWidget *parent)
    : QWidget{parent}, fluent::FluentElement{}
{
    setObjectName(QStringLiteral("settingsView"));
    setAutoFillBackground(false);

    // 使 SettingsView 本身透明
    QPalette pagePalette = palette();
    pagePalette.setColor(QPalette::Window, Qt::transparent);
    setPalette(pagePalette);
    setStyleSheet(QStringLiteral(
        "#settingsView { background: transparent; }"));

    scrollView = new TransparentScrollView(this);
    scrollView->setObjectName(QStringLiteral("settingsScrollView"));
    scrollView->setHorizontalScrollMode(fluent::scrolling::ScrollView::ScrollMode::Disabled);
    scrollView->setVerticalScrollMode(fluent::scrolling::ScrollView::ScrollMode::Auto);
    scrollView->setVerticalScrollBarVisibility(fluent::scrolling::ScrollView::ScrollBarVisibility::Auto);
    scrollView->setZoomMode(fluent::scrolling::ScrollView::ZoomMode::Disabled);
    scrollView->setWidgetResizable(true);

    // 创建内容控件
    QWidget *contentWidget = new QWidget(scrollView);
    contentWidget->setObjectName(QStringLiteral("settingsContentWidget"));
    contentWidget->setAutoFillBackground(false);
    contentWidget->setStyleSheet(QStringLiteral(
        "#settingsContentWidget { background: transparent; }"));
    
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(25, 20, 25, 30);
    contentLayout->setAlignment(Qt::AlignTop);
    contentLayout->setSpacing(10);

    // 标题
    titleLabel = new fluent::textfields::Label(QStringLiteral("设置"), contentWidget);
    titleLabel->setObjectName(QStringLiteral("settingsTitleLabel"));
    titleLabel->setFont(Typography::fontStyle(Typography::FontRole::Title).toQFont());
    titleLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    //-------------------- 最近文件设置------------------------------

    recentFilesSettingsLabel = new fluent::textfields::Label(QStringLiteral("最近文件设置"), contentWidget);
    recentFilesSettingsLabel->setObjectName(QStringLiteral("recentFilesSettingsLabel"));
    recentFilesSettingsLabel->setFont(Typography::fontStyle(Typography::FontRole::BodyLargeStrong).toQFont());
    recentFilesSettingsLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    recentFilesSettingsCard = new fluent::layout::Card(contentWidget);
    recentFilesSettingsCard->setObjectName(QStringLiteral("recentFilesSettingsCard"));
    recentFilesSettingsCard->setAppearance(fluent::layout::Card::Layer);
    recentFilesSettingsCard->setFixedHeight(75);
    recentFilesSettingsCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *recentFilesLayout = new QHBoxLayout(recentFilesSettingsCard);
    recentFilesLayout->setContentsMargins(10, 10, 10, 10);
    recentFilesLayout->setSpacing(10);

    textfields::Label *recentFilesIconLabel = new textfields::Label(recentFilesSettingsCard);
    recentFilesIconLabel->setObjectName(QStringLiteral("recentFilesIconLabel"));
    recentFilesIconLabel->setFixedSize(30, 30);
    recentFilesIconLabel->setPixmap(IconUtils::generatePixmap("ic_fluent_text_number_list_ltr_16_regular", QColor(0, 0, 0), 20));
    recentFilesIconLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    recentFilesCntLabel = new fluent::textfields::Label(QStringLiteral("最大最近文件数"), recentFilesSettingsCard);
    recentFilesCntLabel->setObjectName(QStringLiteral("recentFilesCntLabel"));
    recentFilesCntLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    recentFilesCntLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    recentFilesCntLineEdit = new fluent::textfields::NumberBox(recentFilesSettingsCard);
    recentFilesCntLineEdit->setObjectName(QStringLiteral("recentFilesCntLineEdit"));
    recentFilesCntLineEdit->setValue(settings::instance().maxRecentFilsCnt);
    recentFilesCntLineEdit->setMinimum(1);
    recentFilesCntLineEdit->setSmallChange(1);
    recentFilesCntLineEdit->setFixedWidth(80);
    recentFilesCntLineEdit->setSpinButtonPlacementMode(
        fluent::textfields::NumberBox::SpinButtonPlacementMode::Compact);
    connect(recentFilesCntLineEdit, &fluent::textfields::NumberBox::valueChanged, this, [this](int newValue) {
        settings::instance().maxRecentFilsCnt = newValue;
    });

    recentFilesLayout->addWidget(recentFilesIconLabel);
    recentFilesLayout->addWidget(recentFilesCntLabel);
    recentFilesLayout->addStretch(1);
    recentFilesLayout->addWidget(recentFilesCntLineEdit);

    recentFilesSettingsCard->setLayout(recentFilesLayout);

    //-------------------- 课件文件夹设置------------------------------
    coursewareSettingsLabel = new fluent::textfields::Label(QStringLiteral("课件文件夹设置"), contentWidget);
    coursewareSettingsLabel->setObjectName(QStringLiteral("coursewareSettingsLabel"));
    coursewareSettingsLabel->setFont(Typography::fontStyle(Typography::FontRole::BodyLargeStrong).toQFont());
    coursewareSettingsLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    coursewareSettingsCard = new fluent::layout::Card(contentWidget);
    coursewareSettingsCard->setObjectName(QStringLiteral("coursewareSettingsCard"));
    coursewareSettingsCard->setAppearance(fluent::layout::Card::Layer);
    coursewareSettingsCard->setFixedHeight(75);
    coursewareSettingsCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *coursewareLayout = new QHBoxLayout(coursewareSettingsCard);
    coursewareLayout->setContentsMargins(10, 10, 10, 10);
    coursewareLayout->setSpacing(10);

    coursewareFolderPathLabel = new fluent::textfields::Label(QStringLiteral("课件文件夹路径"), coursewareSettingsCard);
    coursewareFolderPathLabel->setObjectName(QStringLiteral("coursewareFolderPathLabel"));
    coursewareFolderPathLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    coursewareFolderPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    coursewareFolderPathLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    coursewareFolderPathLineEdit = new fluent::textfields::LineEdit(coursewareSettingsCard);
    coursewareFolderPathLineEdit->setObjectName(QStringLiteral("coursewareFolderPathLineEdit"));
    coursewareFolderPathLineEdit->setText(settings::instance().coursewareFolderPath);
    coursewareFolderPathLineEdit->setReadOnly(true);
    coursewareFolderPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    coursewareFolderPathButton = new fluent::basicinput::Button(QStringLiteral("选择"), coursewareSettingsCard);
    coursewareFolderPathButton->setObjectName(QStringLiteral("coursewareFolderPathButton"));
    coursewareFolderPathButton->setFixedWidth(80);
    connect(coursewareFolderPathButton, &fluent::basicinput::Button::clicked, this, [this]() {
        QString folderPath = QFileDialog::getExistingDirectory(
            this, 
            QStringLiteral("选择课件文件夹"), 
            "", 
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!folderPath.isEmpty()) {
            coursewareFolderPathLineEdit->setText(folderPath);
            settings::instance().coursewareFolderPath = folderPath;
        }
    });

    coursewareLayout->addWidget(coursewareFolderPathLabel);
    coursewareLayout->addWidget(coursewareFolderPathLineEdit);
    coursewareLayout->addWidget(coursewareFolderPathButton);

    coursewareSettingsCard->setLayout(coursewareLayout);

    //-------------------- 模型设置------------------------------

    modelSettingsLabel = new fluent::textfields::Label(QStringLiteral("模型设置"), contentWidget);
    modelSettingsLabel->setObjectName(QStringLiteral("modelSettingsLabel"));
    modelSettingsLabel->setFont(Typography::fontStyle(Typography::FontRole::BodyLargeStrong).toQFont());
    modelSettingsLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    // 添加模型路径设置
    modelPathCard = new fluent::layout::Card(contentWidget);
    modelPathCard->setObjectName(QStringLiteral("modelPathCard"));
    modelPathCard->setAppearance(fluent::layout::Card::Layer);
    modelPathCard->setFixedHeight(75);
    modelPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *modelPathLayout = new QHBoxLayout(modelPathCard);
    modelPathLayout->setContentsMargins(10, 10, 10, 10);
    modelPathLayout->setSpacing(10);

    modelPathLabel = new fluent::textfields::Label(QStringLiteral("模型路径"), modelPathCard);
    modelPathLabel->setObjectName(QStringLiteral("modelPathLabel"));
    modelPathLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    modelPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    modelPathLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    
    modelPathLineEdit = new fluent::textfields::LineEdit(modelPathCard);
    modelPathLineEdit->setObjectName(QStringLiteral("modelPathLineEdit"));
    modelPathLineEdit->setText(settings::instance().modelPath);
    modelPathLineEdit->setReadOnly(true);
    modelPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    modelPathButton = new fluent::basicinput::Button(QStringLiteral("选择"), modelPathCard);
    modelPathButton->setObjectName(QStringLiteral("modelPathButton"));
    modelPathButton->setFixedWidth(80);
    connect(modelPathButton, &fluent::basicinput::Button::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, 
            QStringLiteral("选择模型文件"), 
            "", 
            "模型文件 (*.onnx)");
        if (!filePath.isEmpty()) {
            modelPathLineEdit->setText(filePath);
            settings::instance().modelPath = filePath;
        }
    });

    modelPathLayout->addWidget(modelPathLabel);
    modelPathLayout->addWidget(modelPathLineEdit);
    modelPathLayout->addWidget(modelPathButton);

    modelPathCard->setLayout(modelPathLayout);

    // 添加文件词汇表路径设置
    fileVocabPathCard = new fluent::layout::Card(contentWidget);
    fileVocabPathCard->setObjectName(QStringLiteral("fileVocabPathCard"));
    fileVocabPathCard->setAppearance(fluent::layout::Card::Layer);
    fileVocabPathCard->setFixedHeight(75);
    fileVocabPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *fileVocabPathLayout = new QHBoxLayout(fileVocabPathCard);
    fileVocabPathLayout->setContentsMargins(10, 10, 10, 10);
    fileVocabPathLayout->setSpacing(10);

    fileVocabPathLabel = new fluent::textfields::Label(QStringLiteral("文件词汇表路径"), fileVocabPathCard);
    fileVocabPathLabel->setObjectName(QStringLiteral("fileVocabPathLabel"));
    fileVocabPathLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    fileVocabPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    fileVocabPathLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    
    fileVocabPathLineEdit = new fluent::textfields::LineEdit(fileVocabPathCard);
    fileVocabPathLineEdit->setObjectName(QStringLiteral("fileVocabPathLineEdit"));
    fileVocabPathLineEdit->setText(settings::instance().fileVocabPath);
    fileVocabPathLineEdit->setReadOnly(true);
    fileVocabPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    fileVocabPathButton = new fluent::basicinput::Button(QStringLiteral("选择"), fileVocabPathCard);
    fileVocabPathButton->setObjectName(QStringLiteral("fileVocabPathButton"));
    fileVocabPathButton->setFixedWidth(80);
    connect(fileVocabPathButton, &fluent::basicinput::Button::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, 
            QStringLiteral("选择文件词汇表"), 
            "", 
            "词汇表文件 (*.json)");
        if (!filePath.isEmpty()) {
            fileVocabPathLineEdit->setText(filePath);
            settings::instance().fileVocabPath = filePath;
        }
    });

    fileVocabPathLayout->addWidget(fileVocabPathLabel);
    fileVocabPathLayout->addWidget(fileVocabPathLineEdit);
    fileVocabPathLayout->addWidget(fileVocabPathButton);

    fileVocabPathCard->setLayout(fileVocabPathLayout);

    // 添加文本词汇表路径设置
    textVocabPathCard = new fluent::layout::Card(contentWidget);
    textVocabPathCard->setObjectName(QStringLiteral("textVocabPathCard"));
    textVocabPathCard->setAppearance(fluent::layout::Card::Layer);
    textVocabPathCard->setFixedHeight(75);
    textVocabPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *textVocabPathLayout = new QHBoxLayout(textVocabPathCard);
    textVocabPathLayout->setContentsMargins(10, 10, 10, 10);
    textVocabPathLayout->setSpacing(10);

    textVocabPathLabel = new fluent::textfields::Label(QStringLiteral("文本词汇表路径"), textVocabPathCard);
    textVocabPathLabel->setObjectName(QStringLiteral("textVocabPathLabel"));
    textVocabPathLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    textVocabPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    textVocabPathLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    
    textVocabPathLineEdit = new fluent::textfields::LineEdit(textVocabPathCard);
    textVocabPathLineEdit->setObjectName(QStringLiteral("textVocabPathLineEdit"));
    textVocabPathLineEdit->setText(settings::instance().textVocabPath);
    textVocabPathLineEdit->setReadOnly(true);
    textVocabPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    textVocabPathButton = new fluent::basicinput::Button(QStringLiteral("选择"), textVocabPathCard);
    textVocabPathButton->setObjectName(QStringLiteral("textVocabPathButton"));
    textVocabPathButton->setFixedWidth(80);
    connect(textVocabPathButton, &fluent::basicinput::Button::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, 
            QStringLiteral("选择文本词汇表"), 
            "", 
            "词汇表文件 (*.json)");
        if (!filePath.isEmpty()) {
            textVocabPathLineEdit->setText(filePath);
            settings::instance().textVocabPath = filePath;
        }
    });

    textVocabPathLayout->addWidget(textVocabPathLabel);
    textVocabPathLayout->addWidget(textVocabPathLineEdit);
    textVocabPathLayout->addWidget(textVocabPathButton);

    textVocabPathCard->setLayout(textVocabPathLayout);

    // 添加混合映射路径设置
    mixMapPathCard = new fluent::layout::Card(contentWidget);
    mixMapPathCard->setObjectName(QStringLiteral("mixMapPathCard"));
    mixMapPathCard->setAppearance(fluent::layout::Card::Layer);
    mixMapPathCard->setFixedHeight(75);
    mixMapPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *mixMapPathLayout = new QHBoxLayout(mixMapPathCard);
    mixMapPathLayout->setContentsMargins(10, 10, 10, 10);
    mixMapPathLayout->setSpacing(10);

    mixMapPathLabel = new fluent::textfields::Label(QStringLiteral("科目映射表路径"), mixMapPathCard);
    mixMapPathLabel->setObjectName(QStringLiteral("mixMapPathLabel"));
    mixMapPathLabel->setFont(Typography::fontStyle(Typography::FontRole::Body).toQFont());
    mixMapPathLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    mixMapPathLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    
    mixMapPathLineEdit = new fluent::textfields::LineEdit(mixMapPathCard);
    mixMapPathLineEdit->setObjectName(QStringLiteral("mixMapPathLineEdit"));
    mixMapPathLineEdit->setText(settings::instance().mixMapPath);
    mixMapPathLineEdit->setReadOnly(true);
    mixMapPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    mixMapPathButton = new fluent::basicinput::Button(QStringLiteral("选择"), mixMapPathCard);
    mixMapPathButton->setObjectName(QStringLiteral("mixMapPathButton"));
    mixMapPathButton->setFixedWidth(80);
    connect(mixMapPathButton, &fluent::basicinput::Button::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, 
            QStringLiteral("选择科目映射表"), 
            "", 
            "映射文件 (*.json)");
        if (!filePath.isEmpty()) {
            mixMapPathLineEdit->setText(filePath);
            settings::instance().mixMapPath = filePath;
        }
    });

    mixMapPathLayout->addWidget(mixMapPathLabel);
    mixMapPathLayout->addWidget(mixMapPathLineEdit);
    mixMapPathLayout->addWidget(mixMapPathButton);

    mixMapPathCard->setLayout(mixMapPathLayout);

    //-------------------- 添加控件到布局------------------------------
    contentLayout->addWidget(titleLabel);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(recentFilesSettingsLabel);
    contentLayout->addWidget(recentFilesSettingsCard);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(coursewareSettingsLabel);
    contentLayout->addWidget(coursewareSettingsCard);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(modelSettingsLabel);
    contentLayout->addWidget(modelPathCard);
    contentLayout->addWidget(fileVocabPathCard);
    contentLayout->addWidget(textVocabPathCard);
    contentLayout->addWidget(mixMapPathCard);
    contentLayout->addSpacing(10);

    scrollView->setWidget(contentWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(scrollView);
    this->setLayout(mainLayout);
}

SettingsView::~SettingsView() {
}

void SettingsView::onThemeUpdated()
{
    fluent::FluentElement::onThemeUpdated();
    // 保持透明背景，让 Mica 效果透出
    setStyleSheet(QStringLiteral(
        "#settingsView { background: transparent; }"));
    if (scrollView && scrollView->viewport())
        scrollView->viewport()->update();
    update();
}

void SettingsView::paintEvent(QPaintEvent* event)
{
    // 使用透明绘制，让 Mica 效果透出
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(event->rect(), Qt::transparent);
}