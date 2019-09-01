#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include"commonheader.h"
class ColorWidget:public QLineEdit{
    Q_OBJECT
public:
    ColorWidget(){
        this->setReadOnly(true);

    }

    QColor getColor(){
        return this->palette().color(QPalette::Base);
    }

    void setColor(const QColor color){
        auto palette = this->palette();
        palette.setColor(QPalette::Base, color);
        this->setPalette(palette);
        emit colorChanged(color);
    }

    virtual void mousePressEvent(QMouseEvent* e)override{
        QColor color = QColorDialog::getColor(this->getColor(), this);
        if(color.isValid()){
            this->setColor(color);
        }
    }
signals:
    void colorChanged(const QColor& color);
};

class FontWidget:public QLineEdit{
    Q_OBJECT
public:

    FontWidget(){
        this->setReadOnly(true);
    }

    QFont getFontValue(){
        return fontValue;
    }

    void setFontValue(const QFont& fontValue){
        this->fontValue = fontValue;
        QString text = "%1, %2";
        this->setText(text.arg(this->fontValue.family(), QString::number(this->fontValue.pointSize())));
        emit fontChanged(fontValue);
    }

    virtual void mousePressEvent(QMouseEvent* e)override{
        this->setFontValue(QFontDialog::getFont(nullptr, this->fontValue));
//        bool ok;
//        QFont font = QFontDialog::getFont(&ok);
//        if(ok){
//            this->font = font;
//        }
    }
signals:
    void fontChanged(const QFont& font);
private:
    QFont fontValue;
};

class SearchSettings{
public:
    QColor indexColor;
    QColor filenameColor;
    QColor rowNumberColor;
    QColor keywordColor;
    QColor backgroundColor;
    QColor textColor;
    QFont font;
    SearchSettings(){

    }
    SearchSettings(const QColor& _indexColor, const QColor& _filenameColor, const QColor& _rowNumberColor,
                   const QColor& _keywordColor, const QColor& _backgroundColor, const QColor& _textColor,
                   const QFont& _font):
        indexColor(_indexColor), filenameColor(_filenameColor), rowNumberColor(_rowNumberColor),
        keywordColor(_keywordColor), backgroundColor(_backgroundColor), textColor(_textColor),
        font(_font){

    }
};

class SearchSettingWidget : public QWidget{
    Q_OBJECT
public:
    SearchSettingWidget(){
        int row = 0;
        mainLayout->addWidget(indexColorLabel, row, 0);
        mainLayout->addWidget(filenameColorLabel, row++, 2);
        mainLayout->addWidget(rowNumberColorLabel, row, 0);
        mainLayout->addWidget(keywordColorLabel, row++, 2);
        mainLayout->addWidget(backgroundColorLabel, row, 0);
        mainLayout->addWidget(textColorLabel, row++, 2);
        mainLayout->addWidget(fontLabel, row++, 0);

        row = 0;
        mainLayout->addWidget(indexColorWidget, row, 1);
        mainLayout->addWidget(filenameColorWidget, row++, 3);
        mainLayout->addWidget(rowNumberColorWidget, row, 1);
        mainLayout->addWidget(keywordColorWidget, row++, 3);
        mainLayout->addWidget(backgroundColorWidget, row, 1);
        mainLayout->addWidget(textColorWidget, row++, 3);
        mainLayout->addWidget(fontWidget, row++, 1, 1, 3);

        mainLayout->addWidget(demoEdit, row, 0, 1, 4);

        this->setLayout(mainLayout);

        demoEdit->setReadOnly(true);
        demoEdit->setTabStopDistance(40);
        demoEdit->setWordWrapMode(QTextOption::NoWrap);
        demoEdit->setAlignment(Qt::AlignCenter);

        ColorWidget* colorWidgets[] = {indexColorWidget,filenameColorWidget,
                                      rowNumberColorWidget,keywordColorWidget,
                                      backgroundColorWidget, textColorWidget};
        for(auto colorWidget : colorWidgets){
            connect(colorWidget, &ColorWidget::colorChanged, this, &SearchSettingWidget::updateDemo);
        }

        connect(fontWidget, &FontWidget::fontChanged, this, &SearchSettingWidget::updateDemo);
        Searcher::searchSettings = &settings;
    }
#define INDEX_COLOR_KEY "index_color"
#define FILENAME_COLOR_KEY "filename_color"
#define ROW_NUMBER_COLOR_KEY "row_number_color"
#define KEYWORD_COLOR_KEY "keyword_color"
#define BACKGROUND_COLOR_KEY "background_color"
#define TEXT_COLOR_KEY "text_color"
#define FONT_KEY "font"
    void writeDefualtSettings(QSettings& settings){
        QColor indexColor = QColor("orange");
        QColor filenameColor = Qt::green;
        QColor rowNumberColor = QColor("cyan");
        QColor keywordColor = Qt::yellow;
        QColor backgroundColor = Qt::black;
        QColor textColor = Qt::white;
        QFont font = QFont("华文新魏",12);
        settings.setValue(INDEX_COLOR_KEY, indexColor);
        settings.setValue(FILENAME_COLOR_KEY, filenameColor);
        settings.setValue(ROW_NUMBER_COLOR_KEY, rowNumberColor);
        settings.setValue(KEYWORD_COLOR_KEY, keywordColor);
        settings.setValue(BACKGROUND_COLOR_KEY, backgroundColor);
        settings.setValue(TEXT_COLOR_KEY, textColor);
        settings.setValue(FONT_KEY, font);
        qDebug()<<font;
    }

