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

SDL_Rect RectFromWidget(QWidget* widget);

class VolumeDialog : public QDialog
{
    Q_OBJECT
public:
    static void adjustVolumeSliders(SDL_Rect window_rect, ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent = nullptr);

private:
    explicit VolumeDialog(ONScripterLabel* onscripter, int& voice_volume, int& se_volume, int& music_volume, QWidget* parent = nullptr);

    QLineEdit* m_lineEdit;
    QString m_textValue;
    ONScripterLabel* onscripter;
};

class InputStrDialog : public QDialog
{
    Q_OBJECT
public:
    static std::string getInputStr(SDL_Rect window_rect, const std::string& display, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr);

private:
    explicit InputStrDialog(const std::string& label, int maximumInputLength, bool forceDoubleByte, const int* w, const int* h, const int* input_w, const int* input_h, QWidget* parent = nullptr);

    QLineEdit* m_lineEdit;
    QString m_textValue;
};

class VersionDialog : public QDialog
{
    Q_OBJECT
public:
    static void showVersion(SDL_Rect window_rect, const std::string& display, QWidget* parent = nullptr);

private:
    explicit VersionDialog(const std::string& label, QWidget* parent = nullptr);

    QLineEdit* m_lineEdit;
    QString m_textValue;
};



class ExitDialog : public QDialog
{
    Q_OBJECT
public:
    static bool shouldExit(SDL_Rect window_rect, const std::string& display, QWidget* parent = nullptr);

private:
    explicit ExitDialog(const std::string& label, QWidget* parent = nullptr);

    QLineEdit* m_lineEdit;
    QString m_textValue;
};




#endif //ONSCRIPTER_EN_QTDIALOGS_H
