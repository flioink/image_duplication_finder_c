#include <QApplication>
#include <QFile>
#include "Image_duplication_finder_C.h"

QString set_styling();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString style = set_styling();
    app.setStyleSheet(style);

    MainWindow window;

    window.resize(450, 300);
    window.setWindowTitle("Duplicated image detector");
    window.show();
    return app.exec();
}

QString set_styling()
{
    QString path = R"(C:\DL\Cpp_projects\Image_duplication_finder_C\style.css)";
    QFile style_file(path);

    if (style_file.open(QFile::ReadOnly))
    {
        QString style_sheet = QLatin1String(style_file.readAll());
        style_file.close();
        return style_sheet;
    }

    qDebug() << "Could not open stylesheet file!";

}