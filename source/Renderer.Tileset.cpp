#include "Renderer.Tileset.h"
#include "Renderer.h"
#include <imgui.h>
#include <GL/gl3w.h>

unsigned int create_shader(const char *v, const char *f);

static constexpr float tilesetVertices[] =
{ 
    // positions  // texture coords
    +1.0f, +1.0f, 1.0f, 0.0f, // top right
    +1.0f, -1.0f, 1.0f, 1.0f, // bottom right
    -1.0f, -1.0f, 0.0f, 1.0f, // bottom left
    -1.0f, +1.0f, 0.0f, 0.0f, // top left
};

static constexpr unsigned int tilesetIndices[] =
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
    float x = texture(texture1, TexCoord).r;
    vec4 color = texture(texture2, vec2(x / 16.0f * 256.0f, 0.5f));
	FragColor = mix(vec4(x * 16.0f), color, color.a);
}
)";

void renderer_tileset_init(Renderer &r)
{
    glGenBuffers(1, &r.pickerVertexBuffer);
    glGenBuffers(1, &r.pickerElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, r.pickerVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tilesetVertices), tilesetVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.pickerElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tilesetIndices), tilesetIndices, GL_STATIC_DRAW);

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

void renderer_call_tileset(Renderer &r)
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