    void readSettings(const QSettings& settings){
        indexColorWidget->setColor(settings.value(INDEX_COLOR_KEY).value<QColor>());
        filenameColorWidget->setColor(settings.value(FILENAME_COLOR_KEY).value<QColor>());
        rowNumberColorWidget->setColor(settings.value(ROW_NUMBER_COLOR_KEY).value<QColor>());
        keywordColorWidget->setColor(settings.value(KEYWORD_COLOR_KEY).value<QColor>());
        backgroundColorWidget->setColor(settings.value(BACKGROUND_COLOR_KEY).value<QColor>());
        textColorWidget->setColor(settings.value(TEXT_COLOR_KEY).value<QColor>());
        fontWidget->setFontValue(settings.value(FONT_KEY).value<QFont>());
        this->settings = SearchSettings(indexColorWidget->getColor(), filenameColorWidget->getColor(),
                                  rowNumberColorWidget->getColor(), keywordColorWidget->getColor(),
                                  backgroundColorWidget->getColor(), textColorWidget->getColor(),
                                  fontWidget->getFontValue());
        updateDemo();
    }

    void writeSettings(){
        QSettings settings(Searcher::COMPANY_NAME, Searcher::APPLICATION_NAME);
        settings.setValue(INDEX_COLOR_KEY, indexColorWidget->getColor());
        settings.setValue(FILENAME_COLOR_KEY, filenameColorWidget->getColor());
        settings.setValue(ROW_NUMBER_COLOR_KEY, rowNumberColorWidget->getColor());
        settings.setValue(KEYWORD_COLOR_KEY, keywordColorWidget->getColor());
        settings.setValue(BACKGROUND_COLOR_KEY, backgroundColorWidget->getColor());
        settings.setValue(TEXT_COLOR_KEY, textColorWidget->getColor());
    }
    SearchSettings getSettings(){
        return SearchSettings(indexColorWidget->getColor(), filenameColorWidget->getColor(),
                              rowNumberColorWidget->getColor(), keywordColorWidget->getColor(),
                              backgroundColorWidget->getColor(), textColorWidget->getColor(),
                              fontWidget->getFontValue());
    }
public slots:
    void updateDemo(){
        settings = SearchSettings(indexColorWidget->getColor(), filenameColorWidget->getColor(),
                                  rowNumberColorWidget->getColor(), keywordColorWidget->getColor(),
                                  backgroundColorWidget->getColor(), textColorWidget->getColor(),
                                  fontWidget->getFontValue());
        int demoIndex = 1;
        QString demoFilename = "C:/Document/demo.cpp";
        int demoRowNumber = 99;
        QString demoData = "int main(int argc, char** argv)";
        QRegularExpression pattern("int");
        QTextCursor cursor(demoEdit->document());
        auto highlight = [&](int begin, int end, const QColor& color, QTextCursor& cursor){
            cursor.setPosition(begin, QTextCursor::MoveAnchor);
            auto fmt = cursor.charFormat();
            cursor.setPosition(end, QTextCursor::KeepAnchor);
            fmt.setForeground(QBrush(color));
            cursor.setCharFormat(fmt);
        };
        QString templateText = "%1\t%2\t%3:\t%4";
        QString text = templateText
                .arg(QString::number(demoIndex), demoFilename, QString::number(demoRowNumber), demoData);
        demoEdit->setText(text);
        int indexBegin = 0;
        int indexEnd = indexBegin + QString::number(demoIndex).size();
        int filenameLastIndex = demoFilename.lastIndexOf(QRegularExpression("[\\/]"));
//            qDebug()<<filenameLastIndex;
        int filenameBegin = indexEnd+1 + filenameLastIndex + 1;
        int filenameEnd = filenameBegin + demoFilename.size() - filenameLastIndex - 1;
        int rowNumberBegin = filenameEnd + 1;
        int rowNumberEnd = rowNumberBegin + QString::number(demoRowNumber).size();

        auto matches = pattern.globalMatch(demoData);
        QList<int>matchBegins({}), matchEnds({});
        while(matches.hasNext()){
            auto match = matches.next();
            auto matchBegin = match.capturedStart() + rowNumberEnd + 2;
            matchBegins.append(matchBegin);
            matchEnds.append(matchBegin + match.capturedLength());
        }

        int begins[] = {indexBegin, filenameBegin, rowNumberBegin};
        int ends[] = {indexEnd, filenameEnd, rowNumberEnd};
        QColor colors[] = {indexColorWidget->getColor(), filenameColorWidget->getColor(), rowNumberColorWidget->getColor()};
        for(int i = 0; i < 3; i++){
            highlight(begins[i], ends[i], colors[i], cursor);
        }
        for(int i = 0; i < matchEnds.size(); i++){
            highlight(matchBegins[i], matchEnds[i], keywordColorWidget->getColor(), cursor);
        }
        QPalette palette = demoEdit->palette();
        palette.setColor(QPalette::Base, backgroundColorWidget->getColor());
        palette.setColor(QPalette::Text, textColorWidget->getColor());
        demoEdit->setPalette(palette);
        demoEdit->setFont(fontWidget->getFontValue());
    }
private:
    QGridLayout* mainLayout = new QGridLayout;
    QLabel* indexColorLabel = new QLabel("Index Color");
    QLabel* filenameColorLabel = new QLabel("Filename Color");
    QLabel* rowNumberColorLabel = new QLabel("Row Number Color");
    QLabel* keywordColorLabel = new QLabel("Keyword Color");
    QLabel* backgroundColorLabel = new QLabel("Background Color");
    QLabel* textColorLabel = new QLabel("Text Color");
    QLabel* fontLabel = new QLabel("Font");

