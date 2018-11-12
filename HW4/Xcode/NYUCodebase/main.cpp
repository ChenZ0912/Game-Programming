#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// 60 FPS (1.0f/60.0f) (update sixty times a second)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

enum EntityType{ENTITY_PLAYER, ENTITY_STATIC, ENTITY_MONSTER, ENTITY_FLAG};

class SheetSprite{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){}
    
    void Draw(ShaderProgram &program) const {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        float vertices[] = {
            -0.5f , -0.5f,
            0.5f, 0.5f ,
            -0.5f , 0.5f ,
            0.5f, 0.5f ,
            -0.5f, -0.5f  ,
            0.5f , -0.5f };
        
        glUseProgram(program.programID);
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    
    
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
    
};

class Entity {
public:
    Entity(const SheetSprite& sheet, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType);
    void UpdateX(float elapsed);
    void UpdateY(float elapsed);
    void Render(ShaderProgram& program, Entity& player);
    bool collisionX(Entity& entity);
    bool collisionY(Entity& entity);
    void jump();
    void init_Col();
    SheetSprite sheet;
    
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    
    EntityType entityType;
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    bool draw;
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

Entity::Entity(const SheetSprite& sheet, float pX, float pY, float sX, float sY, float vX, float vY, float aX, float aY, EntityType entityType) :
sheet(sheet), position(pX, pY, 0.0f), size(sX * sheet.size * sheet.width/sheet.height, sY * sheet.size, 0.0f), velocity(vX, vY, 0.0f), acceleration(aX, aY, 0.0f), entityType(entityType), draw(true){};

void Entity::UpdateX(float elapsed){
    if (draw) {
        if(entityType == ENTITY_PLAYER){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            acceleration.x = 0.0f;
            if(keys[SDL_SCANCODE_RIGHT]){
                acceleration.x = 3.0f;
            }else if(keys[SDL_SCANCODE_LEFT]){
                acceleration.x = -3.0f;
            }
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 2.0f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
        }
    }
}

void Entity::UpdateY(float elapsed) {
    if (draw) {
    if (entityType == ENTITY_MONSTER) {
        velocity.y += acceleration.y * elapsed;
        position.y += velocity.y * elapsed;
        
        if (position.y > 2.8 || position.y < -2.8) {
            acceleration = -acceleration;
        }
    }
    else if(entityType == ENTITY_PLAYER){
        acceleration.y = -2.0f;
        
        velocity.y += acceleration.y * elapsed;
        position.y += velocity.y * elapsed;
        
        
        // if it's lower than the window
        if(position.y <= -2.0f + size.y * 0.5){
            position.y = -2.0f + size.y * 0.5;
            velocity.y = 0;
            collidedBottom = true;
        }
    }
    }
}

void Entity::Render(ShaderProgram& program, Entity& player){
    if (player.draw) {
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, size);
        
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        glm::vec3 v = player.position;
        v.z = 0;
        v.x = -1 * player.position.x;
        v.y = -1 * player.position.y;
        viewMatrix = glm::translate(viewMatrix, v);
        
        glUseProgram(program.programID);
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelMatrix(modelMatrix);
        program.SetViewMatrix(viewMatrix);
        sheet.Draw(program);
    }
}

void Entity::jump() {
    if (collidedBottom) {
        velocity.y = 3.0f;
    }
}

void Entity:: init_Col() {
    collidedBottom = false;
    collidedTop = false;
    collidedRight = false;
    collidedLeft = false;
}

bool Entity::collisionX(Entity& entity){
    float distance_X = fabs(position.x - entity.position.x);
    float radius1_X = size.x / 2;
    float radius2_X = entity.size.x / 2;
    
    float distance_Y = fabs(position.y - entity.position.y);
    float radius1_Y = size.y / 2;
    float radius2_Y = entity.size.y / 2;
    if(distance_X > radius1_X + radius2_X || distance_Y > radius1_Y + radius2_Y){
        return false;
    }else{
        if(entity.entityType == ENTITY_STATIC){
            double Xpenetration = fabs(distance_X - radius1_X - radius2_X);
            if(position.x > entity.position.x){
                position.x = position.x + Xpenetration + 0.001f;
                collidedLeft = true;
            }else{
                position.x = position.x - Xpenetration - 0.001f;
                collidedRight = true;
            }
            
            velocity.x = 0.0f;
        }
        return true;
    }
}

