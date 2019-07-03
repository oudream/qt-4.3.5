#include <QtGui>
#include <QApplication>

class Widget : public QWidget
{
public:
    Widget(QWidget *parent = 0);
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QWidget *firstPageWidget = new QWidget;
    QWidget *secondPageWidget = new QWidget;
    QWidget *thirdPageWidget = new QWidget;

    QStackedLayout *stackedLayout = new QStackedLayout;
    stackedLayout->addWidget(firstPageWidget);
    stackedLayout->addWidget(secondPageWidget);
    stackedLayout->addWidget(thirdPageWidget);

    QComboBox *pageComboBox = new QComboBox;
    pageComboBox->addItem(tr("Page 1"));
    pageComboBox->addItem(tr("Page 2"));
    pageComboBox->addItem(tr("Page 3"));
    connect(pageComboBox, SIGNAL(activated(int)),
            stackedLayout, SLOT(setCurrentIndex(int)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pageComboBox);
    mainLayout->addLayout(stackedLayout);
    setLayout(mainLayout);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.show();
    return app.exec();
}

