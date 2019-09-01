#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include"commonheader.h"
#include"searchtabwidget.h"
#include"settingdialog.h"

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:

    MainWindow(){
        this->readSettings();
        this->setupMember();
        this->setupUI();
        this->setupEvent();

    }
private:
    SettingDialog* settingDialog = new SettingDialog();

    // search & display
    QTabWidget* displayTabs = new QTabWidget();
    QToolButton* addTabButton = new QToolButton();
    void setupUI(){
        // toolbar
        auto menubar = this->menuBar();
        auto editMenu = new QMenu("Edit");
        auto settingAction = new QAction("Settings");
        menubar->addMenu(editMenu);
        editMenu->addAction(settingAction);
        connect(settingAction, &QAction::triggered, [&]{
            settingDialog->show();
        });


        // display & search
        this->setCentralWidget(displayTabs);
        displayTabs->addTab(new QWidget(), "");
        displayTabs->addTab(createSearchTabWidget(), "Search 1");
        displayTabs->tabBar()->tabButton(1, QTabBar::RightSide)->hide();
        displayTabs->setTabEnabled(0, false);
        displayTabs->tabBar()->setTabButton(0, QTabBar::RightSide, addTabButton);


    }
    void setupEvent(){


        connect(addTabButton, &QToolButton::clicked, [&]{
            displayTabs->addTab(createSearchTabWidget(), QString("Search ") + QString::number(displayTabs->count()));
        });

        connect(displayTabs, &QTabWidget::tabCloseRequested, [&](int index){
            auto tabWidget = displayTabs->widget(index);
            displayTabs->removeTab(index);
            delete tabWidget;
        });

    }

    SearchTabWidget* createSearchTabWidget(){
        SearchTabWidget* ret = new SearchTabWidget();
        connect(ret, &SearchTabWidget::nameChanged, [=](const QString& name){
            this->displayTabs->setTabText(displayTabs->indexOf(ret), name);
        });
        connect(settingDialog, &SettingDialog::settingsChanged, ret, &SearchTabWidget::onSettingChanged);
        return ret;
    }

    void setupMember(){


        addTabButton->setText("+");

        displayTabs->setTabsClosable(true);
    }
    void readSettings(){
        QSettings settings(Searcher::COMPANY_NAME, Searcher::APPLICATION_NAME);
        QVariant isExist = settings.value("exist");
        if(!isExist.isValid()){
            settings.setValue("exist", "true");
            this->writeDefaultSettings(settings);
        }
        this->settingDialog->readSettings(settings);
    }

    void writeDefaultSettings(QSettings& settings){
        this->settingDialog->writeDefualtSettings(settings);
    }





private:
    QString searchPath;
};

#endif // MAINWINDOW_H
