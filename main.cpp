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

const GLuint WIDTH = 600, HEIGHT = 600;

GLuint program;

GLuint colorTexture;
GLuint heightTexture;

int vertexCount, textureWidth, textureHeight;

float textureRatio;
float heightFactor = 10.0f;

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

bool fsRender;
bool flag = false;

GLFWwindow *window;

int windowWidth, windowHeight;


GLFWmonitor* monitor;
const GLFWvidmode* mode;


glm::vec3* vertices;

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
    vs = loadShader("sample.vs", GL_VERTEX_SHADER);
    fs = loadShader("sample.fs", GL_FRAGMENT_SHADER);
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

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen( filename, "rb" );
    unsigned long location = 0;
    int i = 0, j = 0;

    if ( !infile )
    {
        printf("Error opening jpeg file %s\n!", filename );
        return;
    }
    printf("Texture filename = %s\n",filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error( &jerr );
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress( &cinfo );
    /* this makes the library read from infile */
    jpeg_stdio_src( &cinfo, infile );
    /* reading the image header which contains image information */
    jpeg_read_header( &cinfo, TRUE );
    /* Start decompression jpeg here */
    jpeg_start_decompress( &cinfo );

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
    /* read one scan line at a time */
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
    /* wrap up decompression, destroy objects, free pointers and close open files */

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
    cameraGaze = glm::vec3(0.0f,0.0f,1.0f);

    cameraPos = glm::vec3(textureWidth/2, textureWidth/10, -textureWidth/4);

    cameraUp = glm::vec3(0.0f,1.0f,0.0f);

    cameraTarget = cameraPos + cameraGaze*(0.1f) ;

    cameraRight = glm::cross( cameraGaze, cameraUp );

    mCam = glm::lookAt( cameraPos, cameraTarget, cameraUp ) ;

    mNormInv = glm::inverseTranspose(mCam);

    mVP = mPer * mCam;
    
}

void updateCamera(){
    
    cameraPos += cameraSpeed*cameraGaze;

    cameraTarget = cameraPos + cameraGaze*(0.1f) ;

    mCam = glm::lookAt( cameraPos, cameraTarget, cameraUp ) ;

    mNormInv = glm::inverseTranspose(mCam);

    mVP = mPer * mCam;
}

void updateUniforms(){

    glUseProgram(program);

    int location;

    location = glGetUniformLocation( program, "cMat" );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(mCam) );

    location = glGetUniformLocation( program, "pMat" );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(mPer) );

    location = glGetUniformLocation( program, "nMat" );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(mNormInv) );

    location = glGetUniformLocation( program, "vpMat" );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(mVP) );

    location = glGetUniformLocation( program, "heightFactor" );
    glUniform1f( location, heightFactor );
}

void createWorld(){

    vertexCount = 6*textureHeight*textureWidth;

    vertices = new glm::vec3[vertexCount];

    int index = 0;

    for (int i = 0; i < textureWidth; i++)
    {
        for (int j = 0; j < textureHeight; j++)
           {
                //first one

                vertices[index] = glm::vec3( i, 0, j );                       //0
                //std::cout << glm::to_string(vertices[index]) << std::endl;
                vertices[index+1] = glm::vec3( (i+1), 0, (j+1) );             //1
                //std::cout << glm::to_string(vertices[index+1]) << std::endl;                 
                vertices[index+2] = glm::vec3( (i+1), 0, j);                 //2
                //std::cout << glm::to_string(vertices[index+2]) << std::endl; 
                
                //second one

                vertices[index+3] = glm::vec3( i, 0, (j+1) );                 //3
                //std::cout << glm::to_string(vertices[index+3]) << std::endl; 
                vertices[index+4] = glm::vec3( i, 0, j);                     //0
                //std::cout << glm::to_string(vertices[index+4]) << std::endl; 
                vertices[index+5] = glm::vec3( (i+1), 0, (j+1));             //1
                //std::cout << glm::to_string(vertices[index+5]) << std::endl; 

                index += 6;

           } 
    }

}


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods){

    bool fs = false;
    if(action == GLFW_PRESS){
        switch(key){

            case GLFW_KEY_ESCAPE:
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            }
            case GLFW_KEY_P: //fullscreen
            {
                fs = true;
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
            case GLFW_KEY_X:
            {
                cameraSpeed = 0.0f;
                break;
            }
            case GLFW_KEY_I:
            {
                cameraSpeed = 0.0f;
                viewConfig();
                break;
            }
        }
    }

    /*

    if(fs){ //TODO
        fs = false;
        flag = true;
        if(!fsRender){
            fsRender = true;
            windowHeight = mode->height;
            windowWidth = mode->width;

            //std::cout<<width <<" " << height<<std::endl;
            glfwSetWindowMonitor(window, monitor,0,0,windowWidth, windowHeight, mode->refreshRate);

        }
        else{
            fsRender = false;
            windowHeight = heightDisplay;
            windowWidth = widthDisplay;
            //std::cout<<width <<" " << height<<std::endl;

            glfwSetWindowMonitor(window, nullptr, 0, 0, windowWidth, windowHeight, 0);
        }
    }

    */

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
                lightPos.x -= lightChange; 
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                lightPos.x += lightChange;
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
        }
    }
}

int main()
{

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
    initTexture( (char *) "height_gray_mini.jpg", 1, &textureWidth, &textureHeight ); // HEIGHT
    //std::cout<< textureWidth << " " << textureHeight << std::endl;
    initTexture( (char *) "normal_earth_mini.jpg", 0, &textureWidth, &textureHeight ); // COLOR
    //std::cout<< textureWidth << " " << textureHeight << std::endl;
    
    glfwSetWindowSize(window, HEIGHT*textureRatio, HEIGHT);
    glfwSetWindowAspectRatio(window, HEIGHT*textureRatio, HEIGHT);
    createWorld();                          //CREATE THE VERTICES
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, &vertices[0], GL_STATIC_DRAW);//Introduce the vertices to the buffer
    //Introduce vertice info
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *) 0);
    glEnableVertexAttribArray(0); // Used for switching between vertex arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
    glUseProgram(program);

    mPer = glm::perspective(glm::radians(45.0f),1.0f,0.1f,1000.f);

    viewConfig();

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
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        glClearColor(0.2f,0.2f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);

        updateCamera();
        updateUniforms();

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate( );

    return EXIT_SUCCESS;
}

