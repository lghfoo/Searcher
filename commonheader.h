#ifndef COMMONHEADER_H
#define COMMONHEADER_H
#include<QMainWindow>
#include<QLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QGridLayout>
#include<QLineEdit>
#include<QPushButton>
#include<QFile>
#include<QFileDialog>
#include<QListView>
#include<QDockWidget>
#include<QStandardItemModel>
#include<QStyledItemDelegate>
#include<QCheckBox>
#include<QObject>
#include<QTextEdit>
#include<QToolButton>
#include<QLabel>
#include<QDebug>
#include<QTextCodec>
#include<QPlainTextEdit>
#include<QTableView>
#include<QSpinBox>
#include<QRegularExpression>
#include<QRegularExpressionMatch>
#include<QDialogButtonBox>
#include<QMenuBar>
#include<QMenu>
#include<QShowEvent>
#include<QColorDialog>
#include<QSettings>
#include<QStackedLayout>
#include<QFontDialog>
#include<QGroupBox>
#include<QThread>
#include<QSplitter>
#include<QFontMetrics>
class SearchSettings;

namespace Searcher {
    const static QString COMPANY_NAME = "LGHFOO";
    const static QString APPLICATION_NAME = "Searcher";
    static SearchSettings* searchSettings;
    static void Init(){

    }
}
#endif // COMMONHEADER_H