    ColorWidget* indexColorWidget = new ColorWidget();
    ColorWidget* filenameColorWidget = new ColorWidget();
    ColorWidget* rowNumberColorWidget = new ColorWidget();
    ColorWidget* keywordColorWidget = new ColorWidget();
    ColorWidget* backgroundColorWidget = new ColorWidget();
    ColorWidget* textColorWidget = new ColorWidget();
    FontWidget* fontWidget = new FontWidget();

    QTextEdit* demoEdit = new QTextEdit();
    SearchSettings settings;
};

class SettingDialog : public QDialog{
    Q_OBJECT
public:
    SettingDialog(){
        QVBoxLayout* mainLayout = new QVBoxLayout();
        mainLayout->addWidget(tabWidget);
        mainLayout->addWidget(buttonBox);
        tabWidget->addTab(searchSettingWidget, "Search Setting");
        this->setLayout(mainLayout);
        this->setModal(true);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(this, &QDialog::accepted, [&]{
            searchSettingWidget->writeSettings();
            emit settingsChanged();
        });
    }
    void showEvent(QShowEvent* e) override{
        loadCurrentSetting();
    }

    void loadCurrentSetting(){
        qDebug()<<"load setting";
    }

    void writeDefualtSettings(QSettings& settings){
        searchSettingWidget->writeDefualtSettings(settings);
    }

    void readSettings(const QSettings& settings){
        searchSettingWidget->readSettings(settings);
    }
signals:
    void settingsChanged();
private:
    QTabWidget* tabWidget = new QTabWidget();
    SearchSettingWidget* searchSettingWidget = new SearchSettingWidget();
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
};


#endif // SETTINGDIALOG_H
