#include "scan_worker.h"
#include <QDir>
#include <QCryptographicHash>
#include <QThread>
#include <QCoreApplication>



ScanWorker::ScanWorker(search_method method, const QStringList& files)
    : m_method(method), m_files(files)
{
    qDebug() << "Worker constructor - files received:" << m_files.size();
}


void ScanWorker::process()
{
    qDebug() << "Worker::process() called - method:" << static_cast<int>(m_method);

    switch(m_method) {
        case search_method::exact_match:
            qDebug() << "Calling process_exact_match()";
            process_exact_match();
            break;
        case search_method::perceptual_hash:
            qDebug() << "Calling process_perceptual_hash()";
            process_perceptual_hash();
            break;
        case search_method::mean_color:
            qDebug() << "Calling process_mean_color()";
            process_mean_color();
            break;
    }
    qDebug() << "Worker::process() completed";
}



void ScanWorker::process_perceptual_hash()
{

}

void ScanWorker::process_mean_color()
{

}

QString ScanWorker::hashing(const QString &current_file)
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

void ScanWorker::process_exact_match()
{
    QMap<QString, QStringList> hash_to_file_map;

    //Convert files to hashes and add to the file to hash map
    for (int i = 0; i < m_files.size(); ++i)
    {
        QString digest = hashing(m_files[i]);
        hash_to_file_map[digest].append(m_files[i]);

        //Progress update
        int progress = ((i + 1) * 100) / m_files.size(); // calculate percentage

        emit progress_updated(progress);
    }

    //Duplicate detection
    QMap<QString, QStringList> results_map;
    for (auto it = hash_to_file_map.constBegin(); it != hash_to_file_map.constEnd(); it++)
        {
            if (it.value().size() > 1)
                {
                    results_map[it.key()] = it.value();
                }
        }


    qDebug() << results_map.size() << " results found";

    qDebug() << "Worker completed processing";

    if (!results_map.isEmpty())
    {
        emit duplicates_found(results_map);
    }

    emit process_finished();
}

