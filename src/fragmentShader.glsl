#version 330 core
// Minimal GL version support expected from the GPU
in vec3 fNormal;
in vec3 fPosition;
in vec2 TexCoord;

uniform vec3 camPos;
uniform vec3 color;
uniform sampler2D ourTexture;

out vec4 FragColor;
out vec4 Megacolor;	  // Shader output: color of this fragment



	void main() {

		float kd = 0.7;
		float alpha = 0.6;
		float ks = 0.5;

		vec3 n = normalize(fNormal);
		vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // light direction vector (hard-coded just for now)

		vec3 v = normalize(camPos - fPosition);
		vec3 r = normalize(2*n*dot(n,l) - l);
		vec3 ambient = color;
		vec3 diffuse = kd*max(dot(n,l), 0)*(ambient*vec3(1.0,1.0,1.0));
		vec3 specular = ks*pow(max(dot(v,l),0),alpha)*(ambient*vec3(1.0,1.0,1.0));


		Megacolor = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.
		FragColor = texture(ourTexture, TexCoord);
	}
