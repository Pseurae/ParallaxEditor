#include "Core/Renderer.Constants.h"

const float tilesetVertices[] =
    {
        // positions  // texture coords
        +1.0f, +1.0f, 1.0f, 0.0f, // top right
        +1.0f, -1.0f, 1.0f, 1.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 1.0f, // bottom left
        -1.0f, +1.0f, 0.0f, 0.0f, // top left
};

const unsigned int tilesetIndices[] =
{
        0,
        1,
        3,
        1,
        2,
        3,
};

const char tilesetVertexShaderSource[] = R"(
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

const char tilesetFragmentShaderSource[] = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float paletteNum;

void main()
{
    float x = texture(texture1, TexCoord).r;
    vec4 color = texture(texture2, vec2(x + (16.0f / 256.0f) * paletteNum, 0.5f));
	FragColor = mix(vec4(x * 16.0f), color, color.a);
}
)";

const char mapVertexShaderSource[] = R"(
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

const char mapFragmentShaderSource[] = R"(
    #version 330 core
    
    uniform sampler2D texture1;
    uniform sampler2D texture2;
    
    in vec2 TexCoord;
    flat in float Palette;
    out vec4 FragColor;
    
    void main()
    {
        float x = texture(texture1, TexCoord).r;
        vec4 color = texture(texture2, vec2(x + (16.0f / 256.0f) * Palette, 0.5f));
        FragColor = vec4(mix(vec3(x * 16.0f), color.rgb, color.a), x == 0.0 ? 0.0f : 1.0f);
    }
    )";

const unsigned char sDefaultPalette[] = 
{
    16 * 0, 16 * 0, 16 * 0,
    16 * 1, 16 * 1, 16 * 2,
    16 * 2, 16 * 2, 16 * 2,
    16 * 3, 16 * 3, 16 * 3,
    16 * 4, 16 * 4, 16 * 4,
    16 * 5, 16 * 5, 16 * 5,
    16 * 6, 16 * 6, 16 * 6,
    16 * 7, 16 * 7, 16 * 7,
    16 * 8, 16 * 8, 16 * 8,
    16 * 9, 16 * 9, 16 * 9,
    16 * 10, 16 * 10, 16 * 10,
    16 * 11, 16 * 11, 16 * 11,
    16 * 12, 16 * 12, 16 * 12,
    16 * 13, 16 * 13, 16 * 13,
    16 * 14, 16 * 14, 16 * 14,
    16 * 15, 16 * 15, 16 * 15,
};

static const unsigned char transparentUnderlayColors[] = 
{
    0, 0, 0, 0
};
