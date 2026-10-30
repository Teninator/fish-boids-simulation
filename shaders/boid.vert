#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;
uniform mat4 model;

void main() {
    FragPos = aPos; 
    
}