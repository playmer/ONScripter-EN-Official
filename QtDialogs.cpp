//
// Created by Joshua Fisher on 9/17/23.
//


#include "QtDialogs.h"


SDL_Rect RectFromWidget(QWidget* widget)
{
    QRect geo = widget->geometry();
    int x = geo.x();
    int y = geo.y();

    SDL_Rect to_return;
    to_return.x = geo.x();
    to_return.y = geo.y();
    to_return.w = geo.width();
    to_return.h = geo.height();

    return to_return;
}



void VolumeDialog::adjustVolumeSliders(SDL_Rect window_rect, ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent)
{
    VolumeDialog* dialog = new VolumeDialog(onscripter, voice_volume, se_volume, music_volume, parent);
    dialog->setModal(true);
    dialog->show();
    dialog->setWindowFlag(Qt::WindowType::Dialog, true);
    dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
    dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
    dialog->setWindowFlag(Qt::WindowType::WindowSystemMenuHint, false);

    SDL_Rect dialog_rect = RectFromWidget(dialog);
    dialog_rect = Window::CenterDialog(dialog_rect, window_rect);
    dialog->move(dialog_rect.x, dialog_rect.y);

    dialog->exec();

    delete dialog;
}

VolumeDialog::VolumeDialog(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("Volume Dialog"));
    setWindowIcon(this->style()->standardIcon(QStyle::SP_MediaVolume));

    QLabel* slider1Label = new QLabel(QString::fromUtf8("Voice")); // voice_volume
    QSlider* slider1 = new QSlider(Qt::Vertical, parent);
    slider1->setMinimum(0);
    slider1->setMaximum(100);
    slider1->setValue(voice_volume);
    QVBoxLayout* slider1Layout = new QVBoxLayout;
    slider1Layout->addWidget(slider1Label);
    slider1Layout->addWidget(slider1);
    QObject::connect(slider1, &QSlider::sliderMoved, [=](int position) {
        onscripter->SetVoiceVolume(position);
        });

    QLabel* slider2Label = new QLabel(QString::fromUtf8("SFX")); // se_volume
    QSlider* slider2 = new QSlider(Qt::Vertical, parent);
    slider2->setMinimum(0);
    slider2->setMaximum(100);
    slider2->setValue(se_volume);
    QVBoxLayout* slider2Layout = new QVBoxLayout;
    slider2Layout->addWidget(slider2Label);
    slider2Layout->addWidget(slider2);
    QObject::connect(slider2, &QSlider::sliderMoved, [=](int position) {
        onscripter->SetSfxVolume(position);
        });

    QLabel* slider3Label = new QLabel(QString::fromUtf8("BGM")); //music_volume
    QSlider* slider3 = new QSlider(Qt::Vertical, parent);
    slider3->setMinimum(0);
    slider3->setMaximum(100);
    slider3->setValue(music_volume);
    QVBoxLayout* slider3Layout = new QVBoxLayout;
    slider3Layout->addWidget(slider3Label);
    slider3Layout->addWidget(slider3);
    QObject::connect(slider3, &QSlider::sliderMoved, [=](int position) {
        onscripter->SetMusicVolume(position);
        });

    QHBoxLayout* mainLayout = new QHBoxLayout;

    mainLayout->addLayout(slider1Layout);
    mainLayout->addLayout(slider2Layout);
    mainLayout->addLayout(slider3Layout);

    setLayout(mainLayout);

    resize(200, 200);
}


std::string InputStrDialog::getInputStr(SDL_Rect window_rect, const std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent)
{
    std::string toReturn;
    int err = 0;
    while (err == 0)
    {
        InputStrDialog* dialog = new InputStrDialog(display, maximumInputLength, forceDoubleByte, w, h, input_w, input_h, parent);
        dialog->setModal(true);
        dialog->show();
        dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
        dialog->setWindowFlag(Qt::WindowType::Dialog, true);
        dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
        dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);

        SDL_Rect dialog_rect = RectFromWidget(dialog);
        dialog_rect = Window::CenterDialog(dialog_rect, window_rect);
        dialog->move(dialog_rect.x, dialog_rect.y);

        err = dialog->exec();

        toReturn = dialog->m_lineEdit->text().toStdString();

        delete dialog;
    }

    return toReturn;
}


