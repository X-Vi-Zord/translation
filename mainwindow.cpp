#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initial setup.
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());

    // Transfer information about supported languages.
    connect(&ya_translate, SIGNAL(languagesReceived(QMap<QString,QString>)),
            &ya_dictionary, SLOT(getLanguages(QMap<QString,QString>)));

    ya_translate.getSupportedLanguages();   // Get a list of supported languages.
    ya_dictionary.getTranslationDirections();   // Get supported translation directions for Dictionary API.

    // Preset values to display.
    for(const QString& lang : ya_translate.getLanguages().values())
    {
        ui->comboBoxSource->addItem(lang);
        ui->comboBoxTarget->addItem(lang);
    }

    ui->comboBoxSource->setCurrentText("English");
    ui->comboBoxTarget->setCurrentText("Russian");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_detectButton_clicked()
{

    ui->comboBoxSource->setCurrentText(ui->SourceEdit->toPlainText());
}

void MainWindow::on_dictionaryButton_clicked()
{
    QString CBS = ui->comboBoxSource->currentText();
    QString CBT = ui->comboBoxTarget->currentText();
    QString SE = ui->SourceEdit->toPlainText();

    QVector<DictionaryEntry> entries = ya_dictionary.getDictionaryEntry(CBS, CBT,SE);
    QVector<DictionaryEntry>::iterator iter;
    for(iter = entries.begin(); iter != entries.end(); ++iter)
    {
        iter->showEntry();
    }
}

void MainWindow::on_translateButton_clicked()
{
    QString CBS = ui->comboBoxSource->currentText();
    QString CBT = ui->comboBoxTarget->currentText();
    QString SE = ui->SourceEdit->toPlainText();
    QStringList translatedText = ya_translate.translate(CBS, CBT,SE);
    ui->TargetEdit->clear();
    for(const QString& text : translatedText)
    {
        ui->TargetEdit->append(text);
    }
}
