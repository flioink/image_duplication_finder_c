
#include "Image_duplication_finder_C.h" // current name of the project
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug> // for printing results to console
#include <QMap>
#include <QProgressBar>
#include <QGroupBox>
#include <QRadiobutton>






MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    build_UI();
}

void MainWindow::build_UI()
{
    int browse_buttons_width = 90;
    int text_boxes_width = 280;
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
    scan_button->setFixedWidth(200);
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

    QHBoxLayout* feedback_layout = new QHBoxLayout();

    // Method layout
    QHBoxLayout* method_layout = new QHBoxLayout();
    exact_match_radio = new QRadioButton("Exact Match");
    perceptual_hash_radio = new QRadioButton("Perceptual Hash");
    mean_color_radio = new QRadioButton("Mean Color");
    method_layout->addWidget(exact_match_radio);
    method_layout->addWidget(perceptual_hash_radio);
    method_layout->addWidget(mean_color_radio);
    QGroupBox* method_group = new QGroupBox("Search Method");
    method_group->setLayout(method_layout);

    //Scan layout
    QHBoxLayout* scan_layout = new QHBoxLayout();
    scan_layout->addWidget(scan_button);

    // Feedback layout
    QProgressBar* progress_bar = new QProgressBar(this); // progress bar
    feedback_layout->addWidget(progress_bar);
    progress_bar->setAlignment(Qt::AlignCenter);
    progress_bar->setMinimumWidth(100);
    progress_bar->setRange(0, 100);
    progress_bar->setValue(50); // test

    //Browse source images folder section setup
    connect(browse_source_button, &QPushButton::clicked, this, &MainWindow::on_browse_clicked);
    connect(source_edit, &QLineEdit::returnPressed, this, &MainWindow::on_source_path_entered);

    //Browse destination folder section setup
    connect(browse_destination_button, &QPushButton::clicked, this, &MainWindow::on_destination_clicked);
    connect(dest_edit, &QLineEdit::returnPressed, this, &MainWindow::on_dest_path_entered);

    connect(scan_button, &QPushButton::clicked, this, &MainWindow::on_scan_clicked);

    //Adding to the master layout
    master_layout->addLayout(source_layout); // add to master layout
    master_layout->addLayout(destination_layout);
    master_layout->addLayout(scan_layout);
    master_layout->addWidget(method_group);
    master_layout->addLayout(feedback_layout);

    master_layout->setAlignment( Qt::AlignTop);

    setLayout(master_layout);
}

void MainWindow::on_browse_clicked()
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


QString MainWindow::hashing(const QString& current_file)
{
    QFile file(current_file);
    if (!file.open(QIODevice::ReadOnly))
    {
        //do something to show error
        qDebug("Failed to open file");
    }

    const qint64 buffer_size = 4096;

    QCryptographicHash hash(QCryptographicHash::Sha256);


    while (!file.atEnd())
    {
        QByteArray chunk = file.read(buffer_size);
        hash.addData(chunk);
    }

    QString digest = hash.result().toHex();

    return digest;

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
    QMap<QString, QStringList> hash_to_file_map;

    QList<QString> results_map;

    for (const QString& file_path : source_files )
    {
        QString digest = hashing(file_path);
        hash_to_file_map[digest].append(file_path);

    }

    int index = 0;
    for (auto it = hash_to_file_map.constBegin(); it != hash_to_file_map.constEnd(); it++)
    {
        index++ ;
        if (it.value().size() > 1)
        {
            qDebug() << "Duplicate files for hash" << it.key() << ":" << it.value().size();

            results_map.append(it.value());
        }
    }

    qDebug() << index + 1 << "items scanned. Scan complete!";
    qDebug() << results_map.size() << " results found";

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