InputStrDialog::InputStrDialog(const std::string& label, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent)
    : QDialog(parent)
{
    QString display = "Input String Dialog";
    this->setWindowTitle(display);

    QString labelText = QString::fromStdString(label);

    QHBoxLayout* inputLayout = new QHBoxLayout;

    QPushButton* okButton = new QPushButton("Ok", this);
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setMaxLength(maximumInputLength);

    QObject::connect(okButton, &QPushButton::clicked, [=]() {
        this->accept();
        this->done(1);
    });

    inputLayout->addWidget(m_lineEdit);
    inputLayout->addWidget(okButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    mainLayout->addWidget(new QLabel(labelText));
    mainLayout->addLayout(inputLayout);
    setLayout(mainLayout);

    if (w != NULL) {
        setFixedSize(*w, *h);
        m_lineEdit->setFixedSize(*input_w, *input_h);
    }
}


void VersionDialog::showVersion(SDL_Rect window_rect, const std::string& display, QWidget* parent)
{
    VersionDialog* dialog = new VersionDialog(display);
    dialog->setModal(true);
    dialog->show();
    //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
    dialog->setWindowFlag(Qt::WindowType::Dialog, true);
    dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
    dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);

    SDL_Rect dialog_rect = RectFromWidget(dialog);
    dialog_rect = Window::CenterDialog(dialog_rect, window_rect);
    dialog->move(dialog_rect.x, dialog_rect.y);

    dialog->exec();

    delete dialog;
}


VersionDialog::VersionDialog(const std::string& label, QWidget* parent)
    : QDialog(parent)
{
    QIcon messageInfoIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);

    QString display = "Version Dialog";
    setWindowTitle(display);
    setWindowIcon(messageInfoIcon);

    QHBoxLayout* inputLayout = new QHBoxLayout;

    QString labelText = QString::fromStdString(label);
    inputLayout->addWidget(new QLabel(labelText));

    QPushButton* okButton = new QPushButton("Ok", this);

    QObject::connect(okButton, &QPushButton::clicked, [=]() {
        this->accept();
        this->done(1);
        });

    QVBoxLayout* mainLayout = new QVBoxLayout;

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(okButton);
    setLayout(mainLayout);
}

bool ExitDialog::shouldExit(SDL_Rect window_rect, const std::string& display, QWidget* parent)
{
    ExitDialog* dialog = new ExitDialog(display);
    dialog->setModal(true);
    dialog->show();
    //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
    dialog->setWindowFlag(Qt::WindowType::Dialog, true);
    dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
    dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);

    SDL_Rect dialog_rect = RectFromWidget(dialog);
    dialog_rect = Window::CenterDialog(dialog_rect, window_rect);
    dialog->move(dialog_rect.x, dialog_rect.y);

    bool ret = !!dialog->exec();

    delete dialog;

    return ret;
}

ExitDialog:: ExitDialog(const std::string& label, QWidget* parent)
    : QDialog(parent)
{
    QIcon messageInfoIcon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);

    QString display = "Exit Dialog";
    setWindowTitle(display);
    setWindowIcon(messageInfoIcon);

    QPushButton* yesButton = new QPushButton("Yes", this);

    QObject::connect(yesButton, &QPushButton::clicked, [=]() {
        this->accept();
        this->done(1);
        });
    QPushButton* noButton = new QPushButton("No", this);

    QObject::connect(noButton, &QPushButton::clicked, [=]() {
        this->reject();
        this->done(0);
        });

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    QString labelText = QString::fromStdString(label);
    mainLayout->addWidget(new QLabel(labelText));
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

#include "QtDialogs.moc"
