#ifdef _WINDOWS
#include <GL/glew.h>

#endif
#define PI 3.1415926535897932384626433832795f
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


struct Square {
    float vertices[12] = {-0.3, -0.25, 0.3, -0.25, 0.3, 0.25, -0.3, -0.25, 0.3, 0.25, -0.3, 0.25};
    float texCoords[12] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    float current_x;
    float direction;
    float speed;
    Square(float current_x, float direction, float speed) :
    current_x(current_x), direction(direction), speed(speed){}
    
    void move(float time) {
        current_x += direction * speed * time;
    }
};

void checkWallHitted(Square& a, float width) {
    if ((a.current_x + 0.35 >= width && a.direction == 1) ||
        (a.current_x - 0.35 <= -width && a.direction == -1)) {
        a.direction = -a.direction;
    }
}

bool checkCollision(Square& a, Square& b) {
    if ((a.current_x < b.current_x && a.direction == 1 && b.direction == -1) ||
        (a.current_x > b.current_x && a.direction == -1 && b.direction == 1) ||
        (a.direction == b.direction)) {
            return abs(a.current_x - b.current_x) < 0.6;
    }
    else return false;
    
}

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}


float random_square(float init_x) {
    float t = random()*1.777777;
    if (init_x > 0) {
        t = -t;
    }
    return t;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program;
    ShaderProgram program2;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program2.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    glm::mat4 modelMatrix;
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    float lastFrameTicks = 0.0f;
   
    SDL_Event event;
    bool done = false;
    
    Square a(-0.5, -1, 1);
    Square b(0.5, 1, 1.5);
    
    GLuint car = LoadTexture(RESOURCE_FOLDER"car.png");
    GLuint car1 = LoadTexture(RESOURCE_FOLDER"car1.png");
    GLuint car2 = LoadTexture(RESOURCE_FOLDER"car2.png");
    GLuint car21 = LoadTexture(RESOURCE_FOLDER"car21.png");
    GLuint cloud = LoadTexture(RESOURCE_FOLDER"cloud.png");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float degree = 1.0f;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        
        checkWallHitted(a, 1.777777777f);
        checkWallHitted(b, 1.777777777f);
        
        if (checkCollision(a, b)) {
            if (a.direction == b.direction) {
                if (a.direction == -1) {
                    if (a.current_x > b.current_x) {
                        a.direction = -a.direction;
                    }
                    else {
                        b.direction = -b.direction;
                    }
                }
                if (a.direction == 1) {
                    if (a.current_x < b.current_x) {
                        a.direction = -a.direction;
                    }
                    else {
                        b.direction = -b.direction;
                    }
                }
            }
            float t = a.speed;
            a.speed = b.speed;
            b.speed = t;
            a.direction = -a.direction;
            b.direction = -b.direction;
        };
        
        a.move(elapsed);
        b.move(elapsed);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.3f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 3.0f, 1.0f));
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetModelMatrix(modelMatrix);
        float vertices[12] = {-0.3, -0.25, 0.3, -0.25, 0.3, 0.25, -0.3, -0.25, 0.3, 0.25, -0.3, 0.25};
        float texCoords[12] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glBindTexture(GL_TEXTURE_2D, cloud);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(a.current_x, -0.75f, 0.0f));
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetModelMatrix(modelMatrix);
        if (a.direction == 1) glBindTexture(GL_TEXTURE_2D, car1);
        else glBindTexture(GL_TEXTURE_2D, car);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, a.vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, a.texCoords);
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        if (b.direction == 1) glBindTexture(GL_TEXTURE_2D, car21);
        else glBindTexture(GL_TEXTURE_2D, car2);
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(b.current_x, -0.75f, 0.0f));
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetModelMatrix(modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, b.vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, b.texCoords);
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);


        glUseProgram(program2.programID);
        
        program2.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
        modelMatrix = glm::mat4(1.0f);
        degree += (elapsed * 100) * (PI / 180.f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.0f, 0.1f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, degree, glm::vec3(0.0f, 0.0f, 1.0f));
        program2.SetProjectionMatrix(projectionMatrix);
        program2.SetViewMatrix(viewMatrix);
        program2.SetModelMatrix(modelMatrix);
        float fans[] = {0.0f, 0.0f, 0.0f, -0.1f, 0.2f, -0.1f,
            0.0f, 0.0f, 0.1f, 0.0f, 0.1f, 0.2f,
            0.0f, 0.0f, 0.0f, 0.1f, -0.2f, 0.1f,
            0.0f, 0.0f, -0.1f, 0.0f, -0.1f, -0.2f};
        for (int i = 0; i < 24; i++) {
            fans[i] *= 2.0f;
        }
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, fans);
        glEnableVertexAttribArray(program2.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glDisableVertexAttribArray(program2.positionAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
