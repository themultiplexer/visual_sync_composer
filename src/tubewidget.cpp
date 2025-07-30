#include "tubewidget.h"
#include <cmath>


#define PROGRAM_VERTEX_ATTRIBUTE 0


TubeWidget::TubeWidget(QWidget *parent)
    : QOpenGLWidget(parent), peak(0.0), time(0.0)
{
    rng = new std::mt19937(dev());
    pixelRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 144);

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
            layout (location = 1) in float index;
            out vec2 pos;
            flat out int qindex;

            void main() {
                pos = vertex;
                qindex = int(index);
                gl_Position = vec4(vertex, 0.0, 1.0);
            }
        )";
    vshader->compileSourceCode(vsrc);

    // Create and compile the fragment shader.
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc = R"(
            #version 330 core
            flat in int qindex;
            in vec2 pos;
            out vec4 FragColor;
            uniform float peak;
            uniform vec3 color;
            uniform float sizes[144];

            void main() {
                //float y = smoothstep(-0.99, -0.2, pos.x) - smoothstep(0.2, 0.99, pos.x);
                //vec3 c = color * y;
                //FragColor = vec4(c * float(pos.y < (-1.0 + 2 * peak)), 1.0);

                vec2 center = vec2(0.5);
                vec2 rel = (gl_PointCoord - center); // or calculate manually if needed
                float dist = length(rel);

                float radius = sizes[qindex];
                float mask = smoothstep(radius, radius - 0.01, dist);

                vec3 color = vec3(1.0 - mask);
                FragColor = vec4(color, 1.0);
            }
        )";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->link();


    const int quads = 144;
    const float quadWidth = 1.0f;
    const float quadHeight = 0.015f;

    struct Vertex {
        float x, y;   // position
        float quadIndex; // integer index of quad (0 to 143)
    };

    QVector<Vertex> vertices;
    vertices.reserve(quads * 6 * 4); // 6 vertices * (x,y) * quads

    for (int i = 0; i < quads; ++i) {
        float y = i * quadHeight - 1.0;

        float x0 = -quadWidth / 2;
        float x1 =  quadWidth / 2;
        float y0 = y;
        float y1 = y + quadHeight;

        float qi = static_cast<float>(i);

        vertices << Vertex{x0, y0, qi};
        vertices << Vertex{x1, y0, qi};
        vertices << Vertex{x1, y1, qi};

        vertices << Vertex{x0, y0, qi};
        vertices << Vertex{x1, y1, qi};
        vertices << Vertex{x0, y1, qi};
    }

    vao.create();
    vao.bind();
    vertexPositionBuffer.create();
    vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexPositionBuffer.bind();
    vertexPositionBuffer.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    QVector<float> sizes(144);
    // Fill with values between 0.0 (tiny) to 0.5 (full radius)
    for (int i = 0; i < 144; ++i)
        sizes[i] = 0.4f + 0.1f * sin(i * 0.1f);


    //program->bind();
    //program->setUniformValueArray("sizes", sizes.constData(), sizes.size(), 1);

    program->bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, x), 2, sizeof(Vertex));
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, quadIndex), 1, sizeof(Vertex));
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

    GLfloat v[144] = { 0.01 };
    for (int i = 0; i < 144; ++i) {
        v[i] = (*pixelRandom)(*rng);
    }
    program->setUniformValueArray("sizes", v, 144, 1);
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 144 * 6);
    vao.release();
    program->release();
    time += 0.05;
}

void TubeWidget::setPeaked(QColor newColor) {
    peaked = true;
    //qDebug() << newColor;
    color = newColor.toRgb();
}

