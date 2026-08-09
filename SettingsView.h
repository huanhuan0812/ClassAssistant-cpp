#pragma once

#include "components/basicinput/Button.h"
#include "components/layout/Card.h"
#include "components/scrolling/ScrollView.h"
#include "components/textfields/Label.h"
#include "components/textfields/LineEdit.h"
#include <FluentQt/FluentQt.h>
#include <QVBoxLayout>

class SettingsView: public QWidget, public fluent::FluentElement
{
    Q_OBJECT

public:
    explicit SettingsView(QWidget *parent = nullptr);
    ~SettingsView();
private:
    fluent::scrolling::ScrollView *scrollView;

    QVBoxLayout* m_layout;

    fluent::textfields::Label *titleLabel,*recentFilesSettingsLabel,*coursewareSettingsLabel,*modelSettingsLabel;

    fluent::textfields::Label *recentFilesCntLabel;
    fluent::textfields::NumberBox *recentFilesCntLineEdit;


    fluent::textfields::Label *coursewareFolderPathLabel;
    fluent::textfields::LineEdit *coursewareFolderPathLineEdit;
    fluent::basicinput::Button *coursewareFolderPathButton;

    fluent::layout::Card *modelPathCard,*fileVocabPathCard,*textVocabPathCard,*mixMapPathCard;
    fluent::textfields::Label *modelPathLabel,*fileVocabPathLabel,*textVocabPathLabel,*mixMapPathLabel;
    fluent::textfields::LineEdit *modelPathLineEdit,*fileVocabPathLineEdit,*textVocabPathLineEdit,*mixMapPathLineEdit;
    fluent::basicinput::Button *modelPathButton,*fileVocabPathButton,*textVocabPathButton,*mixMapPathButton;

    fluent::layout::Card *recentFilesSettingsCard,*coursewareSettingsCard;

    void onThemeUpdated() override;
    void paintEvent(QPaintEvent* event) override;

signals:
};
