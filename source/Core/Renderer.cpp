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

    InitializePicker();
    InitializeLightMap();
    InitializeMetatiles();
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

void Renderer::InitializeLightMap(void)
{
    GenerateRenderTarget(mLightMapTex);

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

    glGenBuffers(1, &mLightMapVBO);
    glGenBuffers(1, &mLightMapEBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLightMapEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MaxIndices, quadIndices, GL_STATIC_DRAW);

    mLightMapShader = CreateShader(mapVertexShaderSource, mapFragmentShaderSource);
}

void Renderer::InitializeMetatiles(void)
{
    GenerateRenderTarget(mMetatileTex);
    // SpecifyRenderTargetSize(mMetatileTex, 16384, 16);
}

void Renderer::Shutdown(void)
{
    glDeleteBuffers(1, &mPickerVBO);
    glDeleteBuffers(1, &mPickerEBO);

    DeleteTexture(mPaletteTex);
    DeleteTexture(mTilesetTex);

    DeleteRenderTarget(mPickerTex);
    DeleteRenderTarget(mLightMapTex);
    DeleteRenderTarget(mMetatileTex);

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLightMapEBO);

    glBindBuffer(GL_ARRAY_BUFFER, mLightMapVBO);
    glBufferData(GL_ARRAY_BUFFER, mLightMapQuadCount * 4 * sizeof(LightMapVertex), mLightMapVertices, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LightMapVertex), (void *)offsetof(LightMapVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(LightMapVertex), (void *)offsetof(LightMapVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(LightMapVertex), (void *)offsetof(LightMapVertex, palette));
    glEnableVertexAttribArray(2);

    glUseProgram(mLightMapShader);

    glUniform1i(glGetUniformLocation(mLightMapShader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(mLightMapShader, "texture2"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTilesetTex.id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mPaletteTex.id);

    glDrawElements(GL_TRIANGLES, mLightMapQuadCount * 6, GL_UNSIGNED_INT, 0);
    mLightMapQuadCount = 0;
}

void Renderer::DrawTilemap(const Context &ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mLightMapTex.fbo);
    glViewport(0, 0, mLightMapTex.tex.width, mLightMapTex.tex.height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    BatchBackground(ctx.GetDefaultTile());
    for (const auto &[pos, tile] : ctx.GetTiles())
        BatchTile(pos.x, pos.y, tile, mLightMapTex.tex.width, mLightMapTex.tex.height);

    FlushTilemap();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMetatiles(const Context &ctx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mMetatileTex.fbo);
    glViewport(0, 0, mMetatileTex.tex.width, mMetatileTex.tex.height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto drawMetatile = [this, ctx](unsigned int i, unsigned int x, unsigned int y) {
        for (int j = 0; j < 3; ++j)
        for (int k = 0; k < 4; ++k)
        {
            int offsetX = (k % 2), offsetY = k / 2;

            const Tile &tile = (i < 2048) ? 
                ctx.GetPrimaryMetatiles()[k + j * 4 + i * 12] :
                ctx.GetSecondaryMetatiles()[k + j * 4 + (i - 2048) * 12];

            BatchTile(x + offsetX, y + offsetY, tile, mMetatileTex.tex.width, mMetatileTex.tex.height);
        }
    };

    const auto &blockData = ctx.GetBlockData();

    for (int y = 0; y < ctx.GetWidth(); ++y)
    for (int x = 0; x < ctx.GetHeight(); ++x)
    {
        drawMetatile(blockData[y * ctx.GetWidth() + x] & 0b111111111111, x * 2, y * 2);
    }

    FlushTilemap();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Draw(const Context &ctx)
{
    if (!mRedrawFlag) 
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    mLightMapQuadCount = 0;
    glBindVertexArray(mVAO);
    DrawTileset();
    DrawTilemap(ctx);
    DrawMetatiles(ctx);
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
    SpecifyRenderTargetSize(mLightMapTex, width * 16, height * 16);
    SpecifyRenderTargetSize(mMetatileTex, width * 16, height * 16);

    mRedrawFlag = true;
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

void Renderer::BatchTile(unsigned short x, unsigned short y, const Tile &tile, int width, int height)
{
    if (mLightMapQuadCount >= MaxQuads)
        FlushTilemap();

    auto mapSize = ImVec2(width, height);
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
        mLightMapVertices[i + mLightMapQuadCount * 4].pos = ((ImVec2(x, y) + sTransformVectors[i]) * 8.0f) /  mapSize;
        mLightMapVertices[i + mLightMapQuadCount * 4].uv = texCoords[i];
        mLightMapVertices[i + mLightMapQuadCount * 4].palette = tile.palette;
    }
    mLightMapQuadCount++;
}

void Renderer::BatchBackground(const Tile &tile)
{
    int xtiles = mLightMapTex.tex.width / 8, 
        ytiles = mLightMapTex.tex.height / 8;

    for (int y = 0; y < ytiles; ++y)
    for (int x = 0; x < xtiles; ++x)
    {
        BatchTile(x, y, tile, mLightMapTex.tex.width, mLightMapTex.tex.height);
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

