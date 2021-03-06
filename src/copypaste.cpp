/**
 * @file copypaste.cpp
 *
 * MultiCopyPaste - a free and open-source tool to copy/paste and organize your data
 * Copyright (C) 2021 Harsh Mittal <harshmittal2210@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "copypaste.h"
#include "ui_copypaste.h"


CopyPaste::CopyPaste(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CopyPaste)
{
    ui->setupUi(this);
    this->setWindowTitle("Multi Copy Paste");
    this->setWindowIcon(QIcon(":/img/img/paste.png"));

    addNewTab("Default");

}

CopyPaste::~CopyPaste()
{
    delete ui;
}

void CopyPaste::addNewTab(QString tabName)
{
    TabNew *newTabptr=new TabNew(this);
    ui->tabWidget->addTab(newTabptr, tabName);
    newTabptr->setAttribute(Qt::WA_DeleteOnClose,true);
    allTabPtr.append(newTabptr);
}

void CopyPaste::openJSONFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)){
        qWarning("Couldn't open save file.");
        QMessageBox::warning(this,"Load JSON File","Not able to open the JSON File");

        return;
    }
    QJsonParseError JsonParseError;
    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData, &JsonParseError));
    if(JsonParseError.error != QJsonParseError::NoError){
            qDebug()<< "Parse Error"<< JsonParseError.errorString();
            QMessageBox::warning(this,"Load JSON File","Please check the JSON file, there are some issues in it");
            return;
    }
    readAllData(loadDoc.object());
}


void CopyPaste::on_tabWidget_tabCloseRequested(int index)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Confirmation", "Delete This Tab?",QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes){
        qDebug()<<"Removing tab of index:"<<index;
        allTabPtr[index]->close();
        ui->tabWidget->removeTab(index);

        allTabPtr.remove(index);
    }

    if(allTabPtr.length()==0){
        addNewTab("Default");
        ui->statusbar->showMessage("Added Default Tab",3000);
    }

}


void CopyPaste::on_newTabButton_clicked()
{

    addNewTab(QString("Tab %0").arg(ui->tabWidget->count()+1));
}

void CopyPaste::on_tabWidget_tabBarDoubleClicked(int index)
{
//    ui->tabWidget->setTabText(index,"work");
    currDoubleClickedTab=index;
    newWindow = new NewTabDialog(this);
    connect(newWindow,SIGNAL(newTabName(QString)),this,SLOT(recievedNewTabName(QString)));
    newWindow->show();
}

void CopyPaste::recievedNewTabName(QString newName)
{
    ui->tabWidget->setTabText(currDoubleClickedTab,newName);
}

bool CopyPaste::writeAllData(QJsonObject &json){

    json["Author Name"] = "Harsh Mittal";
    json["Total Tabs"] = allTabPtr.length();

    QJsonArray tabsJSON;
    for (int i=0;i<allTabPtr.length() ;i++ ) {
        QJsonObject tabDataJSON;

        // Write data for each tab cell
        QJsonArray allCelldData;
        for (int j=0;j<allTabPtr[i]->allCellPtr.length() ;j++ ) {
            QJsonObject cellData;
            cellData["Text Data"] = allTabPtr[i]->allCellPtr[j]->getTextData();
            cellData["Cell Name"] = allTabPtr[i]->allCellPtr[j]->getCellName();
            allCelldData.append(cellData);
        }
        tabDataJSON["Tab Name"] = ui->tabWidget->tabText(i);
        tabDataJSON["Total Cells"] = allTabPtr[i]->allCellPtr.length();
        tabDataJSON["Tab Data"] = allCelldData;
        tabsJSON.append(tabDataJSON);
    }
    json["Data"] = tabsJSON;
    return true;
}
bool CopyPaste::readAllData(const QJsonObject &json)
{
    int numTabs;
    if(json.contains("Total Tabs")){
        numTabs = json["Total Tabs"].toInt();
    }

    if(json.contains("Data") && json["Data"].isArray()){
        QJsonArray tabsJSON = json["Data"].toArray();

        for(int tabIndex=0;tabIndex<tabsJSON.size();tabIndex++){
            QJsonObject tabDataJSON = tabsJSON[tabIndex].toObject();

            // Create New Tab
            TabNew *newTabptr=new TabNew(this);

            ui->tabWidget->addTab(newTabptr, tabDataJSON["Tab Name"].toString());
            newTabptr->setAttribute(Qt::WA_DeleteOnClose,true);
            allTabPtr.append(newTabptr);

            QJsonArray allCelldData = tabDataJSON["Tab Data"].toArray();

            for (int cellIndex=0;cellIndex<allCelldData.size() ;cellIndex++ ) {
                QJsonObject cellData = allCelldData[cellIndex].toObject();
                QString cellText = cellData["Text Data"].toString();
                QString cellName = cellData["Cell Name"].toString();
                newTabptr->addNewCell(cellText,cellName);
//                qDebug()<<cellData;
            }


        }
    }

    return true;
}

void CopyPaste::on_actionSave_triggered()
{
    QString filter = "JSON File (*.json)";
    QString fileName = QFileDialog::getSaveFileName(this,"Save JSON File",QDir::currentPath(),filter);
//    qDebug()<<fileName;
    if(fileName.isEmpty()){
        return;
    }
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly)){
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject copyPasteObject;
    writeAllData(copyPasteObject);

    file.write(QJsonDocument(copyPasteObject).toJson());
    file.close();

    qDebug()<<"Saved File";

}

void CopyPaste::on_actionExit_triggered()
{
    for (int i=0;i<allTabPtr.length() ;i++ ) {
        allTabPtr[i]->close();
    }
    QApplication::exit();
}

void CopyPaste::on_actionAbout_triggered()
{
    AboutMeDialog aboutMe;
    aboutMe.show();
    aboutMe.exec();
}

void CopyPaste::on_actionLoad_triggered()
{
    QString filter = "JSON File (*.json)";
    QString fileName = QFileDialog::getOpenFileName(this,"Open JSON File",QDir::currentPath(),filter);
//    qDebug()<<fileName;
    if(fileName.isEmpty()){
        return;
    }
    openJSONFile(fileName);

}

void CopyPaste::on_actionFAQ_triggered()
{
    QDesktopServices::openUrl(QUrl("https://harshmittal2210.github.io/blog/Qt/multiCopyPaste", QUrl::TolerantMode));
}

void CopyPaste::on_actionC_triggered()
{
    openJSONFile(":/json/examples/coding/c.json");
}

void CopyPaste::on_actionC_2_triggered()
{
    openJSONFile(":/json/examples/coding/cpp.json");
}

void CopyPaste::on_actionPython_triggered()
{
    openJSONFile(":/json/examples/coding/python.json");
}

void CopyPaste::on_actionJava_triggered()
{
    openJSONFile(":/json/examples/coding/java.json");
}

void CopyPaste::on_actionHTML_triggered()
{
    openJSONFile(":/json/examples/coding/html.json");
}

void CopyPaste::on_actionPersonel_Info_triggered()
{
    openJSONFile(":/json/examples/info.json");
}

void CopyPaste::on_actionDocuments_triggered()
{
    openJSONFile(":/json/examples/doc.json");
}
