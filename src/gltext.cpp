#include "gltext.h"
#include "qdebug.h"
#include <cmath>
#include "font_rendering.h"

// Function to check shader compilation errors
void checkShaderCompileErrors(GLuint shader, QOpenGLContext *context)
{
    GLint success;
    GLchar infoLog[512];

    context->extraFunctions()->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        context->extraFunctions()->glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }
}

const char* fontVertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
    out vec2 TexCoords;

    uniform vec2 resolution;

    void main()
    {
        gl_Position = vec4(vertex.xy / vec2(resolution.x / resolution.y, 1.0), 0.0, 1.0);
        TexCoords = vertex.zw;
    }
)";

const char* fontFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoords;

    uniform sampler2D text;
    uniform vec3 textColor;

    const float smoothing = 0.1f;

    void main()
    {
        float distance = texture2D(text, TexCoords).r;
        float alpha1 = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
        gl_FragColor = vec4(textColor.rgb * alpha1, 1.0);
        //gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
    }
)";


GLText::GLText(QWidget *parent)
    : QOpenGLWidget(parent)
{
    text = "Welcome";
}

GLText::~GLText()
{

}

void GLText::textEvent(const char *text){
    setText(text);
    update();
}

void GLText::initializeGL()
{
    setUpdateBehavior(NoPartialUpdate);
    glClearColor(0,0,0,1);

    LoadFontRendering("/home/josh/VisualSyncComposer/fonts/arial.ttf", context());

    // Compile and link shaders
    GLuint vertexShader = context()->extraFunctions()->glCreateShader(GL_VERTEX_SHADER);
    context()->extraFunctions()->glShaderSource(vertexShader, 1, &fontVertexShaderSource, nullptr);
    context()->extraFunctions()->glCompileShader(vertexShader);
    checkShaderCompileErrors(vertexShader, context());

    GLuint fragmentShader = context()->extraFunctions()->glCreateShader(GL_FRAGMENT_SHADER);
    context()->extraFunctions()->glShaderSource(fragmentShader, 1, &fontFragmentShaderSource, nullptr);
    context()->extraFunctions()->glCompileShader(fragmentShader);
    checkShaderCompileErrors(fragmentShader, context());

    shaderProgram = context()->extraFunctions()->glCreateProgram();
    context()->extraFunctions()->glAttachShader(shaderProgram, vertexShader);
    context()->extraFunctions()->glAttachShader(shaderProgram, fragmentShader);
    context()->extraFunctions()->glLinkProgram(shaderProgram);

    context()->extraFunctions()->glDeleteShader(vertexShader);
    context()->extraFunctions()->glDeleteShader(fragmentShader);

    //context()->extraFunctions()->glUniform2f(context()->extraFunctions()->glGetUniformLocation(shaderProgram, "resolution"), w, h);
}

void GLText::paintGL()
{
    RenderText(shaderProgram, text, 0, -0.9, 0.01, glm::vec3(1.0,0.0,1.0), context());
}

void GLText::setText(std::string _text)
{
    text = _text;
    update();
}

void GLText::resizeGL(int w, int h)
{
    context()->extraFunctions()->glUseProgram(shaderProgram);
    context()->extraFunctions()->glUniform2f(context()->extraFunctions()->glGetUniformLocation(shaderProgram, "resolution"), w, h);
    context()->extraFunctions()->glUseProgram(0);
    update();
}
