#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>
#define DEBUG 1

const GLuint WIDTH = 800, HEIGHT = 600;
GLuint program;


GLfloat vertices[] =
        {
                -0.6f, -0.4f, 0.0f,
                0.6f, -0.4f, 0.0f,
                0.f,  0.6f, 0.0f
        };

void myPrint( const char* toprint ){
    if(DEBUG)
        std::cout<< toprint<< std::endl;
}

GLuint loadShader(const GLchar* file_name, GLenum type) {
    std::ifstream file;
    char *src;
    GLsizei length;
    file.open(file_name);
    file.seekg(0, std::ios::end);
    length = file.tellg();
    src = new char[length];
    file.seekg(0, std::ios::beg);
    file.read(src, length);
    file.close();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, &length);
    glCompileShader(id);
    return id;
}

void initShaders()
{
    GLchar *vsSource, *fsSource;
    GLuint vs,fs;
    program = glCreateProgram();
    vs = loadShader("sample.vs", GL_VERTEX_SHADER);
    fs = loadShader("sample.fs", GL_FRAGMENT_SHADER);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv( vs , GL_COMPILE_STATUS, &success);
    if( !success ){
        glGetShaderInfoLog(vs,512,NULL,infoLog);
        myPrint("ERROR::SHADER::VERTEX::COMPILE");
        myPrint(infoLog);
    }
    glGetShaderiv( fs , GL_COMPILE_STATUS, &success);
    if( !success ){
        glGetShaderInfoLog(fs,512,NULL,infoLog);
        myPrint("ERROR::SHADER::FRAGMENT::COMPILE");
        myPrint(infoLog);

    }
    glAttachShader(program,vs);
    glAttachShader(program,fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if ( !success ){
        glGetShaderInfoLog(program,512,NULL,infoLog);
        myPrint("ERROR::SHADER::PROGRAM");
        myPrint(infoLog);

    }
    glDeleteShader(vs);
    glDeleteShader(fs);

}


int main()
{

    GLint mvp_location, vpos_location, vcol_location;
    GLuint vertex_buffer;
    GLuint VBO, VAO;

    glfwInit( );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    GLFWwindow *window = glfwCreateWindow( WIDTH, HEIGHT, "CENG477", nullptr, nullptr );

    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent( window );

    if ( GLEW_OK != glewInit( ) )
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glfwSwapInterval(1);
    initShaders();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (GLvoid *) nullptr);
    glEnableVertexAttribArray(0);

    while ( !glfwWindowShouldClose( window ) )
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);
        /*
        glm::mat4x4 m = glm::mat4();
        glm::rotate(m,(float) glfwGetTime(), glm::vec3(0,0,1) );
        glm::mat4x4 p = glm::ortho( -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glm::mat4x4 mvp = glm::matrixCompMult(p,m);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp[0][0]);
        */
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate( );

    return EXIT_SUCCESS;
}

