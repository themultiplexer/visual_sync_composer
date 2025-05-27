#ifndef GLTEXT_H
#define GLTEXT_H

#include "wifieventprocessor.h"
#include <GL/gl.h>
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

class GLText : public QOpenGLWidget, public WifiTextEventReceiver
{
public:
    GLText(QWidget *parent = 0);
    ~GLText();

    void setText(std::string text);
    void textEvent(const char *text) override;

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();


    std::string text;
    GLuint shaderProgram;
};

#endif // GLTEXT_H
