#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in VS_OUT {

     mat3 TBN;
} fs_in;
uniform sampler2D NormaldiffuseMap;
uniform sampler2D normal1Map;

uniform vec3 lightPos;

uniform DirLight dirLight;

uniform PointLight pointLight1;
uniform PointLight pointLight2;
uniform PointLight pointLight3;
uniform PointLight pointLight4;
uniform PointLight pointLight5;
uniform PointLight pointLight6;
uniform PointLight pointLight7;
uniform PointLight pointLight8;
uniform PointLight pointLight9;
uniform PointLight pointLight10;

uniform PointLight lampion;

uniform Material material;

uniform SpotLight spotLight;
uniform bool lightOn;
uniform bool pointLightOn;

uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir =fs_in.TBN * normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);
    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = fs_in.TBN *normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading, Blinn Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir =fs_in.TBN * normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main()
{
    normal = texture(normal1Map, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
   //  vec3 color = texture(NormaldiffuseMap, TexCoords).rgb;
         // ambient
    //     vec3 ambient = 0.1 * color;

  //      vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    //        float diff = max(dot(lightDir, normal), 0.0);
      //      vec3 diffuse = diff * color;





    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

   // vec3 reflectDir = reflect(-lightDir, normal);
    //vec3 halfwayDir = normalize(lightDir + viewDir);
   // float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);


  //  vec3 specular = vec3(0.2) * spec;



    vec3 result = CalcDirLight(dirLight, normal, viewDir);

    result += CalcPointLight(lampion, normal, FragPos, viewDir);

    if(pointLightOn){
        result += CalcPointLight(pointLight1, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight2, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight3, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight4, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight5, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight6, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight7, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight8, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight9, normal, FragPos, viewDir);
        result += CalcPointLight(pointLight10, normal, FragPos, viewDir);
    }

    if(lightOn)
        result += CalcSpotLight(spotLight, normal, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}