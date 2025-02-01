#include "Renderer.Tilemap.h"
#include "Renderer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <GL/gl3w.h>

unsigned int create_shader(const char *v, const char *f);

#define MAX_QUAD 32 * 32

struct MapVertex
{
    ImVec2 pos;
    ImVec2 uv;
    float palette;
};

static MapVertex sMapVertices[MAX_QUAD * 4];

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


void renderer_map_init(Renderer &r)
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

static void renderer_load_map_palette(Renderer &r)
{
    glBindTexture(GL_TEXTURE_2D, r.mapPaletteTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_UNSIGNED_BYTE, r.palettes);
    glBindTexture(GL_TEXTURE_2D, 0);
}

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

void renderer_call_map(Renderer &r, unsigned short tilemap[])
{
    renderer_load_map_palette(r);

    for (int i = 0; i < MAX_QUAD; ++i)
        renderer_draw_map_tile(r, i, tilemap[i]);

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
