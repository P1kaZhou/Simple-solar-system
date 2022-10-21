#version 330 core
// Minimal GL version support expected from the GPU
in vec3 fNormal;
in vec3 fPosition;
in vec2 TexCoord;

uniform vec3 camPos;
uniform vec3 diffuseColor;

// out vec3 FragColor;
out vec4 color;	  // Shader output: color of this fragment

struct Material {
	// ...
	sampler2D albedoTex; // texture unit, relate to glActivateTexture(GL_TEXTURE0 + i)
};
uniform Material material;

void main() {

	vec3 FragColor = texture(material.albedoTex, TexCoord).rgb;

	float kd = 0.7;
	float alpha = 0.6;
	float ks = 0.4;

	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // light direction vector (hard-coded just for now)

	vec3 v = normalize(camPos - fPosition);
	vec3 r = normalize(2*n*dot(n,l) - l);
	vec3 ambient = FragColor;
	vec3 diffuse = kd*max(dot(n,l), 0)*(ambient*diffuseColor);
	vec3 specular = ks*pow(max(dot(v,l),0),alpha)*(ambient);

	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.
}