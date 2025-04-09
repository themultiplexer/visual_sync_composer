#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include "helper.h"
#include <GL/gl.h>
#include <QWidget>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>

#define NUM_POINTS 640

class OGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OGLWidget(int min, int max, int step, QWidget *parent = 0);
    ~OGLWidget();

    void setFrequencies(const std::vector<float> &newFrequencies, bool peak, float level);

    int getMin() const;
    int getMax() const;

    void setThresh(float newThresh);

    float getThresh() const;

    float getStart() const;
    void setStart(float newStart);

    float getEnd() const;
    void setEnd(float newEnd);

signals:
    void valueChanged();
    void mouseEnterEvent();
    void mouseLeaveEvent();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao, vao1;
    QOpenGLBuffer vertexPositionBuffer, vertexBuffer;
    GLuint shaderProgram;
    std::vector<float> frequencies;
    std::vector<float> smoothFrequencies;
    bool peaked;
    bool mouseDown;
    float start, end, peak, level, smoothLevel, thresh;
    FixedQueue<float, 1> levels;
    int min, max, step;
    void createVBO();
private:
    bool eventFilter(QObject *obj, QEvent *event);
    GLuint m_vbo;
    bool dragging;

    bool onLine;

    float dx;
    float prestart;
    float preend;
    bool inside;
    double gap;
};

#endif // OGLWIDGET_H
