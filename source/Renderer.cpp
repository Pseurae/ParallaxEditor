#include "Renderer.h"
#include "Global.h"
#include <GL/gl3w.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <iostream>

#define MAX_QUAD 32 * 32

struct MapVertex
{
    ImVec2 pos;
    ImVec2 uv;
    float palette;
};

static MapVertex sMapVertices[MAX_QUAD * 4];

static constexpr float tilesetVertices[] =
{ 
    // positions  // texture coords
    +1.0f, +1.0f, 1.0f, 0.0f, // top right
    +1.0f, -1.0f, 1.0f, 1.0f, // bottom right
    -1.0f, -1.0f, 0.0f, 1.0f, // bottom left
    -1.0f, +1.0f, 0.0f, 0.0f, // top left
};

static constexpr unsigned int indices[] =
{ 
    0, 1, 3,
    1, 2, 3,
};

static constexpr auto *tilesetVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 0.0, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";

static constexpr auto *tilesetFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    float x = texture(texture1, TexCoord).r / 16.0f * 256.0f;
	FragColor = texture(texture2, vec2(x, 0.5f));
}
)";

static constexpr auto *mapVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aPalette;

out vec2 TexCoord;
flat out float Palette;

void main()
{
    gl_Position = vec4(aPos * 2.0f - 1.0f, 0.0, 1.0);
    TexCoord = aTexCoord;
    Palette = aPalette;
}
)";

static constexpr auto *mapFragmentShaderSource = R"(
#version 330 core

uniform sampler2D texture1;
uniform sampler2D texture2;

in vec2 TexCoord;
flat in float Palette;
out vec4 FragColor;

void main()
{
    float x = texture(texture1, TexCoord).r + (16.0f / 256.0f) * Palette;
	FragColor = texture(texture2, vec2(x, 0.5f));
}
)";

static unsigned int create_shader(const char *v, const char *f)
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
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
    }

    unsigned int shaderProgram = glCreateProgram();
    if (v) glAttachShader(shaderProgram, vertexShader);
    if (f) glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

