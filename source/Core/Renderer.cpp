#include "Core/Renderer.h"
#include <GL/gl3w.h>
#include <stb_image.h>
#include <iostream>
#include "Core/Context.h"
#include "Core/Renderer.Constants.h"
#include "Renderer.h"

unsigned int CreateShader(const char *v, const char *f);

void Renderer::Initialize(void)
{
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GenerateTexture(mPaletteTex);
    CreatePaletteTexture();

    GenerateTexture(mTilesetTex);
    CreateTexture(128, 512, mTilesetTex);

    GenerateTexture(mUnderlayTex);
    LoadEmptyUnderlay();

    InitializePicker();
    InitializeMap();
}

void Renderer::InitializePicker(void)
{
    GenerateRenderTarget(mPickerTex);
    SpecifyRenderTargetSize(mPickerTex, 128, 512);

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
    DeleteTexture(mTilesetTex);

    DeleteRenderTarget(mPickerTex);
    DeleteRenderTarget(mMapTex);

    glDeleteVertexArrays(1, &mVAO);
}

static unsigned char *LoadTilesetTexture(const std::string &fname)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(fname.c_str(), &width, &height, &channels, 4);

    if (width != 128 || height != 256 || data == nullptr || channels != 1)
    {
        if (data) stbi_image_free(data);
        return NULL;
    }

    return data;
}

bool Renderer::LoadPrimaryTileset(const std::string &fname)
{
    unsigned char *data = LoadTilesetTexture(fname);

    if (!data)
        return false;

    glBindTexture(GL_TEXTURE_2D, mTilesetTex.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 256, 128, 256, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    mRedrawFlag = true;

    return true;
}

bool Renderer::LoadSecondaryTileset(const std::string &fname)
{
    unsigned char *data = LoadTilesetTexture(fname);

    if (!data)
        return false;

    glBindTexture(GL_TEXTURE_2D, mTilesetTex.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 256, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    mRedrawFlag = true;

    return true;
}

bool Renderer::LoadUnderlay(const std::string &fname)
{
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *data = stbi_load(fname.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!data)
        return false;

    // if (width != mMapTex.tex.width || height != mMapTex.tex.height)
    // {
    //     stbi_image_free(data);
    //     return false;
    // }

    CreateUnderlayTexture(width, height, data);
    stbi_image_free(data);

    return true;
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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTilesetTex.id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);

    glBindFramebuffer(GL_FRAMEBUFFER, mPickerTex.fbo);
    glViewport(0, 0, mPickerTex.tex.width, mPickerTex.tex.height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FlushTilemap(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMapEBO);

    glBindBuffer(GL_ARRAY_BUFFER, mMapVBO);
    glBufferData(GL_ARRAY_BUFFER, mMapQuadCount * 4 * sizeof(MapVertex), mMapVertices, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MapVertex), (void *)offsetof(MapVertex, palette));
    glEnableVertexAttribArray(2);

    glUseProgram(mMapShader);

    glUniform1i(glGetUniformLocation(mMapShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(mMapShader, "texture2"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTilesetTex.id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);

    glDrawElements(GL_TRIANGLES, mMapQuadCount * 6, GL_UNSIGNED_INT, 0);
    mMapQuadCount = 0;
}

void Renderer::DrawTilemap(const Context &ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mMapTex.fbo);
    glViewport(0, 0, mMapTex.tex.width, mMapTex.tex.height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    BatchBackground(ctx.GetDefaultTile());
    for (const auto &[pos, tile] : ctx.GetTiles())
        BatchTile(pos.x, pos.y, tile);

    FlushTilemap();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Draw(const Context &ctx)
{
    if (!mRedrawFlag) 
        return;

    mMapQuadCount = 0;
    glBindVertexArray(mVAO);
    DrawTileset();
    DrawTilemap(ctx);
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

void Renderer::CreateTexture(unsigned int width, unsigned int height, Texture &texture)
{
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::CreatePaletteTexture(void)
{
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    for (int i = 0; i < 16; ++i)
        LoadPalette(sDefaultPalette, i);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::CreateUnderlayTexture(unsigned int width, unsigned int height, const unsigned char *data)
{
    glBindTexture(GL_TEXTURE_2D, mUnderlayTex.id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    mUnderlayTex.width = width;
    mUnderlayTex.height = height;
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
    target.tex.width = width;
    target.tex.height = height;

    glBindTexture(GL_TEXTURE_2D, target.tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::ResizeMapTexture(int width, int height)
{
    SpecifyRenderTargetSize(mMapTex, width * 8, height * 8);
    mRedrawFlag = true;
}

void Renderer::LoadEmptyUnderlay(void)
{
    CreateUnderlayTexture(1, 1, transparentUnderlayColors);
}

static const ImVec2 sTransformVectors[4] =
{
    ImVec2(0.0f, 0.0f),
    ImVec2(1.0f, 0.0f),
    ImVec2(1.0f, 1.0f),
    ImVec2(0.0f, 1.0f),
};

template<class T>
static inline void swap_val(T *v1, T *v2)
{
    T temp = *v1;
    *v1 = *v2;
    *v2 = temp;
}

void Renderer::BatchTile(unsigned short x, unsigned short y, const Tile &tile)
{
    if (mMapQuadCount >= MaxQuads)
        FlushTilemap();

    auto mapSize = ImVec2(mMapTex.tex.width, mMapTex.tex.height);
    ImVec2 texCoords[4];
    {
        auto tileDim = ImVec2(128.0f, 512.0f);

        unsigned int x = tile.id % 16;
        unsigned int y = tile.id / 16;

        texCoords[0] = ImVec2(x, y); // Top-left
        texCoords[1] = ImVec2(x + 1, y); // Top-right
        texCoords[2] = ImVec2(x + 1, y + 1); // Bottom-right
        texCoords[3] = ImVec2(x, y + 1); // Bottom-left

        if (tile.xflip)
        {
            swap_val(&texCoords[0].x, &texCoords[1].x);
            swap_val(&texCoords[2].x, &texCoords[3].x);
        }

        if (tile.yflip)
        {
            swap_val(&texCoords[1].y, &texCoords[2].y);
            swap_val(&texCoords[0].y, &texCoords[3].y);
        }

        texCoords[0] /= tileDim / 8.0f;
        texCoords[1] /= tileDim / 8.0f;
        texCoords[2] /= tileDim / 8.0f;
        texCoords[3] /= tileDim / 8.0f;

        // invert y axis
        texCoords[0].y = 1.0 - texCoords[0].y;
        texCoords[1].y = 1.0 - texCoords[1].y;
        texCoords[2].y = 1.0 - texCoords[2].y;
        texCoords[3].y = 1.0 - texCoords[3].y;
    }

    for (int i = 0; i < 4; ++i)
    {
        mMapVertices[i + mMapQuadCount * 4].pos = ((ImVec2(x, y) + sTransformVectors[i]) * 8.0f) /  mapSize;
        mMapVertices[i + mMapQuadCount * 4].uv = texCoords[i];
        mMapVertices[i + mMapQuadCount * 4].palette = tile.palette;
    }
    mMapQuadCount++;
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