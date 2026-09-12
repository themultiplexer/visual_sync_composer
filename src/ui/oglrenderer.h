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
#include <QQuickFramebufferObject>

#include <QtQml/qqmlregistration.h>


class OGLRenderer : public QQuickFramebufferObject::Renderer
{
public:
    void render() override
    {
        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // your OpenGL rendering here

        update();


    }

    void init() {
            
    }
};