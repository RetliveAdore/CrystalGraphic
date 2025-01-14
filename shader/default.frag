#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec4 colorConf;

layout (location = 0) out vec4 outColor;

layout (binding = 2) uniform sampler2D texSampler;

float RGB2SRGB(float v)
{
    if (v <= 0.0031308)
        return v * 12.92;
    return 1.055 * pow(v, 1.0 / 2.4);
}

float SRGB2RGB(float v)
{
    if (v <= 0.04045)
        return v / 12.92;
    return pow((v + 0.055) / 1.055, 2.4);
}

vec4 transfer(vec4 v)
{
    if (colorConf.x > 0)
    {
        vec4 back;
        back.x = SRGB2RGB(v.x);
        back.y = SRGB2RGB(v.y);
        back.z = SRGB2RGB(v.z);
        back.w = SRGB2RGB(v.w);
        return back;
    }
    return v;
}

void main()
{
    vec4 convColor = fragColor * texture(texSampler, fragTexCoord);
    outColor = transfer(convColor);
}