static void renderer_tileset_init(Renderer &r)
{
    glGenBuffers(1, &r.pickerVertexBuffer);
    glGenBuffers(1, &r.pickerElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, r.pickerVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tilesetVertices), tilesetVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.pickerElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate Tileset
    glGenTextures(1, &r.pickerTilesetTex);
    glBindTexture(GL_TEXTURE_2D, r.pickerTilesetTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // Generate 16-color Palette
    glGenTextures(1, &r.pickerPaletteTex);
    glBindTexture(GL_TEXTURE_2D, r.pickerPaletteTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    r.pickerShader = create_shader(tilesetVertexShaderSource, tilesetFragmentShaderSource);

    glUseProgram(r.pickerShader);
    glUniform1i(glGetUniformLocation(r.pickerShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(r.pickerShader, "texture2"), 1);

    glGenFramebuffers(1, &r.pickerFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, r.pickerFrameBuffer);

    glGenTextures(1, &r.pickerFinalTex);
    glBindTexture(GL_TEXTURE_2D, r.pickerFinalTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.pickerFinalTex, 0);
}

static void renderer_map_init(Renderer &r)
{
    glGenBuffers(1, &r.mapVertexBuffer);
    glGenBuffers(1, &r.mapElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, r.mapVertexBuffer);

    unsigned int quadIndices[MAX_QUAD * 6];
    {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < (MAX_QUAD * 6); i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.mapElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MAX_QUAD * 6, quadIndices, GL_STATIC_DRAW);

    glGenFramebuffers(1, &r.mapFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, r.mapFrameBuffer);

    glGenTextures(1, &r.mapFinalTex);
    glBindTexture(GL_TEXTURE_2D, r.mapFinalTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.mapFinalTex, 0);

    // Generate 256-color Palette
    glGenTextures(1, &r.mapPaletteTex);
    glBindTexture(GL_TEXTURE_2D, r.mapPaletteTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    r.mapShader = create_shader(mapVertexShaderSource, mapFragmentShaderSource);

    glUseProgram(r.mapShader);
    glUniform1i(glGetUniformLocation(r.mapShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(r.mapShader, "texture2"), 1);
}

bool renderer_init(Renderer &r)
{
    glGenVertexArrays(1, &r.vertexArray);
    glBindVertexArray(r.vertexArray);

    renderer_tileset_init(r);
    renderer_map_init(r);

    return true;
}

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

static void renderer_load_map_palette(Renderer &r)
{
    glBindTexture(GL_TEXTURE_2D, r.mapPaletteTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_UNSIGNED_BYTE, r.palettes);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void renderer_call_tileset(Renderer &r)
{
    glUseProgram(r.pickerShader);

    glUniform1i(glGetUniformLocation(r.pickerShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(r.pickerShader, "texture2"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.pickerTilesetTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r.pickerPaletteTex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.pickerElementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, r.pickerVertexBuffer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindFramebuffer(GL_FRAMEBUFFER, r.pickerFrameBuffer);
    glViewport(0, 0, 128, 512);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

enum Mask : short
{
    Index = 0x3FF,
    FlipX = 0x400,
    FlipY = 0x800
};

static constexpr auto sTileWidth = 8.0f / 256.0f;

static const ImVec2 sTransformVectors[4] =
{
    ImVec2(0.0f, 0.0f) * sTileWidth,
    ImVec2(1.0f, 0.0f) * sTileWidth,
    ImVec2(1.0f, 1.0f) * sTileWidth,
    ImVec2(0.0f, 1.0f) * sTileWidth,
};

template<class T>
static inline void swap_val(T *v1, T *v2)
{
    T temp = *v1;
    *v1 = *v2;
    *v2 = temp;
}

static void renderer_draw_map_tile(Renderer &r, int i, unsigned short entry)
{
    unsigned int x = i % 32;
    unsigned int y = i / 32;

    unsigned int t = entry & Mask::Index;

    ImVec2 texCoords[4];
    {
        float tileW = 8.0f / 128.0f;
        float tileH = 8.0f / 512.0f;

        auto tileDim = ImVec2(tileW, tileH);

        unsigned int x = t % 16;
        unsigned int y = t / 16;

        texCoords[0] = ImVec2(x, y); // Top-left
        texCoords[1] = ImVec2(x + 1, y); // Top-right
        texCoords[2] = ImVec2(x + 1, y + 1); // Bottom-right
        texCoords[3] = ImVec2(x, y + 1); // Bottom-left

        if ((entry & Mask::FlipX) == Mask::FlipX)
        {
            swap_val(&texCoords[0].x, &texCoords[1].x);
            swap_val(&texCoords[2].x, &texCoords[3].x);
        }

        if ((entry & Mask::FlipY) == Mask::FlipY)
        {
            swap_val(&texCoords[1].y, &texCoords[2].y);
            swap_val(&texCoords[0].y, &texCoords[3].y);
        }

        texCoords[0] *= tileDim;
        texCoords[1] *= tileDim;
        texCoords[2] *= tileDim;
        texCoords[3] *= tileDim;

        // invert y axis
        texCoords[0].y = 1.0 - texCoords[0].y;
        texCoords[1].y = 1.0 - texCoords[1].y;
        texCoords[2].y = 1.0 - texCoords[2].y;
        texCoords[3].y = 1.0 - texCoords[3].y;
    }

    for (unsigned j = 0; j < 4; ++j)
    {
        sMapVertices[j + i * 4].pos = ImVec2(x, y) * sTileWidth + sTransformVectors[j];
        sMapVertices[j + i * 4].uv = texCoords[j];
        sMapVertices[j + i * 4].palette = (entry >> 12) & 0xF;
    }
}

static void renderer_call_map(Renderer &r)
{
    renderer_load_map_palette(r);

    for (int i = 0; i < MAX_QUAD; ++i)
        renderer_draw_map_tile(r, i, global.tilemap[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.pickerTilesetTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r.mapPaletteTex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.mapElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, r.mapVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sMapVertices), sMapVertices, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, palette));
    glEnableVertexAttribArray(2);

    glUseProgram(r.mapShader);

    glUniform1i(glGetUniformLocation(r.mapShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(r.mapShader, "texture2"), 1);

    glBindFramebuffer(GL_FRAMEBUFFER, r.mapFrameBuffer);
    glViewport(0, 0, 256, 256);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, MAX_QUAD * 6, GL_UNSIGNED_INT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer_call(Renderer &r)
{
    glBindVertexArray(r.vertexArray);
    renderer_call_tileset(r);
    renderer_call_map(r);
}
