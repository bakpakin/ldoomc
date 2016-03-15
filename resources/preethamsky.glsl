#version 330 core

uniform float luminence;
uniform float turbidity;
uniform float reileigh;
uniform float mieCoefficient;
uniform float mieDirectionalG;
uniform float sunPosition;

uniform sampler2D skySampler;

#ifdef VERTEX

layout(location = 0) in vec3 vertex;

smooth out vec3 position;

void main() {
    position = vertex;
    gl_Position = vec4(vertex, 1.0);
}

#endif

#ifdef FRAGMENT

smooth in vec3 position;
out vec4 color;

// Constants
const float e = 2.71828182845904523536028747135266249775724709369995957;
const float pi = 3.141592653589793238462643383279502884197169;

const float n = 1.0003; // refractive index of air
const float N = 2.545E25; // number of molecules per unit volume of air
const float pn = 0.035; // depolatization factor for standard air
const vec3 lambda = vec3(680E-9, 550E-9, 450E-9); // wavelength of used primaries, according to preetham
const vec3 K = vec3(0.686, 0.678, 0.666); // mie K coefficient for the primaries
const float v = 4.0;
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const vec3 up = vec3(0.0, 1.0, 0.0);
const float EE = 1000.0;
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;
const float cutoffAngle = pi/1.95;
const float steepness = 1.5;

vec3 simplifiedRayleigh(float lambda) {
	return 0.0005 / vec3(94, 40, 18);
}

float rayleighPhase(float cosTheta) {
	return (3.0 / (16.0*pi)) * (1.0 + pow(cosTheta, 2.0));
}

vec3 totalMie(vec3 lambda, vec3 K, float T) {
	float c = (0.2 * T ) * 10E-18;
	return 0.434 * c * pi * pow(2.0 * pi / lambda, vec3(v - 2.0)) * K;
}

float hgPhase(float cosTheta, float g) {
    return (1.0 / (4.0*pi)) * ((1.0 - pow(g, 2.0)) / pow(1.0 - 2.0*g*cosTheta + pow(g, 2.0), 1.5));
}

float sunIntensity(float zenithAngleCos) {
    return EE * max(0.0, 1.0 - exp(-((cutoffAngle - acos(zenithAngleCos))/steepness)));
}

void main() {
	color = vec4(0.7, 0.8, 1.0, 1.0);
}

#endif
