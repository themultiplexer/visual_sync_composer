#include "oglwidget.h"
#include "qdebug.h"
#include <cmath>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    out vec2 p;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        p = aPos;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 p;
    void main() {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
    }
)";


OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    for (int i = 0; i < NUM_POINTS; ++i) {
            frequencies.push_back(0.0);
    }
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
    context()->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    context()->extraFunctions()->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
}

void OGLWidget::initializeGL()
{
    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glLineWidth(5.0);
    glPointSize(10.0);

    context()->extraFunctions()->glGenVertexArrays(1, &vertexArray);
    context()->extraFunctions()->glBindVertexArray(vertexArray);

    context()->extraFunctions()->glGenBuffers(1, &vertexBuffer);
    OGLWidget::createVBO();
    context()->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    context()->extraFunctions()->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    context()->extraFunctions()->glEnableVertexAttribArray(0);
    context()->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
    context()->extraFunctions()->glBindVertexArray(0);


    // Compile and link shaders
    GLuint vertexShader = context()->extraFunctions()->glCreateShader(GL_VERTEX_SHADER);
    context()->extraFunctions()->glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    context()->extraFunctions()->glCompileShader(vertexShader);

    GLuint fragmentShader = context()->extraFunctions()->glCreateShader(GL_FRAGMENT_SHADER);
    context()->extraFunctions()->glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    context()->extraFunctions()->glCompileShader(fragmentShader);

    shaderProgram = context()->extraFunctions()->glCreateProgram();
    //context()->extraFunctions()->glAttachShader(shaderProgram, vertexShader);
    //context()->extraFunctions()->glAttachShader(shaderProgram, fragmentShader);
    context()->extraFunctions()->glLinkProgram(shaderProgram);

    context()->extraFunctions()->glDeleteShader(vertexShader);
    context()->extraFunctions()->glDeleteShader(fragmentShader);

}

void OGLWidget::paintGL()
{
    if (peaked) {
        glClearColor(1,0,0,1);
    } else {
        glClearColor(0,0,0,1);
    }

    OGLWidget::createVBO();
    context()->extraFunctions()->glUseProgram(shaderProgram);
    context()->extraFunctions()->glBindVertexArray(vertexArray);
    glDrawArrays(GL_LINE_STRIP, 0, NUM_POINTS);
    peaked = false;
}

void OGLWidget::setFrequencies(const std::vector<float> &newFrequencies, bool peak)
{
    peaked = peak;
    frequencies = newFrequencies;
    update();
}

void OGLWidget::resizeGL(int w, int h)
{

}
