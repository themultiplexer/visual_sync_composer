#include "oglwidget.h"
#include "qdebug.h"
#include <cmath>


#define PROGRAM_VERTEX_ATTRIBUTE 0


OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    for (int i = 0; i < NUM_POINTS; ++i) {
            frequencies.push_back(0.0);
    }
    setMouseTracking(true);
    installEventFilter(this);
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);

    start = 0.0;
    end = 1.0;
}

OGLWidget::~OGLWidget()
{

}

void OGLWidget::createVBO() {
    time_t seconds;
    seconds = time (NULL);

    std::vector<float> vertices;
    for (int i = 0; i < NUM_POINTS; ++i) {
        float from = -0.9 + ((float)i/NUM_POINTS) * 1.8;
        vertices.push_back(from);
        vertices.push_back(frequencies[i] - 0.9);
    }
    vertexBuffer.bind();
    vertexBuffer.allocate(vertices.data(), vertices.size() * sizeof(float));
    //vertexBuffer.release();
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHTING);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);
    glLineWidth(5.0);
    glPointSize(10.0);

    vao1.create();
    vao1.bind();
    vertexBuffer.create();
    vertexBuffer.bind();
    OGLWidget::createVBO();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    vertexBuffer.release();
    vao1.release();


    // Create and compile the vertex shader using a raw string literal.
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc = R"(
            #version 330 core
            layout (location = 0) in vec2 vertex;
            uniform int pressed;
            uniform float start;
            uniform float end;
            out vec2 pos;

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
            uniform int pressed;
            uniform float start;
            uniform float end;
            void main() {
                int visible = (int)(pos.x > start && pos.x < end);
                FragColor = vec4(visible, visible, visible, 0.5); // White color
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

void OGLWidget::paintGL()
{
    if (peaked) {
        glClearColor(1,0,0,1);
    } else {
        glClearColor(0,0,0,1);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    OGLWidget::createVBO();
    vao1.bind();
    glDrawArrays(GL_LINE_STRIP, 0, NUM_POINTS);
    vao1.release();

    program->bind();
    program->setUniformValue("pressed", mouseDown);
    program->setUniformValue("start", start);
    program->setUniformValue("end", end);
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();

    peaked = false;
}


bool OGLWidget::eventFilter(QObject *obj, QEvent *event) {
    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (mouseEvent != nullptr) {
        if (mouseEvent->button() == Qt::LeftButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                mouseDown = true;
                start = -1.0 + 2*((float)mouseEvent->pos().x() / (float)width());
            } else if (event->type() == QEvent::MouseButtonRelease) {
                mouseDown = false;
                end = -1.0 + 2*((float)mouseEvent->pos().x() / (float)width());
            }
            qDebug() << start << end;
        }
        if (mouseDown) {
            end = -1.0 + 2*((float)mouseEvent->pos().x() / (float)width());
        }
        //current = mouseEvent->pos().x();
    }
}


void OGLWidget::setFrequencies(const std::vector<float> &newFrequencies, bool peak)
{
    peaked = peak;
    frequencies = newFrequencies;
    update();
}

void OGLWidget::resizeGL(int w, int h)
{
    //glViewport(0, 0, w, h);
}
