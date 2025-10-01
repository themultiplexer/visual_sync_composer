#include "tubewidget.h"
#include <cmath>


#define PROGRAM_VERTEX_ATTRIBUTE 0


TubeWidget::TubeWidget(QWidget *parent) : QOpenGLWidget(parent), peak(0.0), time(0.0), sizes(20), brightness(20), particle(0), color(0,0,255), effect({1})
{
    rng = new std::mt19937(dev());
    pixelRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

    timeRef = std::chrono::system_clock::now();

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

    // Create and compile the vertex shader using a raw string literal.
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc = R"(
            #version 330 core
            layout (location = 0) in vec2 vertex;
            out vec2 pos;
            flat out int qindex;

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
            uniform float sizes[20];
            uniform float brightness[20];

            void main() {
                vec2 p = pos;
                int tindex = int(((p.y + 1.0) / 2.0) * 20);
                float r = 0.05 * sizes[tindex];
                p.y = mod(p.y, 0.1);
                p.x = p.x / 20.0;

                p = p - vec2(0.0, 0.05);
                vec3 white = vec3(1.0, 1.0, 1.0);
                float d = length(p) - r;
                float tp = 0.05;
                float f = 0.005;
                vec3 col = smoothstep(tp - f, tp + f, d) *  white;
                FragColor = vec4((color - col) * brightness[tindex], 1.0);
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
    program->bind();
    program->setUniformValue("peak", peak);
    program->setUniformValue("color", QVector3D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0));

    GLfloat v[20] = { 0.0 };
    for (int i = 0; i < 20; ++i) {
        v[i] = 0.05;
    }
    program->setUniformValueArray("sizes", v, 20, 1);


    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timeRef);

    if (effect.led_mode == 0) {
        for (int i = 0; i < 20; ++i) {
            brightness[i] = (i == (int)(sin(diff.count() / 100.0) * 10.0) + 10);
        }
    } else {

    }

    if (true || effect.modifiers & 0x01) {
        for (int i = 0; i < 20; ++i) {
            brightness[i] = brightness[i] > 0.0 ? brightness[i] - 0.1 : 0.0;
        }
    }
    program->setUniformValueArray("brightness", brightness.constData(), 20, 1);

    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.release();
    program->release();
    time += 0.05;
}

void TubeWidget::setEffect(CONFIG_DATA effect)
{
    this->effect = effect;
}

void TubeWidget::sync() {
    timeRef = std::chrono::system_clock::now();
}

void TubeWidget::setPeaked(QColor newColor) {
    peaked = true;
    for (int i = 0; i < 20; ++i) {
        brightness[i] = ((*pixelRandom)(*rng)) / 255.0;
    }
    color = newColor.toRgb();
}

