#version 330 core

in vec3 ourColor;

out vec4 FragColor;

uniform float turboActivo;
uniform float useTurboColor;
uniform float useOverrideColor;
uniform float overrideR;
uniform float overrideG;
uniform float overrideB;

void main()
{
    vec3 baseColor = ourColor;
    vec3 turboColor = vec3(1.0, 0.2, 0.1);

    if (useOverrideColor > 0.5)
    {
        baseColor = vec3(overrideR, overrideG, overrideB);
    }

    if (useTurboColor > 0.5)
    {
        baseColor = mix(baseColor, turboColor, turboActivo);
    }

    FragColor = vec4(baseColor, 1.0);
}
