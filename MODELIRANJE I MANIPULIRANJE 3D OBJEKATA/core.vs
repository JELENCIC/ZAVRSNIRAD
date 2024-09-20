#version 330 core
layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

uniform mat4 model; //local object coordinates to camera coordinates
uniform mat4 view; //normalized coordinates to window coordinates
uniform mat4 projection; //camera coordinates to normalized coordinates between 0 and 1

//out vec3 ourColor;
out vec2 TexCoord;

//uniform mat4 transform;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    //gl_Position = transform * vec4(position, 1.0f);
    //ourColor = color;
    TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}
