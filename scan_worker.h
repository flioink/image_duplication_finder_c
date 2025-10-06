
#pragma once
#include "search_methods.h"
#include "Image_duplication_finder_C.h"

class ScanWorker : public QObject
{
    Q_OBJECT
    public:
    explicit ScanWorker(search_method method, const QStringList& files);

    public slots:
    void process();  //Contains switch for algorithms

    signals:
    void progress_updated(int percent);
    void process_finished();
    void duplicates_found(const QMap<QString, QStringList>& duplicates); // to return the resulting list
    void status_update(const QString& message); // for the info label

    private:
    search_method m_method;
    QStringList m_files;

    void process_exact_match();
    void process_perceptual_hash();

    QString average_hash(const QImage &img);

    void process_mean_color();

    void load_file_paths();

    QString hashing(const QString &current_file);
};