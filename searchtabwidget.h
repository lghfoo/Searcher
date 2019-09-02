#ifndef SEARCHTABWIDGET_H
#define SEARCHTABWIDGET_H
#include"commonheader.h"
#include"settingdialog.h"
#include"targetfilewidget.h"
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
    QString keyword;
    QStandardItemModel* model;
    QStringList filter;
    bool isCaseSensitive = false;
    bool isRegex = false;
    bool isRunning = true;
    SearchWorker(const QString& _keyword, QStandardItemModel* _model, const QStringList& _filter,
                 bool _isCaseSensitive, bool _isRegex)
        :keyword(_keyword), model(_model), filter(_filter),
        isCaseSensitive(_isCaseSensitive), isRegex(_isRegex){
    }

    void doWork(){
        int index = 1;
        for(int i = 0; i < model->rowCount() && isRunning; i++){
            auto item = model->item(i);
            if(item->checkState() != Qt::CheckState::Checked)continue;
            QFileInfo fileInfo(item->text());
            if(fileInfo.exists()){
                if(fileInfo.isFile())
                    this->searchFile(fileInfo.filePath(), index, keyword);
                else if(fileInfo.isDir())
                    this->searchDirectory(fileInfo.filePath(), index, keyword);
            }
        }
        emit finished();
        qDebug()<<"finish";
    }

    void searchDirectory(const QString& filePath, int& index, const QString& keyword){
        if(!isRunning)return;
        QDir dir(filePath);
        QFileInfoList fileInfos = dir.entryInfoList(filter, QDir::Filters(QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot));
        for(auto fileInfo : fileInfos){
            if(!isRunning)return;
            if(fileInfo.isFile()){
                searchFile(fileInfo.filePath(), index, keyword);
            }
            else if(fileInfo.isDir()){
                searchDirectory(fileInfo.filePath(), index, keyword);
            }
        }
    }

    void searchFile(const QString& filePath, int& index, const QString& keyword){
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
                if(contains(line, keyword, isCaseSensitive, isRegex)){
                    auto item = SearchResultItem(index++, filePath, rowNumber, line);
//                    resultItems.append(new SearchResultItem(index++, filePath, rowNumber, line));
                    emit renderItemSignal(item);
                    QThread::currentThread()->msleep(20);
                }
                rowNumber++;
            }
        }
        file.close();
    }

    bool contains(const QString& line, const QString& keyword, bool isCaseSensitive, bool isRegex){
        if(isRegex){
            QRegularExpression pattern(keyword);
            if(!isCaseSensitive){
                pattern.setPatternOptions(pattern.patternOptions() | QRegularExpression::PatternOption::CaseInsensitiveOption);
            }
            return line.contains(pattern);
        }
        else{
            auto sensitive = Qt::CaseSensitivity::CaseSensitive;
            if(!isCaseSensitive){
                sensitive = Qt::CaseSensitivity::CaseInsensitive;
            }
            return line.contains(keyword, sensitive);
        }

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
        auto keyword = keywordEdit->text();
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

        QList<int>matchBegins({}), matchEnds({});
        if(this->regexButton->isChecked()){
            auto pattern = QRegularExpression(keyword);
            if(!this->caseSensitiveButton->isChecked()){
                pattern.setPatternOptions(pattern.patternOptions() | QRegularExpression::PatternOption::CaseInsensitiveOption);
            }
            auto matches = pattern.globalMatch(item.data);
            while(matches.hasNext()){
                auto match = matches.next();
                auto matchBegin = match.capturedStart() + rowNumberEnd + 2;
                matchBegins.append(matchBegin);
                matchEnds.append(matchBegin + match.capturedLength());
            }
        }
        else{
            Qt::CaseSensitivity sensitivity = Qt::CaseSensitivity::CaseSensitive;
            if(!this->caseSensitiveButton->isChecked()){
                sensitivity = Qt::CaseSensitivity::CaseInsensitive;
            }
            int start = 0;
            while((start = item.data.indexOf(keyword, start, sensitivity)) != -1){
                matchBegins.append(start + rowNumberEnd + 2);
                matchEnds.append(start + rowNumberEnd + 2 + keyword.size());
                start += keyword.size();
            }
        }

        int begins[] = {indexBegin, filenameBegin, rowNumberBegin};
        int ends[] = {indexEnd, filenameEnd, rowNumberEnd};
        for(int i = 0; i < 3; i++){
            highlight(begins[i], ends[i], colors[i], cursor);
        }
        for(int i = 0; i < matchEnds.size(); i++){
            highlight(matchBegins[i], matchEnds[i], colors[3], cursor);
        }
//        cursor.setCharFormat(oriCharFmt);
    }
    void search(){
        if(this->searchState == IDLE){
            displayEdit->clear();

            SearchWorker* worker = new SearchWorker(keywordEdit->text(), model, filterEdit->text().split(';'),
                                                    this->caseSensitiveButton->isChecked(), this->regexButton->isChecked());
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
    QList<SearchResultItem*>resultItems;
    TargetFileWidget* targetFileWidget = new TargetFileWidget(model);
    QLineEdit* keywordEdit = new QLineEdit("int");
    QLineEdit* filterEdit = new QLineEdit("*.cs;");
    QLabel* filterLabel = new QLabel("Filter");
    QLineEdit* tabNameEdit = new QLineEdit();
    QLabel* tabNameLabel = new QLabel("Name");
    QLabel* keywordLabel = new QLabel("Keyword");
    QPushButton* searchBtn = new QPushButton("Search");
    QPushButton* regexButton = new QPushButton("Regex");
    QPushButton* caseSensitiveButton = new QPushButton("Case Sentive");
    QVBoxLayout* displayLayout = new QVBoxLayout();
    QPlainTextEdit* displayEdit = new QPlainTextEdit();
    QTableView* displayTable = new QTableView();
    QGridLayout* tabInfoLayout = new QGridLayout();

public:
    SearchTabWidget(){
        this->setLayout(displayLayout);
        displayLayout->addLayout(tabInfoLayout);
        tabInfoLayout->addWidget(tabNameLabel, 0, 0);
        tabInfoLayout->addWidget(tabNameEdit, 0, 1, 1, 4);
        tabInfoLayout->addWidget(keywordLabel, 1, 0);
        tabInfoLayout->addWidget(keywordEdit, 1, 1);
        tabInfoLayout->addWidget(searchBtn, 1, 2);
        tabInfoLayout->addWidget(regexButton, 1, 3);
        tabInfoLayout->addWidget(caseSensitiveButton, 1, 4);
        tabInfoLayout->addWidget(filterLabel, 2, 0);
        tabInfoLayout->addWidget(filterEdit, 2, 1, 1, 4);
        displayLayout->addWidget(displayEdit);
        displayLayout->addWidget(targetFileWidget);


        displayEdit->setReadOnly(true);
        displayEdit->setWordWrapMode(QTextOption::NoWrap);
        displayEdit->setTabStopDistance(40);
        regexButton->setCheckable(true);
        caseSensitiveButton->setCheckable(true);

        SearchSettings* settings = Searcher::searchSettings;
        QPalette palette;
        palette.setColor(QPalette::Base, settings->backgroundColor);
        palette.setColor(QPalette::Text, settings->textColor);
        displayEdit->setPalette(palette);
//        displayEdit->setFont(QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(fontFile)).at(0), fontSize));
        displayEdit->setFont(settings->font);

        connect(searchBtn, &QPushButton::clicked, this, &SearchTabWidget::search);
        connect(keywordEdit, &QLineEdit::returnPressed, this, &SearchTabWidget::search);
        connect(tabNameEdit, &QLineEdit::textChanged, [&](const QString& name){
            emit nameChanged(name);
        });
    }
signals:
    void nameChanged(const QString& name);
};

#endif // SEARCHTABWIDGET_H
