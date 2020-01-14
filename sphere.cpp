#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include "glm/mat4x4.hpp"
#include "glm/glm.hpp"
#include <jpeglib.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <fstream>
#define DEBUG 1

float PI = glm::pi<float>();

const GLuint WIDTH = 1000, HEIGHT = 1000;

GLuint program;

GLuint colorTexture;
GLuint heightTexture;

int vertexCount, textureWidth, textureHeight;
int horizontalCount, verticalCount;

float textureRatio;
float heightFactor = 10.0f;
float radius = 350.0f;

glm::mat4 m = glm::mat4();
glm::mat4 mCam, mPer, mNormInv, mVP;

glm::vec3 cameraPos;
glm::vec3 cameraUp;
glm::vec3 cameraGaze;
glm::vec3 cameraRight;
glm::vec3 cameraTarget;
glm::vec3 lightPos;

float cameraSpeed = 0.0f;
float yawChange = 0.05f;
float pitchChange = 0.05f;
float speedChange = 0.01f;
float hfChange = 0.5f;
float lightChange = 5.0f;

int textureOffset = 0;
int textureChange = 1;

bool flag = false;

GLFWwindow *window;

int windowWidth, windowHeight;
int prevWindowWidth, prevWindowHeight;

GLFWmonitor* monitor;
const GLFWvidmode* mode;


glm::vec4* vertices;

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
    GLuint vs,fs;
    program = glCreateProgram();
    vs = loadShader("sphere.vs", GL_VERTEX_SHADER);
    fs = loadShader("sphere.fs", GL_FRAGMENT_SHADER);
    GLint status;
    GLchar infoLog[512];
    glGetShaderiv( vs , GL_COMPILE_STATUS, &status);
    if( !status ){
        glGetShaderInfoLog(vs,512,nullptr,infoLog);
        myPrint("ERROR::SHADER::VERTEX::COMPILE");
        myPrint(infoLog);
    }
    glGetShaderiv( fs , GL_COMPILE_STATUS, &status);
    if( !status ){
        glGetShaderInfoLog(fs,512,nullptr,infoLog);
        myPrint("ERROR::SHADER::FRAGMENT::COMPILE");
        myPrint(infoLog);

    }
    glAttachShader(program,vs);
    glAttachShader(program,fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if ( !status ){
        glGetShaderInfoLog(program,512,nullptr,infoLog);
        myPrint("ERROR::SHADER::PROGRAM");
        myPrint(infoLog);

    }
    glDeleteShader(vs);
    glDeleteShader(fs);

}


void initTexture(char *filename, int textureType, int *w, int *h)
{
    int width, height;

    unsigned char *raw_image = nullptr;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JSAMPROW row_pointer[1];

    FILE *infile = fopen( filename, "rb" );
    unsigned long location = 0;
    int i = 0, j = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file %s\n!", filename );
        return;
    }
    //printf("Texture filename = %s\n",filename);

    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_decompress( &cinfo );
    jpeg_stdio_src( &cinfo, infile );
    jpeg_read_header( &cinfo, TRUE );
    jpeg_start_decompress( &cinfo );

    raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
    row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
    while( cinfo.output_scanline < cinfo.image_height )
    {
        jpeg_read_scanlines( &cinfo, row_pointer, 1 );
        for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    if (textureType == 0){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        GLint colorTextLocation = glGetUniformLocation(program, "colorTexture");
        glUniform1i(colorTextLocation, 0);

    }
    else{
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, heightTexture);

        GLint heightTextLocation = glGetUniformLocation(program, "heightTexture");
        glUniform1i(heightTextLocation, 1);
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    *w = width;
    *h = height;

    textureRatio = width/height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    free( raw_image );
    fclose( infile );

}

void viewConfig(){

    textureOffset = 0;

    heightFactor = 0.0f;

    cameraSpeed = 0.0f;

    lightPos = glm::vec3(0.0f,1600.0f,0.0f);

    cameraGaze = glm::vec3(0.0f,-1.0f,0.0f);
    cameraUp = glm::vec3(0.0f,0.0f,1.0f);

    cameraRight = glm::cross( cameraGaze, cameraUp );

    cameraPos = glm::vec3( 0.0f, 600.0f, 0.0f);

    cameraTarget = cameraPos + cameraGaze*(0.1f) ;

    mCam = glm::lookAt( cameraPos, cameraTarget, cameraUp ) ;

    mNormInv = glm::inverseTranspose(mCam);

    mVP = mPer * (mCam * glm::mat4(1.0f));

}