bool Entity::collisionY(Entity& entity){
    float distance_X = fabs(position.x - entity.position.x);
    float radius1_X = size.x / 2;
    float radius2_X = entity.size.x / 2;
    
    float distance_Y = fabs(position.y - entity.position.y);
    float radius1_Y = size.y / 2;
    float radius2_Y = entity.size.y / 2;
    
    if(distance_X > radius1_X + radius2_X || distance_Y > radius1_Y + radius2_Y){
        return false;
    }else{
        if(entity.entityType == ENTITY_STATIC){
            double Ypenetration = fabs(distance_Y - radius2_Y - radius1_Y);
            if(position.y > entity.position.y){
                position.y = position.y + Ypenetration + 0.001f;
                collidedBottom = true;
            }else{
                position.y = position.y - Ypenetration - 0.001f;
                collidedTop = true;
            }
            velocity.y = 0.0f;
        }
        if(entity.entityType == ENTITY_MONSTER) {
            double Ypenetration = fabs(distance_Y - radius2_Y - radius1_Y);
            if(position.y > entity.position.y){
                position.y = position.y + Ypenetration + 0.001f;
                collidedBottom = true;
                jump();
                entity.draw = false;
            }else{
                position.y = position.y - Ypenetration - 0.001f;
                collidedTop = true;
                velocity.y = 0.0f;
            }
        }
        return true;
    }
}


SDL_Window* displayWindow;

GLuint LoadTexture(const char* filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is corret\n";
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

void Setup(ShaderProgram& program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Run(SDL_Event* event, bool& done, Entity& player){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE){
                player.jump();
            }
        }
    }
}

void Update(float elapsed, Entity& player, vector<Entity> platforms){
    player.init_Col();
    player.UpdateX(elapsed);
    for (Entity& w : platforms){
        if (w.draw) {
            player.collisionX(w);
        }
    }
    
    player.UpdateY(elapsed);
    for (Entity& w : platforms){
        if (w.draw) {
            w.UpdateY(elapsed);
            player.collisionY(w);
        }
        
    }
}

void Render(ShaderProgram& program, Entity& player, vector<Entity> platforms){
    player.Render(program, player);
    for (Entity& w : platforms){
        if (w.draw) {
            w.Render(program, player);
        }
    }
}


int main(int argc, char *argv[])
{
    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    Setup(program);
    
    GLuint planeSheet = LoadTexture(RESOURCE_FOLDER"planes.png");
    GLuint platformSpriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint monsterSpriteSheet = LoadTexture(RESOURCE_FOLDER"spritesheet_aliens.png");
    GLuint tileSheet = LoadTexture(RESOURCE_FOLDER"spritesheet_tiles.png");
    
    SheetSprite playerSheet = SheetSprite(planeSheet, 0.0f/256.0f, 73.0f/512.0f, 88.0f/256.0f, 73.0f/512.0f, 0.2);
    SheetSprite platformSheet = SheetSprite(platformSpriteSheet, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.2);
    SheetSprite monsterSheet = SheetSprite(monsterSpriteSheet, 140.0f/512.0f, 0.0f, 70.0f/512.0f, 70.0f/256.0f, 0.2);
    SheetSprite flagSheet = SheetSprite(tileSheet, 350.0f/516.0f, 70.0f/256.0f, 70.0f/516.0f, 70.0f/ 256.0f, 0.2);
    
    Entity player(playerSheet, -2.0f, -1.0f, 1.5f, 2.0f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
    
    vector<Entity> platforms;
    float posX = -1.5;
    float posY = -1.0;
    for (size_t i = 0; i < 10; i++){
        if (i == 3) {
            Entity monster(monsterSheet, posX, posY, 4.0f, 2.0f, 0.0f, 1.0f, 0.0f, 5.0f, ENTITY_MONSTER);
            platforms.push_back(monster);
            posX += monster.size.x + 0.5;
            posY += monster.size.y * 2;
        }
        else {
            Entity ground(platformSheet, posX, posY, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(ground);
            posX += ground.size.x + 0.1;
            posY += ground.size.y * 2;
        }
    }
    
    for (size_t i = 0; i < 10; i++) {
        Entity ground(platformSheet, posX, posY, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        platforms.push_back(ground);
        posX += ground.size.x + 0.5;
        posY -= ground.size.y * 2;
    }
    
    Entity flag(flagSheet, posX + 0.3, -1.0f, 1.5f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_FLAG);
    platforms.push_back(flag);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
            
        }
        Run(&event, done, player);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP, player, platforms);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        
        Render(program, player, platforms);
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
