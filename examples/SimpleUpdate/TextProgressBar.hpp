#ifndef TEXTPROGRESSBAR_HPP
#define TEXTPROGRESSBAR_HPP

#include <QString>

class TextProgressBar
{
public:
    void clear();
    void update();
    void setMessage(const QString &message);
    void setStatus(int, qint64 value, qint64 maximum);

private:
    QString message;
    int percent = 0;
    qint64 value = 0;
    qint64 maximum = -1;
    int iteration = 0;
};

#endif
