//
// Created by Joshua Fisher on 9/17/23.
//

#ifndef ONSCRIPTER_EN_QTDIALOGS_H
#define ONSCRIPTER_EN_QTDIALOGS_H

#include "QInputDialog"

#include "QLabel"
#include "QMenuBar"
#include "QPushButton"
#include "QSlider"
#include "QStyle"

#include "QBoxLayout"

#include "ONScripterLabel.h"


class VolumeDialog : public QDialog
{
    Q_OBJECT
public:
    static void adjustVolumeSliders(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent)
    {
      VolumeDialog* dialog = new VolumeDialog(onscripter, voice_volume, se_volume, music_volume, parent);
      dialog->setModal(true);
      dialog->show();
      dialog->setWindowFlag(Qt::WindowType::Dialog, true);
      dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
      dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
      dialog->setWindowFlag(Qt::WindowType::WindowSystemMenuHint, false);
      dialog->exec();

      delete dialog;
    }



private:
    explicit VolumeDialog(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent = nullptr)
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

    QLineEdit* m_lineEdit;
    QString m_textValue;
    ONScripterLabel* onscripter;
};






class InputStrDialog : public QDialog
{
    Q_OBJECT
public:
    static std::string getInputStr(const std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr)
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
        err = dialog->exec();

        toReturn = dialog->m_lineEdit->text().toStdString();

        delete dialog;
      }

      return toReturn;
    }



private:
    explicit InputStrDialog(const std::string& label, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr)
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

      // NOTE: These probably need to be relative to the parent window.
      int width = w == NULL ? 0 : *w;
      int height = h == NULL ? 0 : *h;

      move(width, height);
      //setBaseSize()
    }

    QLineEdit* m_lineEdit;

    QString m_textValue;
};

class VersionDialog : public QDialog
{
    Q_OBJECT
public:
    static void showVersion(const std::string& display, QWidget* parent = nullptr)
    {
      VersionDialog* dialog = new VersionDialog(display);
      dialog->setModal(true);
      dialog->show();
      //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
      dialog->setWindowFlag(Qt::WindowType::Dialog, true);
      dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
      dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
      dialog->exec();

      delete dialog;
    }

private:
    explicit VersionDialog(const std::string& label, QWidget* parent = nullptr)
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

    QLineEdit* m_lineEdit;

    QString m_textValue;
};



class ExitDialog : public QDialog
{
    Q_OBJECT
public:
    static bool shouldExit(const std::string& display, QWidget* parent = nullptr)
    {
      ExitDialog* dialog = new ExitDialog(display);
      dialog->setModal(true);
      dialog->show();
      //dialog->setWindowFlag(Qt::WindowType::WindowCloseButtonHint, false);
      dialog->setWindowFlag(Qt::WindowType::Dialog, true);
      dialog->setWindowFlag(Qt::WindowType::CustomizeWindowHint, true);
      dialog->setWindowFlag(Qt::WindowType::WindowTitleHint, true);
      bool ret = !!dialog->exec();

      delete dialog;

      return ret;
    }

private:
    explicit ExitDialog(const std::string& label, QWidget* parent = nullptr)
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

    QLineEdit* m_lineEdit;

    QString m_textValue;
};




#endif //ONSCRIPTER_EN_QTDIALOGS_H
