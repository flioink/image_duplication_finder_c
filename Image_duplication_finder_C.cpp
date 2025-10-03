
#include "Image_duplication_finder_C.h" // current name of the project
#include <qcoreapplication.h>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QProgressBar>
#include <QGroupBox>
#include <QRadiobutton>
#include <QThread>
#include "scan_worker.h"


MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
, current_search_method(search_method::exact_match)
{
    build_UI();
    setFixedSize(500, 350);
}

void MainWindow::build_UI()
{
    int browse_buttons_width = 90;
    int text_boxes_width = 300;
    //Source setup
    source_label = new QLabel("Source Path", this);
    source_edit = new QLineEdit(this);
    source_edit->setFixedWidth(text_boxes_width);
    browse_source_button = new QPushButton("Source Folder", this);
    browse_source_button->setFixedWidth(browse_buttons_width); //lock button width

    //Destination setup
    destination_label = new QLabel("Target Path", this);
    dest_edit = new QLineEdit(this);
    dest_edit->setFixedWidth(text_boxes_width);
    browse_destination_button = new QPushButton("Target Folder", this);
    browse_destination_button->setFixedWidth(browse_buttons_width);

    //Scanning button setup
    scan_button = new QPushButton("Scan", this);
    scan_button->setObjectName("scanButton");
    scan_button->setProperty("class", "scan-button");
    //scan_button->setFixedWidth(text_boxes_width + 30);
    scan_button->setFixedHeight(50);
    scan_button->setEnabled(false);

    //Window master layout
    QVBoxLayout* master_layout = new QVBoxLayout(this);

    QHBoxLayout* source_layout = new QHBoxLayout();
    source_layout->addWidget(source_label);
    source_layout->addWidget(source_edit);
    source_layout->addWidget(browse_source_button);

    QHBoxLayout* destination_layout = new QHBoxLayout();
    destination_layout->addWidget(destination_label);
    destination_layout->addWidget(dest_edit);
    destination_layout->addWidget(browse_destination_button);


    //Scan button layout
    QHBoxLayout* scan_button_layout = new QHBoxLayout();
    scan_button_layout->addWidget(scan_button);

    // Scan method group layout
    QHBoxLayout* scan_method_layout = new QHBoxLayout();
    exact_match_radio = new QRadioButton("Exact Match");
    perceptual_hash_radio = new QRadioButton("Perceptual Hash");
    mean_color_radio = new QRadioButton("Mean Color");
    scan_method_layout->addWidget(exact_match_radio);
    scan_method_layout->addWidget(perceptual_hash_radio);
    scan_method_layout->addWidget(mean_color_radio);
    exact_match_radio->setChecked(true);
    QGroupBox* scan_method_group = new QGroupBox("Search Method");
    scan_method_group->setLayout(scan_method_layout);

    // move method group layout
    QHBoxLayout* move_method_layout = new QHBoxLayout();

    move_all_radio = new QRadioButton("Move All");
    move_all_except_one_radio = new QRadioButton("Move All But One");
    move_method_layout->addWidget(move_all_radio);
    move_method_layout->addWidget(move_all_except_one_radio);
    move_all_radio->setChecked(true);
    QGroupBox* move_method_group = new QGroupBox("Move Method");
    move_method_group->setLayout(move_method_layout);


    // Feedback layout
    QHBoxLayout* feedback_layout = new QHBoxLayout();
    progress_bar = new QProgressBar(this); // progress bar
    feedback_layout->addWidget(progress_bar);
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setMinimumWidth(100);
    progress_bar->setRange(0, 100);
    //progress_bar->setValue(50); // test

    //Text feedback layout
    QHBoxLayout* text_feedback_layout = new QHBoxLayout();
    info_label = new QLabel("Test", this);
    info_label->setAlignment(Qt::AlignCenter);
    text_feedback_layout->addWidget(info_label);
    info_label->setObjectName("textFeedback");
    info_label->setProperty("class", "text-feedback");


    //Adding to the master layout
    master_layout->addLayout(source_layout); // add to master layout
    master_layout->addLayout(destination_layout);
    master_layout->addLayout(scan_button_layout);
    master_layout->addWidget(move_method_group);
    master_layout->addWidget(scan_method_group);
    master_layout->addLayout(feedback_layout);
    master_layout->addLayout(text_feedback_layout);

    master_layout->setAlignment( Qt::AlignTop);

    setLayout(master_layout);
    connect_buttons();
}

// Connecting buttons to their methods here
void MainWindow::connect_buttons()
{
    //Browse source images folder section setup
    connect(browse_source_button, &QPushButton::clicked, this, &MainWindow::on_browse_source_clicked);
    connect(source_edit, &QLineEdit::returnPressed, this, &MainWindow::on_source_path_entered);

    //Browse destination folder section setup
    connect(browse_destination_button, &QPushButton::clicked, this, &MainWindow::on_destination_clicked);
    connect(dest_edit, &QLineEdit::returnPressed, this, &MainWindow::on_dest_path_entered);

    connect(scan_button, &QPushButton::clicked, this, &MainWindow::on_scan_clicked);

    // Connect search method the radio buttons
    connect(exact_match_radio, &QRadioButton::toggled, this, &MainWindow::change_search_method);
    connect(perceptual_hash_radio, &QRadioButton::toggled, this, &MainWindow::change_search_method);
    connect(mean_color_radio, &QRadioButton::toggled, this, &MainWindow::change_search_method);

    connect(move_all_radio, &QRadioButton::toggled, this, &MainWindow::change_move_method);
    connect(move_all_except_one_radio, &QRadioButton::toggled, this, &MainWindow::change_move_method);




}

