#include "tubewidget.h"
#include <cmath>


#define PROGRAM_VERTEX_ATTRIBUTE 0


TubeWidget::TubeWidget(QWidget *parent)
    : QOpenGLWidget(parent), peak(0.0), time(0.0)
{

    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
}

TubeWidget::~TubeWidget()
{

}

void TubeWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0);
    glPointSize(10.0);

    vao1.create();
    vao1.bind();
    vertexBuffer.create();
    vertexBuffer.bind();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    vertexBuffer.release();
    vao1.release();

    // Create and compile the vertex shader using a raw string literal.
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc = R"(
            #version 330 core
            layout (location = 0) in vec2 vertex;
            out vec2 pos;
            uniform float peak;


            void main() {
                pos = vertex;
                gl_Position = vec4(vertex, 0.0, 1.0);
            }
        )";
    vshader->compileSourceCode(vsrc);

    // Create and compile the fragment shader.
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc = R"(
            #version 330 core
            in vec2 pos;
            out vec4 FragColor;
            uniform float peak;
            uniform vec3 color;

            void main() {
                float y = smoothstep(-0.75, -0.2, pos.x) - smoothstep(0.2, 0.75, pos.x);
                vec3 c = color * y;
                FragColor = vec4(c * (float)(pos.y < (-1.0 + 2 * peak)), 1.0);
            }
        )";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->link();

    QVector<QVector2D> vertices;
    vertices << QVector2D(-1.f,  1.f)
             << QVector2D(-1.f, -1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D(-1.f,  1.f)
             << QVector2D( 1.f, -1.f)
             << QVector2D( 1.f, 1.f);

    vao.create();
    vao.bind();
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();
    vertexPositionBuffer.allocate(vertices.constData(), vertices.size() * sizeof(QVector2D));
    program->bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(QVector2D));
    vertexPositionBuffer.release();
    vao.release();
    program->release();
}

void TubeWidget::paintGL()
{
    if (peaked) {
        glClearColor(1,0,0,1);
        peak = 1.0;
        peaked = false;
    } else {
        glClearColor(0,0,0,1);
        if (peak > 0) {
            peak -= 0.05;
        }
    }

    program->bind();
    program->setUniformValue("peak", peak);
    program->setUniformValue("color", QVector3D(color.red(), color.green(), color.blue()));
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();
    time += 0.05;
}

void TubeWidget::setPeaked(QColor newColor) {
    peaked = true;
    //qDebug() << newColor;
    color = newColor.toRgb();
}

