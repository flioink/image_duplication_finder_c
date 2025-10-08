#include "scan_worker.h"
#include <QDir>
#include <QCryptographicHash>
#include <QException>
#include <QImageReader>
//#include <opencv2/opencv.hpp>



ScanWorker::ScanWorker(search_method method, const QStringList& files)
    : m_method(method), m_files(files)
{
    //qDebug() << "Worker constructor - files received:" << m_files.size();
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
    // qDebug() << "Worker::process() completed";
}



QString ScanWorker::hashing(const QString &current_file)
{
    QFile file(current_file);
    if (!file.open(QIODevice::ReadOnly))
    {
        //show error
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
    QString status;


    //Convert files to hashes and add to the file to hash map
    for (int i = 0; i < m_files.size(); ++i)
    {
        QString digest = hashing(m_files[i]);
        hash_to_file_map[digest].append(m_files[i]);

        //Progress update
        int progress = ((i + 1) * 100) / m_files.size(); // calculate percentage

        QString current_number = QString::number(i + 1);
        status = "Checking file " + current_number + " out of " + QString::number(m_files.size());

        emit progress_updated(progress);
        emit status_update(status);
    }

    //Duplicate detection
    int result_counter = 0;
    QMap<QString, QStringList> results_map;
    for (auto it = hash_to_file_map.constBegin(); it != hash_to_file_map.constEnd(); it++)
        {
            if (it.value().size() > 1)
                {
                    result_counter+= it.value().size();
                    results_map[it.key()] = it.value();
                }
        }


    qDebug() << results_map.size() << " results found";
    status = "Identical images found: " + QString::number(result_counter);
    emit status_update(status);

    //qDebug() << "Worker completed processing";

    if (!results_map.isEmpty())
    {
        emit duplicates_found(results_map);
    }

    emit process_finished();
}

void ScanWorker::process_perceptual_hash()
{
    QMap<QString, QStringList> img_hashes_map;

    QString status;

    for (int i = 0; i < m_files.size(); ++i)
    {
        int progress = ((i + 1) * 100) / m_files.size();
        emit progress_updated(progress);

        try
        {
            QImage img(m_files[i]);

            if (img.isNull())
            {
                qDebug() << "Failed to load image " << m_files[i];
                continue;

            }

            img = img.convertToFormat(QImage::Format_RGB888);

            QString img_hash = calculate_average_hash(img);

            img_hashes_map[img_hash].append(m_files[i]);
            QString current_number = QString::number(i + 1);
            status = "Checking file " + current_number + " out of " + QString::number(m_files.size());
            emit status_update(status);


        }
        catch (const QException& e)
        {
            qDebug() << e.what();
        }

    }

    //Duplicate detection
    int result_counter = 0;
    QMap<QString, QStringList> results_map;
    for (auto it = img_hashes_map.constBegin(); it != img_hashes_map.constEnd(); it++)
    {
        if (it.value().size() > 1)
        {
            results_map[it.key()] = it.value();
            result_counter+= it.value().size();
        }
    }

    if (!results_map.isEmpty())
        {
            emit duplicates_found(results_map);

            status = "Visually similar images found: " + QString::number(result_counter);
            emit status_update(status);
        }
    else
    {
        status = "No matching images found.";
        emit status_update(status);
    }

    emit process_finished();

}

QString ScanWorker::calculate_average_hash(const QImage& img)
{
    if (img.isNull())
    {
        return QString(); //Return empty hash for null images
    }

    qint32 kernel = 64;
    QImage small = img.scaled(kernel, kernel, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    qint64 total = 0;
    for (int y = 0; y < small.height(); ++y)
    {
        for (int x = 0; x < small.width(); ++x)
        {
            QColor color = small.pixelColor(x, y);

            total += color.red() + color.green() + color.blue();
        }
    }

    qint64 avg = total / (8 * 8 * 3);

    QString hash;
    for (int y = 0; y < small.height(); ++y)
    {
        for (int x = 0; x < small.width(); ++x)
        {
            QColor color = small.pixelColor(x, y);

            qint64 pixel_value = (color.red() + color.green() + color.blue())/3;

            if (pixel_value > avg)
            {
                hash.append('1');
            }
            else
            {
                hash.append('0');
            }
        }
    }

    return hash;
}

QString ScanWorker::calculate_mean_hash(const QImage& img)
{
    QImage img_result = img.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    qint32 total_red = 0, total_green = 0, total_blue = 0;


    for (int y = 0; y < img_result.height(); ++y)
    {
        for (int x = 0; x < img_result.width(); ++x)
        {
            QColor color = img_result.pixelColor(x, y);

            total_red += color.red();
            total_green += color.green();
            total_blue += color.blue();
        }
    }

    qint32 mean_red = total_red / (64*64);
    qint32 mean_green = total_green / (64*64);
    qint32 mean_blue = total_blue / (64*64);

    QString mean_hash_value = QString("%1%2%3")
    .arg(mean_red, 2, 16, QLatin1Char('0'))
    .arg(mean_green, 2, 16, QLatin1Char('0'))
    .arg(mean_blue, 2, 16, QLatin1Char('0'));

    return mean_hash_value;

}

void ScanWorker::process_mean_color()
{
    QMap<QString, QStringList> img_hashes_map;

    QString status;

    for (int i = 0; i < m_files.size(); ++i)
    {
        int progress = ((i + 1) * 100) / m_files.size();
        emit progress_updated(progress);

        try
        {
            QImage img(m_files[i]);

            if (img.isNull())
            {
                qDebug() << "Failed to load image " << m_files[i];
                QImageReader reader(m_files[i]);
                qDebug() << "Format:" << reader.format();
                qDebug() << "Error:" << reader.errorString();
                qDebug() << "Size:" << QFileInfo(m_files[i]).size();

                continue;

            }

            img = img.convertToFormat(QImage::Format_RGB888);

            QString img_hash = calculate_mean_hash(img);

            img_hashes_map[img_hash].append(m_files[i]);

            QString current_number = QString::number(i + 1);
            status = "Checking file " + current_number + " out of " + QString::number(m_files.size());
            emit status_update(status);


        }
        catch (const QException& e)
        {
            qDebug() << e.what();
        }

    }

    //Duplicate detection
    int result_counter = 0;
    QMap<QString, QStringList> results_map;
    for (auto it = img_hashes_map.constBegin(); it != img_hashes_map.constEnd(); it++)
    {
        if (it.value().size() > 1)
        {
            results_map[it.key()] = it.value();
            result_counter+= it.value().size();
        }
    }

    if (!results_map.isEmpty())
    {
        emit duplicates_found(results_map);

        status = "Visually similar images found: " + QString::number(result_counter);
        emit status_update(status);
    }
    else
    {
        status = "No matching images found.";
        emit status_update(status);
    }

    emit process_finished();

}