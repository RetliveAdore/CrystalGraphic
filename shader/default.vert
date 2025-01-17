#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 texCoord;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec4 colorConf;

layout (binding = 0) uniform UniformBufferObject{
    mat4 model;
    vec4 color;
    vec2 uvPos;
} ubo;
layout (binding = 1) uniform GlobalUniformBufferObject{
    mat4 view;
    vec4 ratio;
    vec4 colorFlag;
} g_ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    vec4 transfer = g_ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position = vec4(
         transfer.x * g_ubo.ratio.x,
        transfer.y * g_ubo.ratio.y,
        transfer.z * g_ubo.ratio.z,
        transfer.w
    );
    fragColor = inColor * ubo.color;
    fragTexCoord = texCoord + ubo.uvPos;
    colorConf = g_ubo.colorFlag;
}