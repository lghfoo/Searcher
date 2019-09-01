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
    SearchResultItem(int _index, QString _filename, int _rowNumber, QString _data)
        :index(_index), filename(_filename), rowNumber(_rowNumber), data(_data){

    }

    QString toQString(){
        QString ret = QString("index: %1, filename: %2, rowNumber: %3, data: %4")
                .arg(QString::number(index), filename, QString::number(rowNumber), data);
        return ret;
    }

};

class SearchThread : public QThread{
    Q_OBJECT
public:
    QString keyword;
    QStandardItemModel* model;
    QStringList filter;
    SearchThread(const QString& _keyword, QStandardItemModel* _model, const QStringList& _filter)
        :keyword(_keyword), model(_model), filter(_filter){
    }

    void run()override{
        int index = 1;
        auto keyword = QRegularExpression(this->keyword);
        for(int i = 0; i < model->rowCount(); i++){
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
        qDebug()<<"finish";
//        renderItems();
    }

    void searchDirectory(const QString& filePath, int& index, const QRegularExpression& keyword){
        QDir dir(filePath);
        QFileInfoList fileInfos = dir.entryInfoList(filter, QDir::Filters(QDir::AllEntries | QDir::NoDotAndDotDot));
        for(auto fileInfo : fileInfos){
            if(fileInfo.isFile())
                searchFile(fileInfo.filePath(), index, keyword);
            else if(fileInfo.isDir())
                searchDirectory(fileInfo.filePath(), index, keyword);
        }
    }

    void searchFile(const QString& filePath, int& index, const QRegularExpression& keyword){
        qDebug()<<"search " <<filePath;
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
            while(!stream.atEnd()){
                QString line = stream.readLine();
                if(line.contains(keyword)){
                    auto item = SearchResultItem(index++, filePath, rowNumber, line);
//                    resultItems.append(new SearchResultItem(index++, filePath, rowNumber, line));
                    emit renderItemSignal(item);
                }
                rowNumber++;
            }
        }
        file.close();
    }

signals:
    void renderItemSignal(const SearchResultItem& item);
private:
};

class SearchTabWidget:public QWidget{
    Q_OBJECT
public:
    void clearResults(){
        for(auto item : resultItems){
            delete item;
        }
        resultItems.clear();
    }
    void renderItems(){
        renderToTextEdit();
    }

    void renderToTable(){

    }

    void renderToTextEdit(){
        auto keyword = keywordEdit->text();
        auto pattern = QRegularExpression(keyword);
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
        int pos = 0;
        QTextCursor cursor(displayEdit->document());
        auto oriCharFmt = cursor.charFormat();
        for(auto item : resultItems){
            QString text = templateText
                    .arg(QString::number(item->index), item->filename, QString::number(item->rowNumber), item->data);
            displayEdit->appendPlainText(text);
            int indexBegin = pos;
            int indexEnd = indexBegin + QString::number(item->index).size();
            int filenameLastIndex = item->filename.lastIndexOf(QRegularExpression("[\\/]"));
//            qDebug()<<filenameLastIndex;
            int filenameBegin = indexEnd+1 + filenameLastIndex + 1;
            int filenameEnd = filenameBegin + item->filename.size() - filenameLastIndex - 1;
            int rowNumberBegin = filenameEnd + 1;
            int rowNumberEnd = rowNumberBegin + QString::number(item->rowNumber).size();

            auto matches = pattern.globalMatch(item->data);
            QList<int>matchBegins({}), matchEnds({});
            while(matches.hasNext()){
                auto match = matches.next();
                auto matchBegin = match.capturedStart() + rowNumberEnd + 2;
                matchBegins.append(matchBegin);
                matchEnds.append(matchBegin + match.capturedLength());
            }
            int begins[] = {indexBegin, filenameBegin, rowNumberBegin};
            int ends[] = {indexEnd, filenameEnd, rowNumberEnd};
            for(int i = 0; i < 3; i++){
                highlight(begins[i], ends[i], colors[i], cursor);
            }
            for(int i = 0; i < matchEnds.size(); i++){
                highlight(matchBegins[i], matchEnds[i], colors[3], cursor);
            }
            pos += text.size() + 1;
        }
        if(!resultItems.isEmpty())
            cursor.setPosition(1, QTextCursor::MoveAnchor);
        cursor.setCharFormat(oriCharFmt);
    }
public slots:
    void renderItem(const SearchResultItem& item){
        qDebug()<<"render item";
        auto keyword = keywordEdit->text();
        auto pattern = QRegularExpression(keyword);
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

        auto matches = pattern.globalMatch(item.data);
        QList<int>matchBegins({}), matchEnds({});
        while(matches.hasNext()){
            auto match = matches.next();
            auto matchBegin = match.capturedStart() + rowNumberEnd + 2;
            matchBegins.append(matchBegin);
            matchEnds.append(matchBegin + match.capturedLength());
        }
        int begins[] = {indexBegin, filenameBegin, rowNumberBegin};
        int ends[] = {indexEnd, filenameEnd, rowNumberEnd};
        for(int i = 0; i < 3; i++){
            highlight(begins[i], ends[i], colors[i], cursor);
        }
        for(int i = 0; i < matchEnds.size(); i++){
            highlight(matchBegins[i], matchEnds[i], colors[3], cursor);
        }
    }
public slots:
    void search(){
        displayEdit->clear();
        SearchThread* thread = new SearchThread(keywordEdit->text(), model, filterEdit->text().split(';'));
        connect(thread, &SearchThread::renderItemSignal, this, &SearchTabWidget::renderItem);
        thread->start();
//        thread->start(QThread::Priority::TimeCriticalPriority);
//        searchThread();
    }

