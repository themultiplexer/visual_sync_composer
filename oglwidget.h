#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <GL/gl.h>
#include <QWidget>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#define NUM_POINTS 512

class OGLWidget : public QOpenGLWidget
{
public:
    OGLWidget(QWidget *parent = 0);
    ~OGLWidget();

    void setFrequencies(const std::vector<float> &newFrequencies, bool peak);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QOpenGLShaderProgram *program;
    GLuint vertexArray, vertexBuffer;
    GLuint shaderProgram;
    std::vector<float> frequencies;
    bool peaked;
    bool mouseDown;
    float start, end;

    void createVBO();
private:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // OGLWIDGET_H
