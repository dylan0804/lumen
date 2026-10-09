#version 330
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D ourTexture;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;
void main()
{
    float ambientStr = 0.1;
    float specularStrength = 0.5;
    vec3 ambient = ambientStr * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 fillColor = texture(ourTexture, TexCoord).rgb;
    vec3 result = (ambient + diffuse + specular) * fillColor;
    FragColor = vec4(result, 1.0);
}
