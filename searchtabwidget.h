#ifndef SEARCHTABWIDGET_H
#define SEARCHTABWIDGET_H
#include"commonheader.h"
#include"settingdialog.h"
#include"targetfilewidget.h"
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
    QCheckBox* isRegexBox = new QCheckBox();
    QCheckBox* isCaseSensitiveBox = new QCheckBox();
    QCheckBox* isEnabledBox = new QCheckBox();
    QPushButton* deleteBtn = new QPushButton();
    KeywordInfo* keywordInfo = nullptr;
    KeywordInfoWidget(KeywordInfo* keywordInfo = nullptr){
        this->setLayout(mainLayout);
        this->mainLayout->addWidget(keywordEdit);
        this->mainLayout->addWidget(colorWidget);
        this->mainLayout->addWidget(isRegexBox);
        this->mainLayout->addWidget(isCaseSensitiveBox);
        this->mainLayout->addWidget(isEnabledBox);
        this->mainLayout->addWidget(deleteBtn);
        if(keywordInfo)this->setData(keywordInfo);

        isRegexBox->setToolTip("Use Regex");
        isCaseSensitiveBox->setToolTip("Case Sensitive");
        isEnabledBox->setToolTip("Enable");

        auto pixmap = QPixmap(":/images/delete16.png");
        auto icon = QIcon(pixmap);
        deleteBtn->setIcon(icon);
        deleteBtn->setIconSize(pixmap.rect().size());
        deleteBtn->setStyleSheet("background-color: rgba(255, 255, 255, 0);");

        colorWidget->setMaximumWidth(25);
    }

    void setData(KeywordInfo* keywordInfo){
        if(this->keywordInfo){
            delete this->keywordInfo;
        }
        this->keywordInfo = keywordInfo;
        this->keywordEdit->setText(this->keywordInfo->keyword);
        this->colorWidget->setColor(this->keywordInfo->highlightColor);
        this->isRegexBox->setChecked(this->keywordInfo->isRegex);
        this->isCaseSensitiveBox->setChecked(this->keywordInfo->isCaseSensitive);
        this->isEnabledBox->setChecked(this->keywordInfo->isEnable);
        connect(this->keywordEdit, &QLineEdit::textChanged, [=](const QString& text){
            this->keywordInfo->keyword = text;
        });
        connect(this->keywordEdit, &QLineEdit::returnPressed, this, &KeywordInfoWidget::returnPressed);
        connect(this->colorWidget, &ColorWidget::colorChanged, [=](const QColor& color){
            this->keywordInfo->highlightColor = color;
        });
        connect(this->isRegexBox, &QCheckBox::stateChanged, [=]{
            this->keywordInfo->isRegex = this->isRegexBox->isChecked();
        });
        connect(this->isCaseSensitiveBox, &QCheckBox::stateChanged, [=]{
            this->keywordInfo->isCaseSensitive = this->isCaseSensitiveBox->isChecked();
        });
        connect(this->isEnabledBox, &QCheckBox::stateChanged, [=]{
           this->keywordInfo->isEnable = this->isEnabledBox->isChecked();
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

struct SearchResultItem{
    int index;
    QString filename;
    int rowNumber;
    QString data;
public:
    SearchResultItem(){

    }
    SearchResultItem(int _index, QString _filename, int _rowNumber, QString _data)
        :index(_index), filename(_filename), rowNumber(_rowNumber), data(_data){

    }

    QString toQString(){
        QString ret = QString("index: %1, filename: %2, rowNumber: %3, data: %4")
                .arg(QString::number(index), filename, QString::number(rowNumber), data);
        return ret;
    }

};
Q_DECLARE_METATYPE(SearchResultItem)

class SearchWorker : public QObject
{
    Q_OBJECT
signals:
    void finished();
public:
    QList<KeywordInfo*>andKeywords;
    QList<KeywordInfo*>notKeywords;
    QStandardItemModel* model;
    QStringList filter;
    bool isRunning = true;
    SearchWorker(QStandardItemModel* _model, const QStringList& _filter,
                 const QList<KeywordInfo*>& _andKeywords, const QList<KeywordInfo*>& _notKeywords)
        :model(_model), filter(_filter), andKeywords(_andKeywords), notKeywords(_notKeywords){
    }

    void doWork(){
        int index = 1;
        for(int i = 0; i < model->rowCount() && isRunning; i++){
            auto item = model->item(i);
            if(item->checkState() != Qt::CheckState::Checked)continue;
            QFileInfo fileInfo(item->text());
            if(fileInfo.exists()){
                if(fileInfo.isFile())
                    this->searchFile(fileInfo.filePath(), index);
                else if(fileInfo.isDir())
                    this->searchDirectory(fileInfo.filePath(), index);
            }
        }
        emit finished();
        qDebug()<<"finish";
    }

    void searchDirectory(const QString& filePath, int& index){
        if(!isRunning)return;
        QDir dir(filePath);
        QFileInfoList fileInfos = dir.entryInfoList(filter, QDir::Filters(QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot));
        for(auto fileInfo : fileInfos){
            if(!isRunning)return;
            if(fileInfo.isFile()){
                searchFile(fileInfo.filePath(), index);
            }
            else if(fileInfo.isDir()){
                searchDirectory(fileInfo.filePath(), index);
            }
        }
    }

    void searchFile(const QString& filePath, int& index){
        if(!isRunning)return;
        QFile file(filePath);
        auto flag = file.open(QIODevice::ReadOnly | QIODevice::Text);
        if(!flag){
            printf("open file error\n");
            return;
        }
        if(file.exists()){
            QTextStream stream(&file);
            stream.setCodec("utf8");
            int rowNumber = 1;
            while(!stream.atEnd() && isRunning){
                QString line = stream.readLine();
                if(isMatch(line)){
                    auto item = SearchResultItem(index++, filePath, rowNumber, line);
                    emit renderItemSignal(item);
                    QThread::currentThread()->msleep(10);
                }
                rowNumber++;
            }
        }
        file.close();
    }

    bool isMatch(const QString& line){
        // and keywords
        for(auto andKeyword : andKeywords){
            if((!andKeyword->isEnable) || andKeyword->keyword.isEmpty())continue;
            bool isRegex = andKeyword->isRegex;
            bool isCaseSensitive= andKeyword->isCaseSensitive;
            if(isRegex){
                QRegularExpression pattern(andKeyword->keyword);
                if(!isCaseSensitive){
                    pattern.setPatternOptions(pattern.patternOptions() | QRegularExpression::PatternOption::CaseInsensitiveOption);
                }
                if(!line.contains(pattern))return false;
            }
            else{
                auto sensitive = Qt::CaseSensitivity::CaseSensitive;
                if(!isCaseSensitive){
                    sensitive = Qt::CaseSensitivity::CaseInsensitive;
                }
                if(!line.contains(andKeyword->keyword, sensitive))return false;
            }
        }
        return true;
    }

signals:
    void renderItemSignal(const SearchResultItem& item);
public slots:
    void stop(){
        this->isRunning = false;
    }
};

class SearchTabWidget:public QWidget{
    Q_OBJECT

public slots:
    void renderItem(const SearchResultItem& item){
        QString templateText = "%1\t%2\t%3:\t%4";
        SearchSettings* settings = Searcher::searchSettings;
        QColor colors[] = {settings->indexColor, settings->filenameColor, settings->rowNumberColor, settings->keywordColor};
        auto highlight = [&](int begin, int end, const QColor& color, QTextCursor& cursor){
            cursor.setPosition(begin, QTextCursor::MoveAnchor);
            auto fmt = cursor.charFormat();
            cursor.setPosition(end, QTextCursor::KeepAnchor);
            fmt.setForeground(QBrush(color));
            cursor.setCharFormat(fmt);
        };
        QTextCursor cursor(displayEdit->document());
        auto oriCharFmt = cursor.charFormat();
        QString text = templateText
                .arg(QString::number(item.index), item.filename, QString::number(item.rowNumber), item.data);
        auto content = displayEdit->toPlainText();
        int indexBegin = 0;
        if(!content.isEmpty())
            indexBegin = displayEdit->toPlainText().size() + 1;
        displayEdit->appendPlainText(text);
        int indexEnd = indexBegin + QString::number(item.index).size();
        int filenameLastIndex = item.filename.lastIndexOf(QRegularExpression("[\\/]"));
//            qDebug()<<filenameLastIndex;
        int filenameBegin = indexEnd+1 + filenameLastIndex + 1;
        int filenameEnd = filenameBegin + item.filename.size() - filenameLastIndex - 1;
        int rowNumberBegin = filenameEnd + 1;
        int rowNumberEnd = rowNumberBegin + QString::number(item.rowNumber).size();
        int begins[] = {indexBegin, filenameBegin, rowNumberBegin};
        int ends[] = {indexEnd, filenameEnd, rowNumberEnd};
        for(int i = 0; i < 3; i++){
            highlight(begins[i], ends[i], colors[i], cursor);
        }

        for(auto keywordInfo : andKeywordInfos){
            if((!keywordInfo->isEnable) || keywordInfo->keyword.isEmpty())continue;
            if(keywordInfo->isRegex){
                auto pattern = QRegularExpression(keywordInfo->keyword);
                if(!keywordInfo->isCaseSensitive){
                    pattern.setPatternOptions(pattern.patternOptions() | QRegularExpression::PatternOption::CaseInsensitiveOption);
                }
                auto matches = pattern.globalMatch(item.data);
                while(matches.hasNext()){
                    auto match = matches.next();
                    auto matchBegin = match.capturedStart() + rowNumberEnd + 2;
                    auto matchEnd = matchBegin + match.capturedLength();
                    highlight(matchBegin, matchEnd, keywordInfo->highlightColor, cursor);
                }
            }
            else{
                Qt::CaseSensitivity sensitivity = Qt::CaseSensitivity::CaseSensitive;
                if(!keywordInfo->isCaseSensitive){
                    sensitivity = Qt::CaseSensitivity::CaseInsensitive;
                }
                int start = 0;
                while((start = item.data.indexOf(keywordInfo->keyword, start, sensitivity)) != -1){
                    auto matchBegin = start + rowNumberEnd + 2;
                    auto matchEnd = matchBegin + keywordInfo->keyword.size();
                    highlight(matchBegin, matchEnd, keywordInfo->highlightColor, cursor);
                    start += keywordInfo->keyword.size();
                }
            }
        }
    }
    void search(){
        if(this->searchState == IDLE){
            displayEdit->clear();

            SearchWorker* worker = new SearchWorker(model, filterEdit->text().split(';'),
                                                    this->andKeywordInfos, this->notKeywordInfos);
            QThread* searchThread = new QThread;
            worker->moveToThread(searchThread);
            connect(searchThread, &QThread::started, worker, &SearchWorker::doWork);

            // Take care of cleaning up when finished too
            connect(worker, &SearchWorker::finished, searchThread, &QThread::quit);
            connect(worker, &SearchWorker::finished, worker, &SearchWorker::deleteLater);
            connect(searchThread, &QThread::finished, searchThread, &QThread::deleteLater);
            connect(searchThread, &QThread::finished, searchThread, [=]{
                this->searchState = IDLE;
                this->searchBtn->setText("Search");
                this->searchBtn->disconnect();
                connect(this->searchBtn, &QPushButton::clicked, this, &SearchTabWidget::search);
            });
            connect(worker, &SearchWorker::renderItemSignal, this, &SearchTabWidget::renderItem);
            connect(this->searchBtn, &QPushButton::clicked, [=]{
                this->searchBtn->disconnect();
                this->searchState = IDLE;
                this->searchBtn->setText("Search");
                worker->stop();
                worker->deleteLater();
                connect(this->searchBtn, &QPushButton::clicked, this, &SearchTabWidget::search);
                searchThread->quit();
                searchThread->wait();
                searchThread->deleteLater();
            });

            searchThread->start();
            this->searchState = SEARCHING;
            this->searchBtn->setText("Stop");
        }
        else{
//            stopSearching();
        }

    }

    void stopSearching(){
        if(this->searchState == SEARCHING){
            this->searchState = IDLE;
            this->searchBtn->setText("Search");
            this->searchThread->quit();
        }
    }

    void onSettingChanged(){
        auto setting = Searcher::searchSettings;
        QPalette palette = this->displayEdit->palette();
        palette.setColor(QPalette::Base, setting->backgroundColor);
        palette.setColor(QPalette::Text, setting->textColor);
        this->displayEdit->setPalette(palette);
        this->displayEdit->setFont(setting->font);

    }
public:
    QThread* searchThread;
    enum SearchingState{IDLE, SEARCHING};
    SearchingState searchState = IDLE;
    QStandardItemModel* model = new QStandardItemModel();
    TargetFileWidget* targetFileWidget = new TargetFileWidget(model);
    QPushButton* searchBtn = new QPushButton("Search");
    QStackedLayout* displayLayout = new QStackedLayout();
    QPlainTextEdit* displayEdit = new QPlainTextEdit();

    // splitter
    QSplitter* targetFileSplitter = new QSplitter();
    QSplitter* displaySplitter = new QSplitter();

    // tab info
    QWidget* tabInfoWidget = new QWidget();
    QGridLayout* tabInfoLayout = new QGridLayout();
    QLabel* tabNameLabel = new QLabel("Name");
    QLineEdit* tabNameEdit = new QLineEdit();
    QLabel* filterLabel = new QLabel("Filter");
    QLineEdit* filterEdit = new QLineEdit("*.cs;");

    // and group
    QGroupBox* andKeywordGroup = new QGroupBox("And Key");
    QVBoxLayout* andKeywordGroupLayout = new QVBoxLayout();
    QPushButton* addAndKeywordBtn = new QPushButton("+");
    QList<KeywordInfo*>andKeywordInfos;


    // not group
    QGroupBox* notKeywordGroup = new QGroupBox("Not Key");
    QVBoxLayout* notKeywordGroupLayout = new QVBoxLayout();
    QPushButton* addNotKeywordBtn = new QPushButton("+");
    QList<KeywordInfo*>notKeywordInfos;

public:
    SearchTabWidget(){
        this->setLayout(displayLayout);

        displayLayout->addWidget(targetFileSplitter);
        // split target and display
        targetFileSplitter->addWidget(displaySplitter);
        targetFileSplitter->addWidget(targetFileWidget);
        targetFileSplitter->setOrientation(Qt::Vertical);
        targetFileSplitter->setStretchFactor(targetFileSplitter->indexOf(displaySplitter), 5);
        targetFileSplitter->setStretchFactor(targetFileSplitter->indexOf(targetFileWidget), 1);
        // split display and info
        displaySplitter->addWidget(displayEdit);
        displaySplitter->addWidget(tabInfoWidget);
        displaySplitter->setOrientation(Qt::Horizontal);
        displaySplitter->setStretchFactor(displaySplitter->indexOf(displayEdit), 4);
        displaySplitter->setStretchFactor(displaySplitter->indexOf(tabInfoWidget), 1);

        // tab info
        tabInfoWidget->setLayout(tabInfoLayout);
        tabInfoLayout->setAlignment(Qt::AlignTop);
        // tab name
        tabInfoLayout->addWidget(tabNameLabel, 0, 0);
        tabInfoLayout->addWidget(tabNameEdit, 0, 1);
        // filter
        tabInfoLayout->addWidget(filterLabel, 1, 0);
        tabInfoLayout->addWidget(filterEdit, 1, 1);
        // and group
        tabInfoLayout->addWidget(andKeywordGroup, 2, 0, 1, 2);
        andKeywordGroup->setLayout(andKeywordGroupLayout);
        andKeywordGroupLayout->addWidget(addAndKeywordBtn);
        connect(this->addAndKeywordBtn, &QPushButton::clicked, [=]{
            auto keywordInfo = new KeywordInfo();
            andKeywordInfos.append(keywordInfo);
            auto keywordInfoWidget = new KeywordInfoWidget(keywordInfo);
            andKeywordGroupLayout->addWidget(keywordInfoWidget);
            connect(keywordInfoWidget, &KeywordInfoWidget::returnPressed, this, &SearchTabWidget::search);
            connect(keywordInfoWidget, &KeywordInfoWidget::beDeleted, [=]{
                andKeywordGroupLayout->removeWidget(keywordInfoWidget);
                andKeywordInfos.removeOne(keywordInfoWidget->getData());
                delete keywordInfoWidget;
            });
        });
        // not group
        tabInfoLayout->addWidget(notKeywordGroup, 3, 0, 1, 2);
        notKeywordGroup->setLayout(notKeywordGroupLayout);
        notKeywordGroupLayout->addWidget(addNotKeywordBtn);
        connect(this->addNotKeywordBtn, &QPushButton::clicked, [=]{

        });
        // search
        tabInfoLayout->addWidget(searchBtn, 4, 0, 1, 2);

        displayEdit->setReadOnly(true);
        displayEdit->setWordWrapMode(QTextOption::NoWrap);
        displayEdit->setTabStopDistance(40);

        SearchSettings* settings = Searcher::searchSettings;
        QPalette palette;
        palette.setColor(QPalette::Base, settings->backgroundColor);
        palette.setColor(QPalette::Text, settings->textColor);
        displayEdit->setPalette(palette);
        displayEdit->setFont(settings->font);

        connect(searchBtn, &QPushButton::clicked, this, &SearchTabWidget::search);
        connect(tabNameEdit, &QLineEdit::textChanged, [&](const QString& name){
            emit nameChanged(name);
        });
    }
signals:
    void nameChanged(const QString& name);
};

#endif // SEARCHTABWIDGET_H