void updateCamera(){
    
    cameraPos += cameraSpeed*cameraGaze;

    //std::cout<<glm::to_string(cameraPos)<<std::endl;

    cameraTarget = cameraPos + cameraGaze*(0.1f) ;

    mCam = glm::lookAt( cameraPos, cameraTarget, cameraUp ) ;

    mNormInv = glm::inverseTranspose(mCam);

    mVP = mPer * (mCam * glm::mat4(1.0f));
}

void updateUniforms(){

    glUseProgram(program);

    int location;

    int cMatlocation = glGetUniformLocation( program, "cMat" );
    glUniformMatrix4fv( cMatlocation, 1, GL_FALSE, glm::value_ptr(mCam) );

    int pMatlocation = glGetUniformLocation( program, "pMat" );
    glUniformMatrix4fv( pMatlocation, 1, GL_FALSE, glm::value_ptr(mPer) );

    int nMatlocation = glGetUniformLocation( program, "nMat" );
    glUniformMatrix4fv( nMatlocation, 1, GL_FALSE, glm::value_ptr(mNormInv) );

    int vpMatlocation = glGetUniformLocation( program, "vpMat" );
    glUniformMatrix4fv( vpMatlocation, 1, GL_FALSE, glm::value_ptr(mVP) );

    int heightFactorlocation = glGetUniformLocation( program, "heightFactor" );
    glUniform1f( heightFactorlocation, heightFactor );

    int textureOffsetlocation = glGetUniformLocation( program, "textureOffset" );
    glUniform1i( textureOffsetlocation, textureOffset );

    int cameraPoslocation = glGetUniformLocation( program, "cameraPos" );
    glUniform3fv( cameraPoslocation, 1, glm::value_ptr(cameraPos) );

    int lightPoslocation = glGetUniformLocation( program, "lightPos" );
    glUniform3fv( lightPoslocation, 1, glm::value_ptr(lightPos) );
}

