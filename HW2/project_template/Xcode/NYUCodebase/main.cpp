#ifdef _WINDOWS
#include <GL/glew.h>
#endif
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

float otho_length = 1.777f;
float otho_width = 1.0f;
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
ShaderProgram program;
SDL_Window* displayWindow;
SDL_Event event;
bool done;
const Uint8* keys = SDL_GetKeyboardState(NULL);
float lastFrameTicks;
bool gameOver;

class Paddle {
public:
    Paddle() {}
    Paddle(glm::vec2 Position, float width, float length)
    : p(Position), width(width), length(length){}
    GLfloat Width() {
        return width;
    }
    GLfloat Length() {
        return length;
    }
    glm::vec2 getPosition() {
        return p;
    }
    void move(float x) {
        p += glm::vec2{0, x};
        if (p.y + width / 2 >= 1) p = glm::vec2{p.x, 1 - width / 2};
        if (p.y - width / 2 <= -1) p = glm::vec2{p.x, -1 + width / 2};
    }
    void draw(ShaderProgram& program) {
        projectionMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::ortho(-otho_length, otho_length, -otho_width, otho_width, -1.0f, 1.0f);
        modelMatrix = glm::mat4(1.0f);
        viewMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(p.x, p.y, 0));
        program.SetModelMatrix(modelMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        
        float vertices[12] = {-length/2, width/2,
            -length/2, -width/2,
            length/2, -width/2,
            length/2, width/2,
            -length/2, width/2,
            length/2, -width/2
        };
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    }
private:
    glm::vec2 p;
    float width;
    float length;
};

class Square {
public:
    Square(){}
    Square(glm::vec2 Position, glm::vec2 v, float r)
    : Position(Position), Velocity(v), r(r){}
    void Move(float time, float window_width, Paddle& p1, Paddle& p2);
    //    void Reset(glm::vec2 postion, glm::vec2 velocity);
    bool checkCollision(Paddle& p);
    glm::vec2 getPosition() {return Position;}
    void draw(ShaderProgram& program) {
        projectionMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::ortho(-otho_length, otho_length, -otho_width, otho_width, -1.0f, 1.0f);
        modelMatrix = glm::mat4(1.0f);
        viewMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(Position.x, Position.y, 0));
        program.SetViewMatrix(viewMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelMatrix(modelMatrix);
        float x = r;
        float vertices[] = {-x, r,
            -x, -r,
            x, -r,
            x, r,
            -x, r,
            x, -r
        };
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    }
private:
    glm::vec2 Position;
    glm::vec2 Velocity;
    float r;
};

void Square::Move(GLfloat time, float window_width, Paddle& p1, Paddle& p2) {
    this->Position = this->Position + time * this->Velocity;
    if (this->Position.x < 0 && this->Velocity.x < 0 && checkCollision(p1)) {
        this->Velocity.x = -this->Velocity.x;
        this->Position.x = p1.getPosition().x + p1.Length();
    }
    else if (this->Position.x > 0 && this->Velocity.x > 0 &&  checkCollision(p2)) {
        this->Velocity.x = -this->Velocity.x;
        this->Position.x = p2.getPosition().x - p2.Length();
    }
    if (this->Position.y < 0 && this->Position.y < -window_width && this->Velocity.y < 0) {
        this->Velocity.y = -this->Velocity.y;
        this->Position.y = -window_width;
    }
    else if (this->Position.y > 0 && this->Position.y > window_width && this->Velocity.y > 0) {
        this->Velocity.y = -this->Velocity.y;
        this->Position.y = window_width;
    }
    
}
bool Square::checkCollision(Paddle& p) {
    float p_x = p.getPosition().x;
    float p_y = p.getPosition().y;
    float w = p.Width();
    float l = p.Length();
    float x = Position.x;
    float y = Position.y;
    return abs(p_x - x) < (r + l / 2) && abs(p_y - y) < (r + w / 2);
}



Square ball;
Paddle p1;
Paddle p2;

bool checkGameOver(Square& ball) {
    return ball.getPosition().x > otho_length || ball.getPosition().x < -otho_length;
}

void Setup() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1067, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    done = false;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    ball = Square(glm::vec2{0.0, 0.42}, glm::vec2{1.0, 0.5}, 0.05);
    p1 = Paddle(glm::vec2{-otho_length + 0.2, 0.0}, 0.8, 0.1);
    p2 = Paddle(glm::vec2{otho_length - 0.2, 0.0}, 0.8, 0.1);
    lastFrameTicks = 0.0f;
    gameOver = false;
}

void Render() {
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    if (!gameOver) {
        if (keys[SDL_SCANCODE_W]) {
            p1.move(0.02);
        }
        if (keys[SDL_SCANCODE_S]) {
            p1.move(-0.02);
        }
        if (keys[SDL_SCANCODE_UP]) {
            p2.move(0.02);
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            p2.move(-0.02);
        }
        ball.Move(elapsed, 1, p1, p2);
    }
}

void Run() {
    glUseProgram(program.programID);
    glClear(GL_COLOR_BUFFER_BIT);
    ball.draw(program);
    p1.draw(program);
    p2.draw(program);
    if (checkGameOver(ball)) {
        gameOver = true;
    }
}
int main(int argc, char *argv[])
{
    Setup();
    
    while (!done) {
       
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        Render();
        Run();
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}
