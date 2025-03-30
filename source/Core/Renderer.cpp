#include "Core/Renderer.h"
#include <GL/gl3w.h>
#include <stb_image.h>
#include <iostream>
#include "Core/Context.h"
#include "Core/Renderer.Constants.h"

unsigned int CreateShader(const char *v, const char *f);

void Renderer::Initialize(void)
{
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GenerateTexture(mPaletteTex);
    CreatePaletteTexture();

    GenerateTexture(mTilesetTexes[0]);
    GenerateTexture(mTilesetTexes[1]);

    InitializePicker();
    InitializeMap();
}

void Renderer::InitializePicker(void)
{
    GenerateRenderTarget(mPickerTexes[0]);
    GenerateRenderTarget(mPickerTexes[1]);

    glGenBuffers(1, &mPickerVBO);
    glGenBuffers(1, &mPickerEBO);

    glBindBuffer(GL_ARRAY_BUFFER, mPickerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, tilesetVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mPickerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, tilesetIndices, GL_STATIC_DRAW);

    mPickerShader = CreateShader(tilesetVertexShaderSource, tilesetFragmentShaderSource);
}

void Renderer::InitializeMap(void)
{
    GenerateRenderTarget(mMapTex);

    unsigned int quadIndices[MaxIndices];
    {
        unsigned int offset = 0;
        for (unsigned int i = 0; i < MaxIndices; i += 6)
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

    glGenBuffers(1, &mMapVBO);
    glGenBuffers(1, &mMapEBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMapEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MaxIndices, quadIndices, GL_STATIC_DRAW);

    mMapShader = CreateShader(mapVertexShaderSource, mapFragmentShaderSource);
}

void Renderer::Shutdown(void)
{
    glDeleteBuffers(1, &mPickerVBO);
    glDeleteBuffers(1, &mPickerEBO);

    DeleteTexture(mPaletteTex);
    DeleteTexture(mTilesetTexes[0]);
    DeleteTexture(mTilesetTexes[1]);

    DeleteRenderTarget(mPickerTexes[0]);
    DeleteRenderTarget(mPickerTexes[1]);
    DeleteRenderTarget(mMapTex);

    glDeleteVertexArrays(1, &mVAO);
}

void Renderer::LoadPrimaryTileset(const std::string &fname)
{
    LoadTexture(fname, mTilesetTexes[0]);
    SpecifyRenderTargetSize(mPickerTexes[0], mTilesetTexes[0].width, mTilesetTexes[0].height);
    mRedrawFlag = true;
}

void Renderer::LoadSecondaryTileset(const std::string &fname)
{
    LoadTexture(fname, mTilesetTexes[1]);
    SpecifyRenderTargetSize(mPickerTexes[1], mTilesetTexes[1].width, mTilesetTexes[1].height);
    mRedrawFlag = true;
}

void Renderer::DrawTileset(void)
{
    glUseProgram(mPickerShader);
    glUniform1i(glGetUniformLocation(mPickerShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(mPickerShader, "texture2"), 1);
    glUniform1f(glGetUniformLocation(mPickerShader, "paletteNum"), (float)mPickerPalNum);

    glBindBuffer(GL_ARRAY_BUFFER, mPickerVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mPickerEBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    for (int i = 0; i < 2; ++i)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTilesetTexes[i].id);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);

        glBindFramebuffer(GL_FRAMEBUFFER, mPickerTexes[i].fbo);
        glViewport(0, 0, mPickerTexes[i].tex.width, mPickerTexes[i].tex.height);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawTilemap(const Context &ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mMapTex.fbo);

    BatchBackground(ctx.GetDefaultTile());
    for (const auto &[pos, tile] : ctx.GetTiles())
        BatchTile(pos.x, pos.y, tile);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Draw(const Context &ctx)
{
    if (!mRedrawFlag) 
        return;

    glBindVertexArray(mVAO);
    DrawTileset();
    DrawTilemap(ctx);
    FlushRender();
    mRedrawFlag = false;
}

void Renderer::LoadPalette(const Palette &palette, int slot)
{
    LoadPalette(palette.GetColors().data(), slot);
}

void Renderer::ClearPalette(int slot)
{
    LoadPalette(sDefaultPalette, slot);
}

void Renderer::LoadPalette(const void *data, int slot)
{
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, slot * 16, 0, 16, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::LoadTexture(const std::string &fname, Texture &texture)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &width, &height, &channels, 4);

    if ((width % 8) != 0 || (height % 8) != 0 || data == nullptr)
        return;

    texture.width = width;
    texture.height = height;
    
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::CreatePaletteTexture(void)
{
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    for (int i = 0; i < 16; ++i)
        LoadPalette(sDefaultPalette, i);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::GenerateTexture(Texture &tex)
{
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::GenerateRenderTarget(RenderTarget &target)
{
    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

    glGenTextures(1, &target.tex.id);
    GenerateTexture(target.tex);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.tex.id, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DeleteTexture(const Texture &tex)
{
    glDeleteTextures(1, &tex.id);
}

void Renderer::DeleteRenderTarget(const RenderTarget &target)
{
    glDeleteFramebuffers(1, &target.fbo);
    glDeleteTextures(1, &target.tex.id);
}

void Renderer::SpecifyRenderTargetSize(RenderTarget &target, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, target.tex.id);

    target.tex.width = width;
    target.tex.height = height;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::ResizeMapTexture(int width, int height)
{
    SpecifyRenderTargetSize(mMapTex, width * 8, height * 8);
    mRedrawFlag = true;
}

void Renderer::BatchTile(unsigned short x, unsigned short y, const Tile &tile)
{
}

void Renderer::BatchBackground(const Tile &tile)
{
    int xtiles = mMapTex.tex.width / 8, 
        ytiles = mMapTex.tex.height / 8;

    for (int y = 0; y < ytiles; ++y)
    for (int x = 0; x < xtiles; ++x)
    {
        BatchTile(x, y, tile);
    }
}

void Renderer::FlushRender(void)
{}

unsigned int CreateShader(const char *v, const char *f)
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