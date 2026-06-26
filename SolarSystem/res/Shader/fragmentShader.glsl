#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D diffuseTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 baseColor;
uniform bool useTexture;
uniform bool useLighting;
uniform float alpha;

void main()
{
 vec4 sourceColor = useTexture ? texture(diffuseTexture, TexCoords) : vec4(baseColor, alpha);

 if (sourceColor.a < 0.1)
 {
 discard;
 }

 if (!useLighting)
 {
 FragColor = vec4(sourceColor.rgb, sourceColor.a * alpha);
 return;
 }

 vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
 vec3 viewDir = normalize(viewPos - FragPos);
 vec3 reflectDir = reflect(-lightDir, norm);

 float diff = max(dot(norm, lightDir), 0.0);
 float spec = pow(max(dot(viewDir, reflectDir), 0.0), 24.0);

 vec3 ambient = 0.22 * sourceColor.rgb;
 vec3 diffuse = diff * sourceColor.rgb;
 vec3 specular = vec3(0.18) * spec;

 FragColor = vec4(ambient + diffuse + specular, sourceColor.a * alpha);
}
