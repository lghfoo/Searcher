#ifndef UTIL_H
#define UTIL_H
#include"commonheader.h"
class Util{
public:
    static void shrinkButton(QPushButton* button){
        auto size = QFontMetrics(button->font())
                .size(Qt::TextSingleLine, button->text())
                + QSize(8, 11);
        if(size.width() < size.height())
            size.setWidth(size.height());
        button->setFixedSize(size);
    }
};

#endif // UTIL_H
