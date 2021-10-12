#version 450    // Use GLSL 4.5

layout(location = 0) in vec3 fragColor;     // Interpolated color from vertex (location must match)
layout(location = 0) out vec4 outColor;     // Final output color (must also have location)

void main()
{
    outColor = vec4(fragColor, 1.0);
}