void MainWindow::on_browse_source_clicked()
{
    //take the output and put it in a QString
    QString folder = QFileDialog::getExistingDirectory(this, "Select a source folder");

    if (!folder.isEmpty())
    {

        this->source_folder = folder;
        source_edit->setText(folder);
        update_scan_button_state();

    }

    else
    {

        this->source_folder.clear();
        update_scan_button_state();
        source_edit->setText("Invalid path!");

    }
}



void MainWindow::on_source_path_entered()
{
    QString folder = source_edit->text();
    check_entered_text(folder, "source");

    update_scan_button_state();

}

void MainWindow::on_dest_path_entered()
{
    QString folder = dest_edit->text();
    check_entered_text(folder, "dest");

    update_scan_button_state();

}

void MainWindow::check_entered_text(QString& folder, const QString& location)// validates urls in the text boxes
{
    folder = folder.trimmed();

    if (!folder.isEmpty() && QFileInfo(folder).exists() && QFileInfo(folder).isDir())
    {
        QString absolute_path = QFileInfo(folder).absoluteFilePath();

        if (location == "source")
        {
            this->source_folder = absolute_path;
            source_edit->setText(absolute_path);

        }
        else
        {
            this->destination_folder = absolute_path;
            dest_edit->setText(absolute_path);
        }
    }

    else
    {
        if (location == "source")
        {
            this->source_folder.clear();
            source_edit->setText("Invalid path!");

        }

        else
        {
            this->destination_folder.clear();
            dest_edit->setText("Invalid path!");
        }

    }

}

void MainWindow::on_destination_clicked()
{
    //take the output and put it in a QString
    QString folder = QFileDialog::getExistingDirectory(this, "Select a dest folder");

    if (!folder.isEmpty())
    {
        this->destination_folder = folder;
        dest_edit->setText(folder);
        update_scan_button_state();
    }

    else
    {
        this->destination_folder.clear();
        update_scan_button_state();
        dest_edit->setText("Invalid path!");
    }

}

void MainWindow::update_scan_button_state()
{
    if (!source_folder.isEmpty() && QFileInfo(source_folder).exists() && !destination_folder.isEmpty() &&
        QFileInfo(destination_folder).exists())
    {
        if (source_folder != destination_folder)
        {
            scan_button->setEnabled(true);
        }

    }

    else // issues without the explicit else
    {
        scan_button->setEnabled(false);
    }
}

void MainWindow::change_search_method()
{
    if (exact_match_radio->isChecked())
    {
        qDebug() << "Searching for exact match";
        current_search_method = search_method::exact_match;
    }

    else if (perceptual_hash_radio->isChecked())
    {
        qDebug() << "Searching for perceptual hash";
        current_search_method = search_method::perceptual_hash;
    }

    else if (mean_color_radio->isChecked())
    {
        qDebug() << "Searching for mean_color";
        current_search_method = search_method::mean_color;
    }

}

void MainWindow::set_search_method_algorithm()
{
    switch(current_search_method) {
        case search_method::exact_match:
            qDebug() << "Searching for exact match";
            //worker thread gets set here accordingly
            break;
        case search_method::perceptual_hash:
            qDebug() << "Searching for perceptual hash";
            break;
        case search_method::mean_color:
            qDebug() << "Searching for mean color";
            break;
    }

}

void MainWindow::change_move_method()
{
    if (move_all_radio->isChecked())
    {
        qDebug() << "Moving all files to destination";
    }

    else if (move_all_except_one_radio->isChecked())
    {
        qDebug() << "Moving all files except one to destination";
    }

}

void MainWindow::update_progress_bar(int x)
{
    qDebug() << "Progress: " << x << "%";
    progress_bar->setValue(x);
}

void MainWindow::load_file_paths()
{

    qDebug() << "Loading file paths";
    source_files.clear();  // Clearing the current list of urls
    QDir dir(source_folder);

    if (!dir.exists())
    {
        qDebug() << "Source folder does not exist!";
    }

    QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif", "*.tiff"};

    QStringList files = dir.entryList(filters, QDir::Files);

    for (const QString& file_name : files) {
        QString full_path = dir.absoluteFilePath(file_name);
        source_files.append(full_path);
    }

    qDebug() << "Found" << source_files.size() << "image files";

}


void MainWindow::on_scan_clicked()
{
    load_file_paths();

    QThread* thread = new QThread;
    ScanWorker* worker = new ScanWorker(current_search_method, source_files); // Remove 'this'

    connect(worker, &ScanWorker::progress_updated,
        this, &MainWindow::update_progress_bar,
        Qt::QueuedConnection);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ScanWorker::process);
    connect(worker, &ScanWorker::process_finished, thread, &QThread::quit);
    connect(worker, &ScanWorker::process_finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);



    connect(worker, &ScanWorker::process_finished, this, [this]() {
        is_scanning = false;
        scan_button->setEnabled(true);
    });

    thread->start();
    is_scanning = true;
    scan_button->setEnabled(false);
}