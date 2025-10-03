
#pragma once
#include <QObject>
#include "search_methods.h"

class ScanWorker : public QObject
{
    Q_OBJECT
    public:
    explicit ScanWorker(search_method method, const QStringList& files);
    //~ScanWorker();

    public slots:
    void process();  // Contains switch statement for algorithms

    signals:
    void progress_updated(int percent);
    void process_finished();

    private:
    search_method m_method;
    QStringList m_files;


    void process_exact_match();
    void process_perceptual_hash();
    void process_mean_color();

    void load_file_paths();



    QString hashing(const QString &current_file);
};