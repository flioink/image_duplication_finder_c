#pragma once

#include <QMessageBox>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QProgressBar;
class QRadioButton;


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    QString hashing(const QString& current_file);



private slots:

    void on_browse_clicked();
    void on_source_path_entered();

    void load_file_paths();
    void on_scan_clicked();
    void on_destination_clicked();
    void on_dest_path_entered();



private:
    QString source_folder;
    QString destination_folder;

    QStringList source_files;

    QLabel* source_label;
    QLabel* destination_label;

    QLineEdit* source_edit;
    QLineEdit* dest_edit;

    QPushButton* browse_source_button;
    QPushButton* browse_destination_button;
    QPushButton* scan_button;

    QProgressBar* progress_bar;

    QRadioButton* exact_match_radio;
    QRadioButton* perceptual_hash_radio;
    QRadioButton* mean_color_radio;


    void build_UI();
    void update_scan_button_state();
    void check_entered_text(QString& folder, const QString& location);
    void style();
};


