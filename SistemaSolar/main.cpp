#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "stb_image.h"
#include "objloader.hpp"
#include "camera.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void setTexture(int index, const char * imgTexture);

//Para não precisar usar glm::
using namespace glm;

// camera
Camera camera(vec3(0.0f, 0.0f, 6.0f), vec3(-1.0f, -3.0f, 0.0f));
float lastX = 4 / 2.0f;
float lastY = 3 / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLFWwindow* window;
GLuint Texture[3];

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Falha ao iniciar o GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Sistema Solar (Sol/ Terra/ Lua)", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Falha ao abrir a janela GLFW.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Falha ao incializar a GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint programID = LoadShaders("TransformVertexShaderUV.vertexshader", "TextureFragmentShader.fragmentshader");
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    mat4 Projection = perspective(radians(camera.Zoom), 4.0f/3.0f, 0.1f, 100.0f);

    // Camera matrix
    mat4 View = camera.GetViewMatrix();
    
    // SOL
    mat4 Sol = mat4(1.0f);
    Sol = scale(Sol, vec3(0.9f));
    mat4 MVPSol = Projection * View * Sol;
    
    // TERRA
    mat4 Terra = mat4(1.0f);
    Terra = scale(Terra, vec3(0.005f));
    mat4 MVPTerra = Projection * View * Terra;

    // LUA
    mat4 Lua = mat4(1.0f);
    Lua = scale(Lua, vec3(0.0002f));
    mat4 MVPLua = Projection * View * Lua;
    
    camera.MovementSpeed = 10.0f;
    
    // ---------------------------------------------------------------------------
    // Carrega e cria uma textura
    // ---------------------------------------------------------------------------
    glEnable (GL_TEXTURE_2D);
    glGenTextures(3, Texture); //3 qtd de texturas
    
    setTexture(0, "2k_sun.jpg");
    setTexture(1, "2k_earth_daymap.jpg");
    setTexture(2, "2k_moon.jpg");

    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<vec3> normais;
    bool res = loadOBJ("sphere.obj", vertices, uvs, normais);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
    
    float angRotationSol = 0.0f;
    float angRotationTerra = 0.0f;
    float angRotationLua = 0.0f;

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        
        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window);
        
        View = camera.GetViewMatrix();
        
        Projection = perspective(radians(camera.Zoom), 4.0f/3.0f, 0.1f, 100.0f);

        // SOL
        angRotationSol += 0.005;
        Sol = rotate(mat4(1.0f), radians(angRotationSol), vec3(0.0f,1.0f,0.0f));
        MVPSol = Projection * View * Sol;
        
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPSol[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture[0]);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        
        
        // TERRA
        angRotationTerra += 0.3;
        Terra = translate(Sol, vec3(2.5f * cos((float)glfwGetTime() * 1.2f), 0.0f, 2.5f * sin((float)glfwGetTime() * 1.2f)));
        Terra = rotate(Terra, radians(angRotationTerra), vec3(0.0f,1.0f,0.0f));
        Terra = scale(Terra, vec3(0.3f));
        
        MVPTerra = Projection * View * Terra;

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPTerra[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture[1]);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        
        // LUA
        angRotationLua += 0.5;
        Lua = translate(Terra, vec3(2.0f * cos(glfwGetTime() * 0.1f), 0.0f, 2.0f * sin(glfwGetTime() * 0.1f)));
        Lua = rotate(Lua, radians(angRotationLua), vec3(0.0f,-1.0f,0.0f));
        Lua = scale(Lua, vec3(0.3f));
        MVPLua = Projection * View * Lua;

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPLua[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture[2]);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        
        glUniform1i(TextureID, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(3, Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    glfwTerminate();

    return 0;
}


void setTexture(int index, const char * imgTexture) {
    Texture[index] = index;
    glBindTexture(GL_TEXTURE_2D, Texture[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imgTexture, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        fprintf(stderr, "Falha ao carregar a textura ");
    }
    
    stbi_image_free(data);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