void createWorld(){

    horizontalCount = 250;
    verticalCount = 125;

    vertexCount = 6*horizontalCount*verticalCount;

    vertices = new glm::vec4[vertexCount];

    int index = 0;

    float alpha0, alpha1, alpha2, alpha3;
    float beta0, beta1, beta2, beta3;

    float horizontalStep = (2 * PI)/horizontalCount;
    float verticalStep = PI/verticalCount;

    float u0, u1, u2, u3;
    float v0, v1, v2, v3;


    for (int i = 0; i < verticalCount; i++) //TODO EQUAL
    {

        beta0 = i * verticalStep;
        v0 = float(i) / verticalCount;

        beta1 = (i+1) * verticalStep;
        v1 = float(i+1) / verticalCount;

        beta2 = i * verticalStep;
        v2 = float(i) / verticalCount;

        beta3 = (i+1) * verticalStep;
        v3 = float(i+1) / verticalCount;

        for (int j = 0; j < horizontalCount; j++)
           {

                alpha0 = j * horizontalStep;
                u0 = float(j) / horizontalCount;

                alpha1 = j * horizontalStep;
                u1 = float(j) / horizontalCount;

                alpha2 = (j+1) * horizontalStep;
                u2 = float(j+1) / horizontalCount;

                alpha3 =  (j+1) * horizontalStep;
                u3 = float(j+1) / horizontalCount;

                //first one

                vertices[index] = glm::vec4( alpha0, beta0, u0,v0  );  
                                    //0
                vertices[index+1] = glm::vec4( alpha1, beta1, u1,v1  );                    //1
                //std::cout << glm::to_string(vertices[index+1]) << std::endl;                 
                vertices[index+2] = glm::vec4( alpha3, beta3, u3,v3  );                    //2
                //std::cout << glm::to_string(vertices[index+2]) << std::endl; 
                
                //second one

                vertices[index+3] = glm::vec4( alpha2, beta2, u2,v2  );                    //3
                vertices[index+4] = glm::vec4( alpha0, beta0, u0,v0 );                     //1
                vertices[index+5] = glm::vec4( alpha3, beta3, u3,v3  );                    //0

                index += 6;

           } 
    }

}


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods){

    if(action == GLFW_PRESS){
        switch(key){

            case GLFW_KEY_ESCAPE:
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            }
            case GLFW_KEY_P: //fullscreen
            {

                if(!flag){
                    flag = true;
                    prevWindowWidth = windowWidth;
                    prevWindowHeight = windowHeight;
                    windowHeight = mode->height;
                    windowWidth = mode->width;

                    //std::cout<<width <<" " << height<<std::endl;
                    glfwSetWindowMonitor(window, monitor,0,0,windowWidth, windowHeight, mode->refreshRate);

                }
                else{
                    flag = false;
                    windowWidth = prevWindowWidth;
                    windowHeight = prevWindowHeight;
                    //std::cout<<width <<" " << height<<std::endl;

                    glfwSetWindowMonitor(window, nullptr, 0, 0, windowWidth, windowHeight, 0);
                }
                break;
            }
            case GLFW_KEY_X:
            {
                cameraSpeed = 0.0f;
                break;
            }
            case GLFW_KEY_I:
            {
                viewConfig();
                break;
            }
        }
    }

    
    if(action == GLFW_PRESS || action == GLFW_REPEAT){
        switch(key){
            case GLFW_KEY_W://camera gaze move
            {
                cameraGaze = glm::rotate(cameraGaze, pitchChange, cameraRight );
                cameraUp = glm::rotate(cameraUp, pitchChange, cameraRight );
                break;
            }
            case GLFW_KEY_A:
            {
                cameraGaze = glm::rotate(cameraGaze, pitchChange, cameraUp );
                cameraRight = glm::rotate(cameraRight, pitchChange, cameraUp );
                break;
            }
            case GLFW_KEY_S:
            {
                cameraGaze = glm::rotate(cameraGaze, -pitchChange, cameraRight );
                cameraUp = glm::rotate(cameraUp, -pitchChange, cameraRight );
                break;
            }
            case GLFW_KEY_D:
            {
                cameraGaze = glm::rotate(cameraGaze, -pitchChange, cameraUp );
                cameraRight = glm::rotate(cameraRight, -pitchChange, cameraUp );
                break;
            }
            case GLFW_KEY_Q: //texture map move
            {
                textureOffset += textureChange;
                //std::cout<<textureOffset<<std::endl;
                break;
            }
            case GLFW_KEY_E:
            {
                textureOffset -= textureChange;
                break;
            }
            case GLFW_KEY_UP: //light
            {
                lightPos.z += lightChange;
                break;
            }
            case GLFW_KEY_DOWN:
            {
                lightPos.z -= lightChange;
                break;
            }
            case GLFW_KEY_LEFT:
            {
                lightPos.x += lightChange; 
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                lightPos.x -= lightChange;
                break;
            }
            case GLFW_KEY_T:
            {
                lightPos.y += lightChange;
                break;
            }
            case GLFW_KEY_G:
            {
                lightPos.y -= lightChange;
                break;
            }
            case GLFW_KEY_R://heightfactor
            {
                heightFactor += hfChange;
                break;
            }
            case GLFW_KEY_F:
            {
                heightFactor -= hfChange;
                break;
            }
            case GLFW_KEY_Y://camera speed
            {
                cameraSpeed += speedChange;
                break;
            }
            case GLFW_KEY_H:
            {
                cameraSpeed -= speedChange;
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{

    if(argc == 3){

        GLuint vertex_buffer;
        GLuint VBO, VAO;
        glfwInit( );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
        //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
        window = glfwCreateWindow( WIDTH, HEIGHT, "CENG477", nullptr, nullptr );
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
        glfwSetKeyCallback(window, keyCallback);
        glfwSwapInterval(1);                    //60 fps
        initShaders();                          //Shader initialisation
        glGenVertexArrays(1, &VAO);             //VERTEX ARRAY
        glGenBuffers(1, &VBO);                  //VERTEX BUFFER
        glGenTextures(1,&colorTexture);
        glGenTextures(1,&heightTexture);
        
        initTexture( argv[1], 1, &textureWidth, &textureHeight ); // HEIGHT TODO
        initTexture( argv[2], 0, &textureWidth, &textureHeight ); // COLOR TODO

        createWorld();                          //CREATE THE VERTICES
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertexCount, &vertices[0], GL_STATIC_DRAW);//Introduce the vertices to the buffer
        //Introduce vertice info
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *) 0);

        glEnableVertexAttribArray(0); // Used for switching between vertex arrays
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);    
        glUseProgram(program);

        mPer = glm::perspective(45.0f,1.0f,0.1f,1000.f);

        viewConfig();
        updateUniforms();

        GLint textWidthLocation = glGetUniformLocation(program, "textureWidth");
        glUniform1i(textWidthLocation, textureWidth);
        GLint textHeightLocation = glGetUniformLocation(program, "textureHeight");
        glUniform1i(textHeightLocation, textureHeight);

        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        glEnable(GL_DEPTH_TEST);

        while ( !glfwWindowShouldClose( window ) )
        {

            glViewport(0, 0, windowWidth, windowHeight);
            
            glClearDepth(1.0f);
            glClearColor(0.0f,0.0f,0.2f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(program);
            glBindVertexArray(VAO);

            updateCamera();
            updateUniforms();

            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate( );
    }
    else{
        myPrint("Please input 2 jpg files");
    }

    return EXIT_SUCCESS;
}

