#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLight;
uniform vec3 pointLightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
vec3 ambient2;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
vec3 diffuse2;
vec3 specular2;
float specularStrength = 0.5f;

uniform bool fog;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f; 

void computePointLight(){
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 lightPosEye = vec3(view * model * vec4(pointLight,1.0f));
    vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);
    vec3 viewDir = normalize(- fPosEye.xyz);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 halfVector = normalize(lightDirN + viewDir);

    float dist = length(lightPosEye - fPosEye.xyz);
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    ambient2 = att * ambientStrength * pointLightColor;
    diffuse2 = att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;

    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);
    specular = att * specularStrength * specCoeff * pointLightColor;
}

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

float computeFog()
{
    float fogDensity = 0.05f;
    float fragmentDistance = length(view * model * vec4(fPosition, 1.0f));
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    computeDirLight();
    computePointLight();
    float fogFactor = computeFog();
    //compute final vertex color
    //computing dir light result
    vec3 lightResult = ((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb);
    //adding point light result
    lightResult += ((ambient2 + diffuse2) * texture(diffuseTexture, fTexCoords).rgb + specular2 * texture(specularTexture, fTexCoords).rgb);
    vec3 color = min(lightResult, 1.0f);
    vec4 fogColor = vec4(0.75f, 0.7f, 0.5f, 1.0f);
    if(fog)
        fColor = mix(fogColor,vec4(color, 1.0f),fogFactor);
    else
        fColor = vec4(color, 1.0f);
}
