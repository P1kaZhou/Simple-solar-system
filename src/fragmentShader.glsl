#version 330 core
// Minimal GL version support expected from the GPU
in vec3 fNormal;
out vec4 color;	  // Shader output: color of this fragment
uniform vec3 camPos;
in vec3 fPosition;

void main() {

	float kd = 0.7;
	float alpha = 0.6;
	float ks = 0.5;

	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // light direction vector (hard-coded just for now)

	vec3 v = normalize(camPos - fPosition);
	vec3 r = normalize(2*n*dot(n,l) - l);
	vec3 ambient = vec3(1.0,1.0,0.0);
	vec3 diffuse = kd*max(dot(n,l), 0)*(ambient*vec3(1.0,1.0,1.0));
	vec3 specular = ks*pow(max(dot(v,l),0),alpha)*(ambient*vec3(1.0,1.0,1.0));
	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.

}
