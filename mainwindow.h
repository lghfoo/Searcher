#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
class TabWidget:public QWidget{
    Q_OBJECT
public:
    TabWidget(){
        this->setLayout(displayLayout);
        displayLayout->addLayout(searchLayout);
        displayLayout->addWidget(displayEdit);
        searchLayout->addWidget(keywordEdit);
        searchLayout->addWidget(searchBtn);

        displayEdit->setReadOnly(true);

    }
private:
    QLineEdit* keywordEdit = new QLineEdit();
    QPushButton* searchBtn = new QPushButton("Search");
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QVBoxLayout* displayLayout = new QVBoxLayout();
    QTextEdit* displayEdit = new QTextEdit();
};

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    QStandardItemModel* targetFilesModel = new QStandardItemModel();
    MainWindow(){
        this->setupMember();
        this->setupUI();
        this->setupEvent();
    }
private:

    // tagetfiles
    QVBoxLayout* targetFilesLayout = new QVBoxLayout();
    QHBoxLayout* choosePathLayout = new QHBoxLayout();
    QPushButton* chooseFileBtn = new QPushButton("Add Files");
    QPushButton* chooseFolderBtn = new QPushButton("Add Folder");
    QPushButton* changeFileBtn = new QPushButton("Change File");
    QPushButton* changeFolderBtn = new QPushButton("Change Folder");
    QPushButton* removePathBtn = new QPushButton("Remove");
    QListView* targetFilesListView = new QListView();
    QDockWidget* targetFilesDock = new QDockWidget();
    QWidget* targetFilesWidget = new QWidget();

    // search & display
    QTabWidget* displayTabs = new QTabWidget();
    QToolButton* addTabButton = new QToolButton();


    QFileDialog* fileDialog = new QFileDialog();
    QFileDialog* folderDialog = new QFileDialog();
    void setupUI(){
        this->setCentralWidget(displayTabs);
        displayTabs->addTab(new QWidget(), "");
        displayTabs->addTab(new TabWidget(), "Search 1");
        displayTabs->tabBar()->tabButton(1, QTabBar::RightSide)->hide();

        // targetfiles
        this->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, targetFilesDock);
        targetFilesDock->setWidget(targetFilesWidget);
        targetFilesWidget->setLayout(targetFilesLayout);
        targetFilesLayout->addLayout(choosePathLayout);
        targetFilesLayout->addWidget(targetFilesListView);
        auto btns = {chooseFileBtn, chooseFolderBtn, changeFileBtn, changeFolderBtn, removePathBtn};
        for(auto btn: btns){
            choosePathLayout->addWidget(btn);
        }

        // display & search
        displayTabs->setTabEnabled(0, false);
        displayTabs->tabBar()->setTabButton(0, QTabBar::RightSide, addTabButton);
    }
    void setupEvent(){
        connect(chooseFileBtn, &QPushButton::clicked, [&]{
            QStringList choseFilenames = fileDialog->getOpenFileNames();
            for(auto filename : choseFilenames){
                targetFilesModel->appendRow(createItem(filename));
            }
        });

        connect(chooseFolderBtn, &QPushButton::clicked, [&]{
            QString choseDir = fileDialog->getExistingDirectory();
            if(choseDir.isEmpty())return;
            targetFilesModel->appendRow(createItem(choseDir));
        });

        connect(changeFileBtn, &QPushButton::clicked, [&]{
            if(getSelectedItemCount(*targetFilesListView) != 1)return;
            QString choseFilename = fileDialog->getOpenFileName();
            targetFilesModel->setItem(targetFilesListView->selectionModel()->currentIndex().row(), this->createItem(choseFilename));
        });

        connect(changeFolderBtn, &QPushButton::clicked, [&]{
            if(getSelectedItemCount(*targetFilesListView) != 1)return;
            QString choseDir = fileDialog->getExistingDirectory();
            if(choseDir.isEmpty())return;
            targetFilesModel->setItem(targetFilesListView->selectionModel()->currentIndex().row(), this->createItem(choseDir));
        });

        connect(removePathBtn, &QPushButton::clicked, [&]{
            if(!isHasSelectedItem(*targetFilesListView)){
                return;
            }
            removeAllSelectedItem(*targetFilesListView);
        });

        connect(addTabButton, &QToolButton::clicked, [&]{
            auto tab =  new TabWidget();
            displayTabs->addTab(tab, QString("Search ") + QString::number(displayTabs->count()));
        });

        connect(displayTabs, &QTabWidget::tabCloseRequested, [&](int index){
            displayTabs->removeTab(index);
        });
    }
    void setupMember(){
        fileDialog->setFileMode(QFileDialog::AnyFile);
        folderDialog->setFileMode(QFileDialog::Directory);

        targetFilesListView->setModel(targetFilesModel);
        targetFilesListView->setSelectionMode( QAbstractItemView::ExtendedSelection );

        addTabButton->setText("+");

        displayTabs->setTabsClosable(true);
    }
    QStandardItem* createItem(QString path){
        QStandardItem* item = new QStandardItem(path);
        item->setCheckable(true);
        item->setCheckState(Qt::CheckState::Checked);
        return item;
    }

    bool isHasSelectedItem(const QListView& view){
        return !view.selectionModel()->selectedIndexes().isEmpty();
    }



    int getSelectedItemCount(const QListView& view){
        return view.selectionModel()->selectedIndexes().size();
    }

    void removeAllSelectedItem(const QListView& view){
        QModelIndexList list = view.selectionModel()->selectedIndexes();
        std::sort(list.begin(), list.end(), qGreater<QModelIndex>());
        for(auto index : list) {
            view.model()->removeRow(index.row());
        }
    }


private:
    QString searchPath;
};

#endif // MAINWINDOW_H
