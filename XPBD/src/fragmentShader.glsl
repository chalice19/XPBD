#version 330 core            // minimal GL version support expected from the GPU

struct LightSource {
  vec3 position;
  vec3 color;
  float intensity;
};

uniform LightSource lightSrc;
uniform sampler2D shadowMapTex;
uniform mat4 shadowMapMVP;

struct Material {
  vec3 albedo;
  sampler2D albedoTex;
  int albedoTexLoaded;

  sampler2D normalTex;
  int normalTexLoaded;
};

uniform Material material;

uniform vec3 camPos;

in vec3 fPositionModel;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorOut; // shader output: the color response attached to this fragment

uniform mat4 modelMat;
uniform mat3 normMat;

float pi = 3.1415927;
float shadowOffset = 0.001;

void main() {
  vec3 n = (material.normalTexLoaded == 1) ?
    normalize(normMat*((texture(material.normalTex, fTexCoord).rgb - 0.5)*2.0)) : // colors are in [0,1]^3, and normals are in [-1,1]^3
    normalize(fNormal);

  // linear barycentric interpolation does not preserve unit vectors
  // vec3 wo = normalize(camPos - fPosition); // unit vector pointing to the camera

  vec4 ShadowCoord = shadowMapMVP*modelMat*vec4(fPositionModel, 1);
  ShadowCoord /= ShadowCoord.w;

  // so far, ShadowCoord is in [-1,1]^3, put it in [0,1]^3:
  ShadowCoord = ShadowCoord*0.5 + 0.5;

  vec3 radiance = vec3(0, 0, 0);
  if(texture(shadowMapTex, ShadowCoord.xy).r < ShadowCoord.z - shadowOffset) {
    // useful : debug
    // radiance = vec3(ShadowCoord2D, 0);
  } else {
    vec3 wi = normalize(lightSrc.position - fPosition); // unit vector pointing to the light source
    vec3 Li = lightSrc.color*lightSrc.intensity;
    vec3 albedo = material.albedoTexLoaded==1 ? texture(material.albedoTex, fTexCoord).rgb : material.albedo;
    //vec3 albedo = material.albedo;

    radiance += Li*albedo*max(dot(n, wi), 0);
    // radiance += Li*albedo;
  }

  {
    // !!!!!! DEBUG YOUR TEXTURES !!!!!! :
    //   radiance = vec3(1) * max(dot(n,wo),0.0);
    //   radiance = 0.001*radiance + 0.999*((texture(material.normalTex, fTexCoord).rgb));
    //   radiance = 0.001*radiance + 0.999*((texture(material.albedoTex, fTexCoord).rgb));
  }

  colorOut = vec4(radiance, 1.0); // build an RGBA value from an RGB one
}
