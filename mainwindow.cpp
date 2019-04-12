#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "confedit.h"
#include "importconf.h"
#include <vinteract.h>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateConfTable();
    ui->configTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->configTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showMenu(QPoint)));
    this->v2Inst = new v2Instance();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->v2Inst;
}

void MainWindow::on_actionEdit_triggered()
{
    ConfEdit *e = new ConfEdit(this);
    e->show();
}

void MainWindow::on_actionExisting_config_triggered()
{
    importConf *f = new importConf(this);
    f->show();
}
void MainWindow::showMenu(QPoint pos)
{
    QMenu *popMenu = new QMenu(ui->configTable);
    QAction *select = new QAction("Select", ui->configTable);
    QAction *del = new QAction("Delete", ui->configTable);
    popMenu->addAction(select);
    popMenu->addAction(del);
    popMenu->move(cursor().pos());
    popMenu->show();
    connect(select, SIGNAL(triggered()), this, SLOT(geneConf()));
    connect(del, SIGNAL(triggered()), this, SLOT(delConf()));
}
void MainWindow::geneConf()
{
    int row = ui->configTable->selectionModel()->currentIndex().row();
    vConfig tmpConf;
    int idIntable = ui->configTable->model()->data(ui->configTable->model()->index(row, 4)).toInt();
    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(confDatabase);
    }
    if (!database.open()) {
        qDebug() << "Failed to open database while querying.";
    } else {
        QSqlQuery myQuery(database);
        myQuery.exec("update confs set selected = 0");
        QString queryString = "update confs set selected = 1 where id = " + QString::number(idIntable);
        myQuery.exec(queryString);
    }
    emit updateConfTable();
    tmpConf.query(idIntable);
    if (tmpConf.isCustom == 1) {
        QString src = "conf/" + QString::number(idIntable) + ".conf";
        if (QFile::exists("config.json")) {
            QFile::remove("config.json");
        }
        QFile::copy(src, "config.json");
    } else {//Config generator
    }
}
void MainWindow::delConf()
{
    int row = ui->configTable->selectionModel()->currentIndex().row();
    int idIntable = ui->configTable->model()->data(ui->configTable->model()->index(row, 4)).toInt();
    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(confDatabase);
    }
    if (!database.open()) {
        qDebug() << "Failed to open database while deleting row.";
    } else {
        QSqlQuery myQuery(database);
        QString queryString = "delete from confs where id = " + QString::number(idIntable);
        myQuery.exec(queryString);
    }
    QString rmFile = "conf/" + QString::number(idIntable) + ".conf";
    QFile::remove(rmFile);
    emit updateConfTable();
}
void MainWindow::updateConfTable()
{
    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(confDatabase);
    }
    if (!database.open()) {
        qDebug() << "Failed to open database while updating table.";
    } else {
        QSqlQuery myQuery(database);
        myQuery.exec("select COUNT(*) from confs;");
        myQuery.first();
        int rows = myQuery.value(0).toInt();
        QStandardItemModel* model = new QStandardItemModel(rows, 5);
        ui->configTable->setModel(model);
        model->setHeaderData(0, Qt::Horizontal, "Alias");
        model->setHeaderData(1, Qt::Horizontal, "Host");
        model->setHeaderData(2, Qt::Horizontal, "Port");
        model->setHeaderData(3, Qt::Horizontal, "Checked");
        model->setHeaderData(4, Qt::Horizontal, "idInTable");
        ui->configTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->configTable->setColumnHidden(4, true);
        ui->configTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->configTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        myQuery.exec("select * from confs");
        myQuery.first();
        for (int i = 0; i < rows; ++i) {
            model->setItem(i, 0, new QStandardItem(myQuery.value(3).toString()));
            model->setItem(i, 1, new QStandardItem(myQuery.value(1).toString()));
            model->setItem(i, 2, new QStandardItem(myQuery.value(2).toString()));
            model->setItem(i, 4, new QStandardItem(myQuery.value(0).toString()));
            if (myQuery.value(8).toInt() == 1) {
                model->setItem(i, 3, new QStandardItem("√"));
            }
            myQuery.next();
        }
    }
}

void MainWindow::updateLog()
{
    ui->logText->insertPlainText(this->v2Inst->v2Process->readAllStandardOutput());
}

void MainWindow::on_startButton_clicked()
{
    this->v2Inst->start(this);
}

void MainWindow::on_stopButton_clicked()
{
    this->v2Inst->stop();
    ui->logText->clear();
}

void MainWindow::on_restartButton_clicked()
{
    this->v2Inst->stop();
    ui->logText->clear();
    this->v2Inst->start(this);
}

void MainWindow::on_clbutton_clicked()
{
    ui->logText->clear();
}

void MainWindow::on_rtButton_clicked()
{
    emit updateConfTable();
}
