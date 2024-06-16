#version 400

in vec3 normal;
in vec3 fragPos;
in vec2 textureCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shiny;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 coefficients;
};

struct DirectedLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 coefficients;
    float cutoff;
    float outerCutoff;
};

uniform Material material;
#define POINT_LIGHTS 4
uniform PointLight pointLights[POINT_LIGHTS];
uniform DirectedLight directedLight;
uniform SpotLight spotLight;
uniform vec3 viewPos;

out vec4 fragColor;

vec3 computeDirectedLight(DirectedLight light, vec3 normal, vec3 viewDirection){
    vec3 lightDirection = normalize(-light.direction);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoords)) ;
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoords));

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shiny);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoords)) ;

    return ambient + diffuse + specular;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.coefficients.x + light.coefficients.y * distance + light.coefficients.z * distance * distance);

    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoords)) ;
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoords));

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shiny);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoords)) ;

    return (ambient + diffuse + specular) * attenuation;
}

vec3 computeSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position - fragPos);
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.coefficients.x + light.coefficients.y * distance + light.coefficients.z * distance * distance);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoords)) ;
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoords));

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shiny);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoords)) ;

    diffuse *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    vec3 norm = normalize(normal);
    vec3 viewDirection = normalize(viewPos - fragPos);

    vec3 result = computeDirectedLight(directedLight, norm, viewDirection);
    result += computeSpotLight(spotLight, norm, fragPos, viewDirection);
    for(int i = 0; i < POINT_LIGHTS; ++i){
        result += computePointLight(pointLights[i], norm, fragPos, viewDirection);
    }
    fragColor = vec4(result, 1.0);
}