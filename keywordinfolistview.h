#ifndef KEYWORDINFOLISTVIEW_H
#define KEYWORDINFOLISTVIEW_H
#include"settingdialog.h"
#include"util.h"
// button padding size (8, 11)

class KeywordInfo{
public:
    QString keyword = "";
    QColor highlightColor = Qt::red;
    bool isRegex = false;
    bool isCaseSensitive = false;
    bool isEnable = true;
};

class KeywordInfoWidget:public QWidget{
    Q_OBJECT
public:
    QHBoxLayout* mainLayout = new QHBoxLayout();
    QLineEdit* keywordEdit = new QLineEdit();
    ColorWidget* colorWidget = new ColorWidget();
    QPushButton* isRegexBtn = new QPushButton("R");
    QPushButton* isCaseSensitiveBtn = new QPushButton("C");
    QPushButton* isEnabledBtn = new QPushButton("E");
    QPushButton* deleteBtn = new QPushButton();
    KeywordInfo* keywordInfo = nullptr;
    KeywordInfoWidget(KeywordInfo* keywordInfo = nullptr){
        this->setLayout(mainLayout);
        this->mainLayout->addWidget(keywordEdit);
        this->mainLayout->addWidget(colorWidget);
        this->mainLayout->addWidget(isRegexBtn);
        this->mainLayout->addWidget(isCaseSensitiveBtn);
        this->mainLayout->addWidget(isEnabledBtn);
        this->mainLayout->addWidget(deleteBtn);

        isRegexBtn->setToolTip("Use Regex");
        isCaseSensitiveBtn->setToolTip("Case Sensitive");
        isEnabledBtn->setToolTip("Enable");

        auto pixmap = QPixmap(":/images/delete16.png");
        auto icon = QIcon(pixmap);
        deleteBtn->setIcon(icon);
        deleteBtn->setIconSize(pixmap.rect().size());
        deleteBtn->setStyleSheet("background-color: rgba(255, 255, 255, 0);");

        QPushButton* btns[] = {isRegexBtn, isCaseSensitiveBtn, isEnabledBtn};
        for(auto btn : btns){
            Util::shrinkButton(btn);
            btn->setCheckable(true);
        }
        colorWidget->setMaximumWidth(25);
        mainLayout->setSpacing(0);


        if(keywordInfo)this->setData(keywordInfo);
    }

    void setData(KeywordInfo* keywordInfo){
        if(this->keywordInfo){
            delete this->keywordInfo;
        }
        this->keywordInfo = keywordInfo;
        this->keywordEdit->setText(this->keywordInfo->keyword);
        this->colorWidget->setColor(this->keywordInfo->highlightColor);
        this->isRegexBtn->setChecked(this->keywordInfo->isRegex);
        this->isCaseSensitiveBtn->setChecked(this->keywordInfo->isCaseSensitive);
        this->isEnabledBtn->setChecked(this->keywordInfo->isEnable);
        connect(this->keywordEdit, &QLineEdit::textChanged, [=](const QString& text){
            this->keywordInfo->keyword = text;
        });
        connect(this->keywordEdit, &QLineEdit::returnPressed, this, &KeywordInfoWidget::returnPressed);
        connect(this->colorWidget, &ColorWidget::colorChanged, [=](const QColor& color){
            this->keywordInfo->highlightColor = color;
        });
        connect(this->isRegexBtn, &QPushButton::toggled, [=](bool checked){
            this->keywordInfo->isRegex = checked;
        });
        connect(this->isCaseSensitiveBtn, &QPushButton::toggled, [=](bool checked){
            this->keywordInfo->isCaseSensitive = checked;
        });
        connect(this->isEnabledBtn, &QPushButton::toggled, [=](bool checked){
           this->keywordInfo->isEnable = checked;
        });
        connect(this->deleteBtn, &QPushButton::clicked, this, &KeywordInfoWidget::beDeleted);

    }
    KeywordInfo* getData(){
        return this->keywordInfo;
    }
    ~KeywordInfoWidget(){
        delete keywordInfo;
        keywordInfo = nullptr;
    }
signals:
    void beDeleted();
    void returnPressed();
};

class KeywordInfoListView:public QGroupBox{
    Q_OBJECT
public:
    KeywordInfoListView(const QString& title = "Keyword Info List"){
        this->setLayout(keywordGroupLayout);
        this->setTitle(title);
        keywordGroupLayout->addWidget(addKeywordBtn);
        keywordGroupLayout->addWidget(keywordListScrollArea);
        keywordListScrollArea->setWidget(keywordListWidget);
        keywordListScrollArea->setWidgetResizable(true);
        keywordListWidget->setLayout(keywordListLayout);
        keywordListLayout->setAlignment(Qt::AlignTop);
        connect(this->addKeywordBtn, &QPushButton::clicked, [=]{
            this->addKeywordInfo(new KeywordInfo());
        });
    }
    const QList<KeywordInfo*>& getData(){
        return keywordInfos;
    }
    void addKeywordInfo(KeywordInfo* keywordInfo){
        keywordInfos.append(keywordInfo);
        auto keywordInfoWidget = new KeywordInfoWidget(keywordInfo);
        keywordListLayout->addWidget(keywordInfoWidget);
        connect(keywordInfoWidget, &KeywordInfoWidget::returnPressed, this, &KeywordInfoListView::returnPressed);
        connect(keywordInfoWidget, &KeywordInfoWidget::beDeleted, [=]{
            keywordListLayout->removeWidget(keywordInfoWidget);
            keywordInfos.removeOne(keywordInfoWidget->getData());
            delete keywordInfoWidget;
        });
    }

    ~KeywordInfoListView(){
        for(auto info : keywordInfos){
            delete info;
        }
        keywordInfos.clear();
    }
private:
    QVBoxLayout* keywordGroupLayout = new QVBoxLayout();
    QPushButton* addKeywordBtn = new QPushButton("+");
    QWidget* keywordListWidget = new QWidget();
    QVBoxLayout* keywordListLayout = new QVBoxLayout();
    QScrollArea* keywordListScrollArea = new QScrollArea();
    QList<KeywordInfo*>keywordInfos;
signals:
    void returnPressed();
};

#endif // KEYWORDINFOLISTVIEW_H
