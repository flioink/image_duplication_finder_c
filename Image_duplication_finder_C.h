#pragma once

#include <QMessageBox>
#include <QWidget>
#include "search_methods.h"

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

    void on_browse_source_clicked();
    void on_source_path_entered();
    void on_destination_clicked();
    void on_dest_path_entered();

    void load_file_paths();
    void on_scan_clicked();

    void change_search_method();
    void set_search_method_algorithm();
    void change_move_method();


public slots:

    void update_progress_bar(int x);



private:
    QStringList source_files;
    QString source_folder;
    QString destination_folder;
    //enum
    search_method current_search_method;

    QLabel* source_label;
    QLabel* destination_label;
    QLabel* info_label;

    QLineEdit* source_edit;
    QLineEdit* dest_edit;

    QPushButton* browse_source_button;
    QPushButton* browse_destination_button;
    QPushButton* scan_button;

    QProgressBar* progress_bar;

    QRadioButton* exact_match_radio;
    QRadioButton* perceptual_hash_radio;
    QRadioButton* mean_color_radio;
    QRadioButton* move_all_except_one_radio;
    QRadioButton* move_all_radio;

    bool is_scanning = false;


    void build_UI();
    void connect_buttons();
    void update_scan_button_state();
    void check_entered_text(QString& folder, const QString& location);


};


