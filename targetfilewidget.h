#ifndef TARGETFILEWIDGET_H
#define TARGETFILEWIDGET_H
#include"commonheader.h"
class TargetFileWidget:public QWidget{
    // tagetfiles
    QVBoxLayout* targetFilesLayout = new QVBoxLayout();
    QHBoxLayout* choosePathLayout = new QHBoxLayout();
    QPushButton* chooseFileBtn = new QPushButton("Add Files");
    QPushButton* chooseFolderBtn = new QPushButton("Add Folder");
    QPushButton* changeFileBtn = new QPushButton("Change File");
    QPushButton* changeFolderBtn = new QPushButton("Change Folder");
    QPushButton* removePathBtn = new QPushButton("Remove");
    QListView* targetFilesListView = new QListView();
    QFileDialog* fileDialog = new QFileDialog();
    QFileDialog* folderDialog = new QFileDialog();
public:
    QStandardItemModel* targetFilesModel;
    TargetFileWidget(QStandardItemModel* targetFilesModel){
        this->targetFilesModel = targetFilesModel;
        setLayout(targetFilesLayout);
        targetFilesLayout->addLayout(choosePathLayout);
        targetFilesLayout->addWidget(targetFilesListView);
        auto btns = {chooseFileBtn, chooseFolderBtn, changeFileBtn, changeFolderBtn, removePathBtn};
        for(auto btn: btns){
            choosePathLayout->addWidget(btn);
        }


        fileDialog->setFileMode(QFileDialog::AnyFile);
        folderDialog->setFileMode(QFileDialog::Directory);

        targetFilesListView->setModel(targetFilesModel);
        targetFilesListView->setSelectionMode( QAbstractItemView::ExtendedSelection );

        connect(chooseFileBtn, &QPushButton::clicked, [=]{
            QStringList choseFilenames = fileDialog->getOpenFileNames();
            for(auto filename : choseFilenames){
                auto item = createItem(filename);
                targetFilesModel->appendRow(item);
            }
        });

        connect(chooseFolderBtn, &QPushButton::clicked, [=]{
            QString choseDir = fileDialog->getExistingDirectory();
            if(choseDir.isEmpty())return;
            targetFilesModel->appendRow(createItem(choseDir));
        });

        connect(changeFileBtn, &QPushButton::clicked, [=]{
            if(getSelectedItemCount(*targetFilesListView) != 1)return;
            QString choseFilename = fileDialog->getOpenFileName();
            targetFilesModel->setItem(targetFilesListView->selectionModel()->currentIndex().row(), this->createItem(choseFilename));
        });

        connect(changeFolderBtn, &QPushButton::clicked, [=]{
            if(getSelectedItemCount(*targetFilesListView) != 1)return;
            QString choseDir = fileDialog->getExistingDirectory();
            if(choseDir.isEmpty())return;
            targetFilesModel->setItem(targetFilesListView->selectionModel()->currentIndex().row(), this->createItem(choseDir));
        });

        connect(removePathBtn, &QPushButton::clicked, [=]{
            if(!isHasSelectedItem(*targetFilesListView)){
                return;
            }
            removeAllSelectedItem(*targetFilesListView);
        });

        this->setupDebug();
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

    void setupDebug(){
        targetFilesModel->appendRow(createItem("C:/Users/35974/Documents/QtProjects/Searcher/mainwindow.h"));

        // debug log
//        QRegularExpression pattern("abcd");
//        QRegularExpressionMatchIterator iterator = pattern.globalMatch("abcd  abcd");
//        while(iterator.hasNext()){
//            qDebug()<<iterator.next();
//        }
    }
};

#endif // TARGETFILEWIDGET_H
