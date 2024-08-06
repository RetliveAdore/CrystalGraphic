#version 330
in vec2 uv_out;
uniform vec4 paintColor;
uniform sampler2D aTex0;
out vec4 FragColor;
void main()
{
    FragColor = texture(aTex0, uv_out) * paintColor;
}