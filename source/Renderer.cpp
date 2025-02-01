#include "Renderer.h"
#include "Global.h"
#include <GL/gl3w.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <iostream>

#include "Renderer.Tileset.h"
#include "Renderer.Tilemap.h"

unsigned int create_shader(const char *v, const char *f)
{
    unsigned int vertexShader, fragmentShader, geometryShader;
    int success;
    char infoLog[512];

    if (v)
    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &v, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
    }

    if (f)
    {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &f, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
    }

    unsigned int shaderProgram = glCreateProgram();
    if (v) glAttachShader(shaderProgram, vertexShader);
    if (f) glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

bool renderer_init(Renderer &r)
{
    glGenVertexArrays(1, &r.vertexArray);
    glBindVertexArray(r.vertexArray);

    renderer_tileset_init(r);
    renderer_map_init(r);

    return true;
}

void renderer_call(Renderer &r, unsigned short tilemap[])
{
    glBindVertexArray(r.vertexArray);
    renderer_call_tileset(r);
    renderer_call_map(r, tilemap);
}

// Start of helpers
void renderer_load_primary(Renderer &r, unsigned char *data)
{
    glBindTexture(GL_TEXTURE_2D, r.pickerTilesetTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 256, 128, 256, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_load_secondary(Renderer &r, unsigned char *data)
{
    glBindTexture(GL_TEXTURE_2D, r.pickerTilesetTex);
    // Account for the texture flip.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 256, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_load_palette(Renderer &r, int idx, const Palette plt)
{
    memcpy(r.palettes[idx], plt, sizeof(r.palettes[idx]));
}

void renderer_change_palette(Renderer &r, int idx)
{
    glBindTexture(GL_TEXTURE_2D, r.pickerPaletteTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 1, GL_RGB, GL_UNSIGNED_BYTE, r.palettes[idx]);
    glBindTexture(GL_TEXTURE_2D, 0);
}