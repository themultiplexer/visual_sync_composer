#include "radioselection.h"

RadioSelection::RadioSelection(std::string title, std::vector<std::string> selection, std::function<void(int)> callback, int defaultValue, QWidget *parent) : QGroupBox(parent) {
    QHBoxLayout *selectorLayout = new QHBoxLayout(this);
    selectorLayout->addWidget(new QLabel(title.c_str()));
    for (int i = 0; i < selection.size(); i++) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(selection[i].c_str());
        if (i == defaultValue) {
            radio->setChecked(true);
            callback(i);
        }
        //std::bind(callback, std::placeholders::_1, i)
        connect(radio, &QRadioButton::toggled, this, [callback, i]() {
            callback(i);
        });
        selectorLayout->addWidget(radio);
    }
    this->setMaximumHeight(45);
    this->setStyleSheet("QGroupBox { padding: 0px; }");
    selectorLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(selectorLayout);
}
