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
#include <fstream>
#define DEBUG 1

const GLuint WIDTH = 800, HEIGHT = 600;

GLuint program;

GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;

GLuint colorTexture;
GLuint heightTexture;

GLuint idMVPMatrix;

int vertexCount, textureWidth, textureHeight;

/***********************Variables defined by me *****************************/

glm::vec3 camera_pos;
glm::vec3 camera_up = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 camera_gaze = glm::vec3(0.0, 0.0, 1.0);
glm::vec3 camera_cross = cross(camera_up, camera_gaze);

GLfloat camera_speed = 0.0f;

bool fullscreen_mode;

int current_widthDisplay, current_heigthDisplay;


GLFWmonitor* primary_monitor;
const GLFWvidmode* vidmode;

/* These four variables should have the same type due to the template argument T
   that is used in the definition of the glm::perspective function */
GLfloat fovy = 45; /* Set the field of view for the projection matrix */
GLfloat aspect_ratio = 1;
GLfloat near = 0.1;
GLfloat far = 1000;

GLfloat heightFactor = 10.0f;

glm::vec3* vertices;
float vertices2[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
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

    glGenerateMipmap(GL_TEXTURE_2D);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    free( row_pointer[0] );
    free( raw_image );
    fclose( infile );

}


void viewConfig(){
    glm::mat4 m = glm::mat4();
    int matrixLocation = glGetUniformLocation( program, "rMat" );
    glUniformMatrix4fv( matrixLocation, 1, GL_FALSE, glm::value_ptr(m) );
}

void renderFragments(){

    float sinVal = sin( (float) glfwGetTime() ) / 2.0f;
    int colorLocation = glGetUniformLocation( program, "ourColor" );
    glUniform4f( colorLocation, sinVal, 0.0f, 0.5f, 1.0f);

}

void createWorld(){

    vertexCount = 6*textureHeight*textureWidth;

    vertices = new glm::vec3[vertexCount];

    int index = 0;
    
    //first one

    vertices[index] = glm::vec3( -0.5f, -0.5f, 0.0f );                       //0
    //std::cout << glm::to_string(vertices[index]) << std::endl;
    vertices[index+1] = glm::vec3( 0.5f, 0.5f, 0.0f );             //1
    //std::cout << glm::to_string(vertices[index+1]) << std::endl;                 
    vertices[index+2] = glm::vec3( 0.5f, -0.5f, 0.0f );                 //2
    //std::cout << glm::to_string(vertices[index+2]) << std::endl; 
    
    //second one
    

    vertices[index+3] = glm::vec3( -0.5f, 0.5f, 0.0f );                 //3
    //std::cout << glm::to_string(vertices[index+3]) << std::endl; 
    vertices[index+4] = glm::vec3( -0.5f, -0.5f, 0.0f );                     //0
    //std::cout << glm::to_string(vertices[index+4]) << std::endl; 
    vertices[index+5] = glm::vec3( 0.5f, 0.5f, 0.0f );             //1
    //std::cout << glm::to_string(vertices[index+5]) << std::endl; 

    /*
    

    for (int i = 0; i < textureWidth; i++)
    {
        for (int j = 0; j < textureHeight; j++)
           {
                //first one

                vertices[index] = glm::vec3( i, 0, j );                       //0
                //std::cout << glm::to_string(vertices[index]) << std::endl;
                vertices[index+1] = glm::vec3( (i+1), 0, (j+1) );             //1
                //std::cout << glm::to_string(vertices[index+1]) << std::endl;                 
                vertices[index+2] = glm::vec3( (i+1), 0, j );                 //2
                //std::cout << glm::to_string(vertices[index+2]) << std::endl; 
                
                //second one

                vertices[index+3] = glm::vec3( i, 0, (j+1) );                 //3
                //std::cout << glm::to_string(vertices[index+3]) << std::endl; 
                vertices[index+4] = glm::vec3( i, 0, j );                     //0
                //std::cout << glm::to_string(vertices[index+4]) << std::endl; 
                vertices[index+5] = glm::vec3( (i+1), 0, (j+1) );             //1
                //std::cout << glm::to_string(vertices[index+5]) << std::endl; 

                index += 6;

           } 
    }
    */

    /*
    for (int i = 0; i < textureWidth; i++)
    {
        for (int j = 0; j < textureHeight; j++)
           {
                //first one

                vertices[index] = glm::vec3( i, 0, j );                       //0
                //std::cout << glm::to_string(vertices[index]) << std::endl;
                vertices[index+1] = glm::vec3( (i+1), 0, (j+1) );             //1
                //std::cout << glm::to_string(vertices[index+1]) << std::endl;                 
                vertices[index+2] = glm::vec3( (i+1), 0, j );                 //2
                //std::cout << glm::to_string(vertices[index+2]) << std::endl; 
                
                //second one

                vertices[index+3] = glm::vec3( i, 0, (j+1) );                 //3
                //std::cout << glm::to_string(vertices[index+3]) << std::endl; 
                vertices[index+4] = glm::vec3( i, 0, j );                     //0
                //std::cout << glm::to_string(vertices[index+4]) << std::endl; 
                vertices[index+5] = glm::vec3( (i+1), 0, (j+1) );             //1
                //std::cout << glm::to_string(vertices[index+5]) << std::endl; 

                index += 6;

           } 
    }
    */
}


int main()
{

    GLint mvp_location, vpos_location, vcol_location;
    GLuint vertex_buffer;
    GLuint VBO, VAO;

    glfwInit( );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

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

    glfwSwapInterval(1);                    //60 fps

    initShaders();                          //Shader initialisation

    glGenVertexArrays(1, &VAO);             //VERTEX ARRAY

    glGenBuffers(1, &VBO);                  //VERTEX BUFFER

    textureWidth = 1;
    textureHeight = 1;

    createWorld();                          //CREATE THE VERTICES

    glBindVertexArray(VAO);

    std::cout<< vertexCount << std::endl;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, &vertices[0], GL_STATIC_DRAW);//Introduce the vertices to the buffer


    //Introduce vertice info
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *) 0);
    glEnableVertexAttribArray(0); // Used for switching between vertex arrays

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);    

    /*

    //Introduce vertice attrib 2 info
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (GLvoid *) (3*sizeof(GLfloat)) ));
    glEnableVertexAttribArray(1); // Used for switching between vertex arrays

    */

    /*

    glGenTextures(1,&colorTexture);
    glGenTextures(1,&heightTexture);

    initTexture( (char *) "height_gray_mini.jpg", 1, &textureWidth, &textureHeight ); // HEIGHT
    initTexture( (char *) "normal_earth_mini.jpg", 0, &textureWidth, &textureHeight ); // COLOR

    */

    viewConfig();

    GLint textWidthLocation = glGetUniformLocation(program, "textureWidth");
    glUniform1i(textWidthLocation, textureWidth);

    GLint textHeightLocation = glGetUniformLocation(program, "textureHeight");
    glUniform1i(textHeightLocation, textureHeight);

    while ( !glfwWindowShouldClose( window ) )
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        

        glClearColor(0.2f,0.2f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);

        //renderFragments();

        /*
        textCoord = vec2( (1 - float(position.x) / (textureWidth + 1 ) ), (1 - float(position.z) / (textureHeight + 1 ) ) );
        glm::mat4x4 m = glm::mat4();
        glm::rotate(m,(float) glfwGetTime(), glm::vec3(0,0,1) );
        glm::mat4x4 p = glm::ortho( -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glm::mat4x4 mvp = glm::matrixCompMult(p,m);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp[0][0]);
        */

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate( );

    return EXIT_SUCCESS;
}