    void searchThread(){
        displayEdit->clear();
        clearResults();
        int index = 1;
        int renderPos = 0;
        auto keyword = QRegularExpression(keywordEdit->text());
        for(int i = 0; i < model->rowCount(); i++){
            auto item = model->item(i);
            if(item->checkState() != Qt::CheckState::Checked)continue;
            QFileInfo fileInfo(item->text());
            if(fileInfo.exists()){
                if(fileInfo.isFile())
                    searchFile(fileInfo.filePath(), index, keyword, renderPos);
                else if(fileInfo.isDir())
                    searchDirectory(fileInfo.filePath(), index, keyword, renderPos);
            }
        }
        qDebug()<<"finish";
        qDebug()<<"total: "<<resultItems.size();
//        renderItems();
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
    void searchDirectory(const QString& filePath, int& index, const QRegularExpression& keyword, int& renderPos){
        QDir dir(filePath);
        QStringList filter = filterEdit->text().split(';');
        QFileInfoList fileInfos = dir.entryInfoList(filter, QDir::Filters(QDir::AllEntries | QDir::NoDotAndDotDot));
        for(auto fileInfo : fileInfos){
            if(fileInfo.isFile())
                searchFile(fileInfo.filePath(), index, keyword, renderPos);
            else if(fileInfo.isDir())
                searchDirectory(fileInfo.filePath(), index, keyword, renderPos);
        }
    }

    void searchFile(const QString& filePath, int& index, const QRegularExpression& keyword, int& renderPos){
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
            while(!stream.atEnd()){
                QString line = stream.readLine();
                if(line.contains(keyword)){
                    auto item = SearchResultItem(index++, filePath, rowNumber, line);
//                    resultItems.append(new SearchResultItem(index++, filePath, rowNumber, line));
                    renderItem(item);
                }
                rowNumber++;
            }
        }
        file.close();
    }
public:
    QStandardItemModel* model = new QStandardItemModel();
    QList<SearchResultItem*>resultItems;
    TargetFileWidget* targetFileWidget = new TargetFileWidget(model);
    QLineEdit* keywordEdit = new QLineEdit("int");
    QLineEdit* filterEdit = new QLineEdit("*.cs;");
    QLabel* filterLabel = new QLabel("Filter");
    QLineEdit* tabNameEdit = new QLineEdit();
    QLabel* tabNameLabel = new QLabel("Name");
    QPushButton* searchBtn = new QPushButton("Search");
    QLabel* keywordLabel = new QLabel("Keyword");
    QVBoxLayout* displayLayout = new QVBoxLayout();
    QPlainTextEdit* displayEdit = new QPlainTextEdit();
    QTableView* displayTable = new QTableView();
    QGridLayout* tabInfoLayout = new QGridLayout();

public:
    ~SearchTabWidget(){
        this->clearResults();
    }

    SearchTabWidget(){
        this->setLayout(displayLayout);
        displayLayout->addLayout(tabInfoLayout);
        tabInfoLayout->addWidget(tabNameLabel, 0, 0);
        tabInfoLayout->addWidget(tabNameEdit, 0, 1, 1, 2);
        tabInfoLayout->addWidget(keywordLabel, 1, 0);
        tabInfoLayout->addWidget(keywordEdit, 1, 1);
        tabInfoLayout->addWidget(searchBtn, 1, 2);
        tabInfoLayout->addWidget(filterLabel, 2, 0);
        tabInfoLayout->addWidget(filterEdit, 2, 1, 1, 2);

        displayLayout->addWidget(displayEdit);

        displayLayout->addWidget(targetFileWidget);
//        displayLayout->addWidget(displayTable);
        displayEdit->setReadOnly(true);
        displayEdit->setWordWrapMode(QTextOption::NoWrap);
        displayEdit->setTabStopDistance(40);
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
