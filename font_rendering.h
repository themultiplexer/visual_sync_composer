#include "qopenglcontext.h"
#include "qopenglextrafunctions.h"
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <freetype/freetype.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;
unsigned int VAO, VBO;
bool firstword = true;

int LoadFontRendering(std::string font_path, QOpenGLContext *context) {
    Characters.clear();
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    FT_Set_Pixel_Sizes(face, 0, 256);
    FT_GlyphSlot slot = face->glyph;

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        FT_Error fterr = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (fterr)
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << fterr << std::endl;
            continue;
        }

        FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);

        // generate texture
        unsigned int font_texture;
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            font_texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // configure VAO/VBO for texture quads
    // -----------------------------------
    context->extraFunctions()->glGenVertexArrays(1, &VAO);
    context->extraFunctions()->glGenBuffers(1, &VBO);
    context->extraFunctions()->glBindVertexArray(VAO);
    context->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    context->extraFunctions()->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    context->extraFunctions()->glEnableVertexAttribArray(0);
    context->extraFunctions()->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    context->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
    context->extraFunctions()->glBindVertexArray(0);

    return 0;
}


void RenderText(GLuint shader, std::string text, float x, float y, float scale, glm::vec3 color, QOpenGLContext *context)
{
    // activate corresponding render state	
    context->extraFunctions()->glUseProgram(shader);
    
    glActiveTexture(GL_TEXTURE0);
    context->extraFunctions()->glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    float width = 0.0f;
    glm::vec3 current_color = color;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];
        width += ch.Size.x * scale;
        if (c != text.end()){
            width += (ch.Advance >> 6) * scale;
        }
    }

    x = -width/4.0;
    for (c = text.begin(); c != text.end(); c++)
    {
        if (*c == ' ') {
            current_color = glm::vec3(0.0, 1.0, 1.0);
        }
        context->extraFunctions()->glUniform3fv(context->extraFunctions()->glGetUniformLocation(shader, "textColor"), 1, glm::value_ptr(current_color));

        int ch_index = c - text.begin();
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        context->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, VBO);
        context->extraFunctions()->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        context->extraFunctions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    context->extraFunctions()->glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    context->extraFunctions()->glUseProgram(0);
}
