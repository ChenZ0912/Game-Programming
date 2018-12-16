#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include "Entity.h"

#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity_untextured.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;

bool collidesWithX(Entity& entity, GameMode& mode, Entity& player){
    float distance_X = fabs(player.position.x - entity.position.x);
    float radius1_X = player.size.x / 2;
    float radius2_X = entity.size.x / 2;
    
    float distance_Y = fabs(player.position.y - entity.position.y);
    float radius1_Y = player.size.y / 2;
    float radius2_Y = entity.size.y / 2;
    
    if(distance_X > radius1_X + radius2_X || distance_Y > radius1_Y + radius2_Y){
        return false;
    }else{
        if(entity.entityType == ENTITY_COIN){
            entity.position.x += -3000.0f;
            mode = STATE_WIN;
            player.position.x = -3.35f;
            player.position.y = -1.0f;
            player.velocity.x = 0.0f;
            player.velocity.y = 0.0f;
            
            entity.velocity.x = 0.0f;
            entity.velocity.y = 0.0f;
            
            entity.position.x -= -2000.0f;
            
        }else if(entity.entityType == ENTITY_STATIC){
            double Xpenetration = fabs(distance_X - radius1_X - radius2_X);
            if(player.position.x > entity.position.x){
                player.position.x += Xpenetration + 0.001f;
                player.collidedLeft = true;
            }else{
                player.position.x -= (Xpenetration + 0.001f);
                player.collidedRight = true;
            }
            player.velocity.x = 0.0f;
        }else if(entity.entityType == ENTITY_ENEMY){
            entity.collidedLeft = true;
            entity.collidedRight = true;
        }
        return true;
    }
}

bool collidesWithY(Entity& entity, GameMode& mode, Entity& player){
    
    float distance_X = fabs(player.position.x - entity.position.x);
    float radius1_X = player.size.x / 2;
    float radius2_X = entity.size.x / 2;
    
    float distance_Y = fabs(player.position.y - entity.position.y);
    float radius1_Y = player.size.y / 2;
    float radius2_Y = entity.size.y / 2;
    
    if(distance_X > radius1_X + radius2_X || distance_Y > radius1_Y + radius2_Y){
        return false;
    }else{
        if(entity.entityType == ENTITY_COIN){
            entity.position.x += 2000.0f;
            mode = STATE_WIN;
            player.position.x = -3.35f;
            player.position.y = -1.0f;
            player.velocity.x = 0.0f;
            player.velocity.y = 0.0f;
            
            
            entity.position.x -= 2000.0f;
        }else if(entity.entityType == ENTITY_STATIC){
            double Ypenetration = fabs(distance_Y - radius2_Y - radius1_Y);
            if(player.position.y > entity.position.y){
                player.position.y += Ypenetration + 0.00001f;
                player.collidedBottom = true;
                entity.collidedTop = true;
            }else{
                player.position.y -= (Ypenetration + 0.00001f);
                player.collidedTop = true;
            }
            player.velocity.y = 0.0f;
            
        }else if(entity.entityType == ENTITY_ENEMY){
            if(player.position.y>entity.position.y){
                player.collidedBottom = true;
                entity.collidedTop = true;
            }else{
                player.collidedTop = true;
                entity.collidedBottom = true;
            }
        }
        return true;
    }
}


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

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

class Particle {
public:
    Particle(float x, float y):position(x, y, 0.0f), velocity(0.0f, 0.2f, 0.0f), size(0.4f, 0.4f, 1.0f){}

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    float lifetime;
};

class ParticleEmitter{
public:
    ParticleEmitter(unsigned int particleCount, float x, float y):
    position(x, y, 0.0f), gravity(-0.02f), maxLifetime(1.5f){
        for(int i=0; i < particleCount; i++){
            Particle newParticle(position.x, position.y);
            newParticle.lifetime = ((float)rand()/(float)RAND_MAX) * maxLifetime;
            newParticle.size.x *= ((float)rand()/(float)RAND_MAX);
            newParticle.size.y *= ((float)rand()/(float)RAND_MAX);
            newParticle.velocity.y *= ((float)rand()/(float)RAND_MAX);
            particles.push_back(newParticle);
        }
    }

    void Update(float elapsed){
        for(int i = 0; i<particles.size(); i++){
            particles[i].position.x += particles[i].velocity.x*elapsed;
            particles[i].position.y += particles[i].velocity.y*elapsed;
            
            particles[i].velocity.y -= gravity * elapsed;
            particles[i].lifetime += elapsed;
            
            if(particles[i].lifetime >= maxLifetime){
                particles[i].lifetime = 0.0f;
                particles[i].position.x = position.x;
                particles[i].position.y = position.y;
                particles[i].velocity.y = 0.2f;
                particles[i].velocity.y *= ((float)rand()/(float)RAND_MAX);
            }
        }
        
    }
    void Render(ShaderProgram* program){
        
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        program->SetViewMatrix(viewMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetProjectionMatrix(projectionMatrix);

        glBlendFunc (GL_SRC_ALPHA, GL_ONE);

        glUseProgram(program->programID);
        
        std::vector<float> vertices;
        std::vector<float> texCoords;

        for(int i = 0; i < particles.size(); i++){
            float m = (particles[i].lifetime/maxLifetime);
            float size = lerp(startSize, endSize, m);

            vertices.insert(vertices.end(), {
                particles[i].position.x - size, particles[i].position.y + size,
                particles[i].position.x - size, particles[i].position.y - size,
                particles[i].position.x + size, particles[i].position.y + size,
                particles[i].position.x + size, particles[i].position.y + size,
                particles[i].position.x - size, particles[i].position.y - size,
                particles[i].position.x + size, particles[i].position.y - size
            });
            texCoords.insert(texCoords.end(), {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
            });
    }
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, fartTexture);
        glDrawArrays(GL_TRIANGLES, 0, int(vertices.size()/2));
        
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
    };
    

    float startSize = 0.4f;
    float endSize = 0.0f;
    glm::vec3 position;
    float gravity;
    GLuint fartTexture = LoadTexture(RESOURCE_FOLDER"fire2.png");

    float maxLifetime;
    std::vector<Particle> particles;
};



void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(text.size()*6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}


class mainMenuState{
public:
    mainMenuState():
    font1(SheetSprite(), -2.08, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font2(SheetSprite(), -0.28f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font3(SheetSprite(), 1.57f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font4(SheetSprite(), -0.8f,-0.8f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    button1(-1.9f, 0.3f, 1.3f, 0.5f),
    button2(-0.1f, 0.3f, 1.3f, 0.5f),
    button3(1.8f, 0.3f, 1.3f, 0.5f),
    button4(-0.1f, -0.8f, 1.7f, 0.55f){
        fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
        menuTexture = LoadTexture(RESOURCE_FOLDER"cover_image.png");
        winTexture = LoadTexture(RESOURCE_FOLDER"success.png");
        loseBGTexture = LoadTexture(RESOURCE_FOLDER"bg_desert.png");
        font1.size.x = 0.2f;
        font1.size.y = 0.2f;
        font2.size.x = 0.2f;
        font2.size.y = 0.2f;
        font3.size.x = 0.2f;
        font3.size.y = 0.2f;
        font4.size.x = 0.2f;
        font4.size.y = 0.2f;
    };
    
    
    GLuint fontTexture;
    GLuint menuTexture;
    GLuint winTexture;
    GLuint loseBGTexture;
    
    Entity font1;
    Entity font2;
    Entity font3;
    Entity font4;
    Entity_Untextured button1;
    Entity_Untextured button2;
    Entity_Untextured button3;
    Entity_Untextured button4;
};

class GameState{
public:
    GameState():fire(0, 0, 0), fire2(0, 0, 0){}
    GameState(GLuint level_num):fire(12, -0.65, -1.9), fire2(20, -0.5, 1.0), level_num(level_num)
    {
        fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
        player_stand = LoadTexture(RESOURCE_FOLDER"alienBlue_stand.png");
        player_jump = LoadTexture(RESOURCE_FOLDER"alienBlue_jump.png");
        boxCoin_disabledTexture = LoadTexture(RESOURCE_FOLDER"boxCoinAlt_disabled.png");
        boxCoinTexture = LoadTexture(RESOURCE_FOLDER"boxCoin.png");
        coinTexture = LoadTexture(RESOURCE_FOLDER"itemSpriteSheet.png");
        
        EasyLevel = LoadTexture(RESOURCE_FOLDER"bg_desert.png");
        MediumLevel = LoadTexture(RESOURCE_FOLDER"bg_grasslands.png");
        HardLevel = LoadTexture(RESOURCE_FOLDER"bg_shroom.png");
        
        robotWalk1 = LoadTexture(RESOURCE_FOLDER"shipGreen_manned.png");
        robotWalk2 = LoadTexture(RESOURCE_FOLDER"shipGreen_manned.png");
        
        
        SheetSprite platformSheet = SheetSprite(boxCoin_disabledTexture,
                                                0, 0,
                                                1, 1,
                                                0.2);
        SheetSprite playerSheet = SheetSprite(player_stand,
                                              0.0f, 0.0f,
                                              1, 1,
                                              0.2);
        SheetSprite coinSheet = SheetSprite(coinTexture,
                                            288.0f/1024.0f, 432.0f/1024.0f,
                                            70.0f/1024.0f, 70.0f/1024.0f,
                                            0.2);
        
        font1 = Entity(SheetSprite(), -1.08-1.5f+0.3f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        font2 = Entity(SheetSprite(), -1.08-1.5f+2.0f+0.1f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        player = Entity(playerSheet, -3.35f, -1.0f, 1.0f/1.3, 1.5f/1.3, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
        coin = Entity(coinSheet, 2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);
        
        robots.push_back(new Entity(playerSheet, -1.5f, -1.5f + 2.0 * 0.2 * 0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY));
        robots.push_back(new Entity(playerSheet, -1.5f+0.9f, -1.5f+0.7f+2.0*0.2*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY));
        robots.push_back(new Entity(playerSheet, -1.5f+2.3f, -1.5f+2.1f+2.0*0.2*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY));
        robots.push_back(new Entity(playerSheet, -1.0f, -1.0+2.0*0.2*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -2.0f, -2.0f, ENTITY_ENEMY));
        robots.push_back(new Entity(playerSheet, -2.0f, -2.0+1.0/1.5*0.5*0.2, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -2.0f, -2.0f, ENTITY_ENEMY));
        
        player.standTextureID = player_stand;
        player.jumpTextureID = player_jump;
        
        for (char i = 1; i <= 11; i++) {
            if (i < 10) {
                char file[80];
                sprintf(file, RESOURCE_FOLDER"p2_walk0%d.png", i);
                GLuint walk = LoadTexture(file);
                player.move.push_back(walk);
            }
            else {
                char file[80];
                sprintf(file, RESOURCE_FOLDER"p2_walk%d.png", i);
                GLuint walk = LoadTexture(file);
                player.move.push_back(walk);
            }
            
        }
        
        int d = 1;
        for (Entity* robot : robots) {
            robot->move.push_back(robotWalk1);
            robot->move.push_back(robotWalk2);
            robot->robotNum = d++;
        }
        font1.size.x = 0.2f;
        font1.size.y = 0.2f;
        font2.size.x = 0.2f;
        font2.size.y = 0.2f;
        
        jumpSound = Mix_LoadWAV("jump1.wav");

        if(level_num == 1){
            
            
            float posX = -1.5f;
            float posY = -1.8f;
            
            for (size_t i = 0; i < 5; i++){
                Entity* newWoodPtr = new Entity(platformSheet, posX, posY, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
                newWoodPtr->boxCoin_disabledTextureID = boxCoin_disabledTexture;
                newWoodPtr->boxCoinTextureID = boxCoinTexture;
                platforms.push_back(newWoodPtr);
                posX += 0.8f;
                posY += 0.7f;
            }
        }else if(level_num == 2){
            
            coin.position.x = -1.3f;
            coin.position.y = 0.25f;
            
            Entity* platform1 = new Entity(platformSheet, -1.0f, -1.3f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform1);
            Entity* platform2 = new Entity(platformSheet, 0.9f, -0.35f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform2);
            Entity* platform3 = new Entity(platformSheet, -1.0f, 1.0f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform3);
            
            for(Entity* platformPtr:platforms){
                platformPtr->boxCoin_disabledTextureID = boxCoin_disabledTexture;
                platformPtr->boxCoinTextureID = boxCoinTexture;
            }
            
        }else if(level_num == 3){
            robots[0]->acceleration.x = -5.0f;
            robots[1]->acceleration.x = -5.0f;
            robots[2]->acceleration.x = -5.0f;
            robots[3]->acceleration.x = -5.0f;
            
            coin.position.x = -3.0f;
            coin.position.y = 1.4f;
            
            Entity* platform1 = new Entity(platformSheet, -1.0f, -1.0f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform1);
            Entity* platform2 = new Entity(platformSheet, 1.1f, -0.8f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform2);
            Entity* platform3 = new Entity(platformSheet, -0.8f, 0.4f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform3);
            Entity* platform4 = new Entity(platformSheet, -2.0f, -1.5f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform4);
            Entity* platform5 = new Entity(platformSheet, 2.0f, 0.0f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            platforms.push_back(platform5);
            
            for(Entity* platformPtr:platforms){
                platformPtr->boxCoin_disabledTextureID = boxCoin_disabledTexture;
                platformPtr->boxCoinTextureID = boxCoinTexture;
            }
        }
    };
    

    GLuint fontTexture;
    GLuint coinTexture;
    GLuint player_stand;
    GLuint player_jump;
    GLuint platformSpriteSheet;
    GLuint boxCoin_disabledTexture;
    GLuint boxCoinTexture;
    
    GLuint walk1;
    GLuint walk2;

    GLuint EasyLevel;
    GLuint MediumLevel;
    GLuint HardLevel;
    
    GLuint robotWalk1;
    GLuint robotWalk2;
    
    int level_num;

    SheetSprite platformSheet;
    SheetSprite playerSheet;
    SheetSprite itemSheet;
    SheetSprite alienSheet;
    

    std::vector<Entity*> platforms;
    std::vector<Entity*> robots;
    Entity player;
    Entity coin;
    
   
    Entity font1;
    Entity font2;
    
    Mix_Chunk* jumpSound;
    
    ParticleEmitter fire;
    ParticleEmitter fire2;
};

void drawBackground(ShaderProgram* program, GLuint textureID, GameState* gameState){
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.55f*2, 2.0f*2, 1.0f));
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    float vertices01[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices01);
    glEnableVertexAttribArray(program->positionAttribute);
    
    float texCoords01[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords01);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

void setup(ShaderProgram* program, ShaderProgram* program_untextured){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Final Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program_untextured->Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    Mix_Music *EasyLevelM;
    EasyLevelM = Mix_LoadMUS("BGM.wav");
    
    Mix_PlayMusic(EasyLevelM, -1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void processEventGame(SDL_Event* event, bool& done, GameState* gameState){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && gameState->player.collidedBottom == true){
                Mix_PlayChannel(-1, gameState->jumpSound, 0);
                if (gameState->level_num == 1) gameState->player.velocity.y = 1.8f;
                else if (gameState->level_num == 2) gameState->player.velocity.y = 2.0f;
                else if (gameState->level_num == 3) gameState->player.velocity.y = 2.1f;
            }
        }
    }
}

void processEventWin(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}

void processEventLose(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}

void processEventMenu(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode, GameState*& gameState){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_MOUSEBUTTONDOWN){
            if(event->button.button == 1){
                float unitX = (((float)event->button.x / 1280.0f) * 3.554f ) - 1.777f;
                float unitY = (((float)(720-event->button.y) / 720.0f) * 2.0f ) - 1.0f;
                
                float button1PosX = menuState->button1.position.x * 0.5;
                float button1PosY = menuState->button1.position.y * 0.5;
                float button1SizeX = menuState->button1.size.x * 0.5;
                float button1SizeY = menuState->button1.size.y * 0.5;

                
                if(unitX>=button1PosX-button1SizeX/2 && unitX<=button1PosX+button1SizeX/2
                   && unitY>=button1PosY-button1SizeY/2 && unitY<=button1PosY+button1SizeY/2){
                    gameState = new GameState(1);
                    gameMode = STATE_GAME;
                }
                
                float button2PosX = menuState->button2.position.x * 0.5;
                float button2PosY = menuState->button2.position.y * 0.5;
                float button2SizeX = menuState->button2.size.x * 0.5;
                float button2SizeY = menuState->button2.size.y * 0.5;

                if(unitX>=button2PosX-button2SizeX/2 && unitX<=button2PosX+button2SizeX/2
                   && unitY>=button2PosY-button2SizeY/2 && unitY<=button2PosY+button2SizeY/2){
                    gameState = new GameState(2);
                    gameMode = STATE_GAME;
                }
                
                float button3PosX = menuState->button3.position.x * 0.5;
                float button3PosY = menuState->button3.position.y * 0.5;
                float button3SizeX = menuState->button3.size.x * 0.5;
                float button3SizeY = menuState->button3.size.y * 0.5;
                
                if(unitX>=button3PosX-button3SizeX/2 && unitX<=button3PosX+button3SizeX/2
                   && unitY>=button3PosY-button3SizeY/2 && unitY<=button3PosY+button3SizeY/2){
                    gameState = new GameState(3);
                    gameMode = STATE_GAME;
                }
                
                float button4PosX = menuState->button4.position.x * 0.5;
                float button4PosY = menuState->button4.position.y * 0.5;
                float button4SizeX = menuState->button4.size.x * 0.5;
                float button4SizeY = menuState->button4.size.y * 0.5;
                
                if(unitX>=button4PosX-button4SizeX/2 && unitX<=button4PosX+button4SizeX/2
                   && unitY>=button4PosY-button4SizeY/2 && unitY<=button4PosY+button4SizeY/2){
                    done = true;
                }
                
                
            }
        }
    }
}

void updateMainMenu(mainMenuState& state, float elapsed){}

void updateBackToMenu(GameMode& mode){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MENU;
    }
}


void updateGameLevel(float elapsed, GameState*& gameState, GameMode& mode){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MENU;
    }
    gameState->player.collidedBottom = false;
    gameState->player.collidedTop = false;
    gameState->player.collidedRight = false;
    gameState->player.collidedLeft = false;
    if (gameState->level_num == 1) {
        gameState->player.updateY(elapsed);
        gameState->robots[0]->updateY(elapsed);
        gameState->robots[1]->updateY(elapsed);
        gameState->robots[2]->updateY(elapsed);
        for (Entity* platformPtr : gameState->platforms){
            collidesWithY(*platformPtr, mode, gameState->player);
            
        }
        collidesWithY(gameState->coin, mode, gameState->player);
        for (int i = 0; i < 3; ++i) {
            collidesWithY(*(gameState->robots[i]), mode, gameState->player);
        }
       
        
        
        gameState->player.updateX(elapsed, mode,
                                  gameState->player,
                                  gameState->platforms,
                                  gameState->level_num);
        
        
        for (int i = 0; i < 3; i++) {
            gameState->robots[i]->updateX(elapsed, mode, gameState->player, gameState->platforms, gameState->level_num);
        }
        
        for (Entity* platformPtr : gameState->platforms){
            collidesWithX(*platformPtr, mode, gameState->player);
        }
        collidesWithX(gameState->coin, mode, gameState->player);
        for (int i = 0; i < 3; ++i) {
                collidesWithX(*(gameState->robots[i]), mode, gameState->player);
        }
        gameState->fire.Update(elapsed);
        gameState->fire2.Update(elapsed);
    }
    
    else if (gameState->level_num == 2) {
        gameState->player.updateY(elapsed);
        for (int i = 3; i < 5; ++i) {
            gameState->robots[i]->updateY(elapsed);
        }
        
       
        for (Entity* platformPtr : gameState->platforms){
            collidesWithY(*platformPtr, mode, gameState->player);
        }
        collidesWithY(gameState->coin, mode, gameState->player);
        for (int i = 3; i < 5; ++i) {
            collidesWithY(*(gameState->robots[i]), mode, gameState->player);
        }
        
        
        gameState->player.updateX(elapsed, mode, gameState->player, gameState->platforms, gameState->level_num);
        for (int i = 3; i < 5; ++i) {
            gameState->robots[i]->updateX(elapsed, mode, gameState->player, gameState->platforms,  gameState->level_num);
        }
        
        
        
        for (Entity* platformPtr : gameState->platforms){
            collidesWithX(*platformPtr, mode, gameState->player);
        }
        collidesWithX(gameState->coin, mode, gameState->player);
        for (int i = 3; i < 5; i++) {
                collidesWithX(*(gameState->robots[i]), mode, gameState->player);
        }
        gameState->fire.Update(elapsed);
        gameState->fire2.Update(elapsed);
    }
    else if (gameState->level_num == 3) {
        gameState->player.updateY(elapsed);
        for (int i = 0; i < 5; i++) {
            if (i != 3) {
                    gameState->robots[i]->updateY(elapsed);
            }
        }
        
        for (Entity* platformPtr : gameState->platforms){
            collidesWithY(*platformPtr, mode, gameState->player);
        }
        collidesWithY(gameState->coin, mode, gameState->player);
        for (int i = 0; i < 5; i++) {
            if (i != 3) {
                collidesWithY(*(gameState->robots[i]), mode, gameState->player);
            }
        }
        
        gameState->player.updateX(elapsed, mode, gameState->player, gameState->platforms, gameState->level_num);
        for (int i = 0; i < 5; i++) {
            if (i != 3) {
                 gameState->robots[i]->updateX(elapsed, mode, gameState->player, gameState->platforms, gameState->level_num);
            }
        }
        
        for (Entity* platformPtr : gameState->platforms){
            collidesWithX(*platformPtr, mode, gameState->player);
        }
        collidesWithX(gameState->coin, mode, gameState->player);
        for (int i = 0; i < 5; i++) {
            if (i != 3) {
                collidesWithX(*(gameState->robots[i]), mode, gameState->player);
            }
        }
    }
    
}


void renderWin(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState){
    light lights[4] = {*new light(-1.8, 0.3f), *new light(0.7f, 0.3f), *new light(-0.7f, 0.3f), *new light(1.6f,0.3)};
    program->SetLightPos(lights);
    program->SetLightIntensity(3.0f);
    drawBackground(program, menuState->winTexture, NULL);
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    DrawText(program, menuState->fontTexture, "YOU'VE WON!!!", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z));
    program->SetModelMatrix(modelMatrix);

    DrawText(program, menuState->fontTexture, "PRESE ESC TO RETURN", menuState->font2.size.x, -menuState->font2.size.x/2.5);
}

void renderLose(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState){
    light lights[4] = {*new light(0.0, 0.3f), *new light(0.7f, -0.8f), *new light(-0.7f, -0.8f), *new light(1000.0f,0.3)};
    program->SetLightPos(lights);
    program->SetLightIntensity(4.0f);
    drawBackground(program, menuState->loseBGTexture, NULL);
    
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.6f, 0.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z));
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    DrawText(program, menuState->fontTexture, "YOU LOST!!!!", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    modelMatrix = glm::mat4(1.0f);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font4.position.x, menuState->font4.position.y, menuState->font4.position.z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, 0.0f, 0.0f));
    program->SetModelMatrix(modelMatrix);
    
    DrawText(program, menuState->fontTexture, "PRESE ESC TO RETURN", menuState->font2.size.x, -menuState->font2.size.x/2.5);
}



void renderMenu(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState, float elapsed){
    light lights[4] = {*new light(-2.08, 0.3f), *new light(-0.28f, 0.3f), *new light(1.57f, 0.3f), *new light(-0.8f,-0.8f)};
    program->SetLightPos(lights);
    program->SetLightIntensity(1.0f);
    drawBackground(program, menuState->menuTexture, NULL);
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font1.position.x, menuState->font1.position.y, menuState->font1.position.z));
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    menuState->button1.Draw(program_untextured);
    menuState->button2.Draw(program_untextured);
    menuState->button3.Draw(program_untextured);
    menuState->button4.Draw(program_untextured);
    
    
    DrawText(program, menuState->fontTexture, "EASY", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z));
    program->SetModelMatrix(modelMatrix);
    
    
    DrawText(program, menuState->fontTexture, "MEDIUM", menuState->font2.size.x, -menuState->font2.size.x/2.5);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font3.position.x, menuState->font3.position.y, menuState->font3.position.z));
    program->SetModelMatrix(modelMatrix);
    DrawText(program, menuState->fontTexture, "HARD", menuState->font3.size.x, -menuState->font3.size.x/2.5);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(menuState->font4.position.x, menuState->font4.position.y, menuState->font4.position.z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.2, 0.0, 0.0));
    program->SetModelMatrix(modelMatrix);
    DrawText(program, menuState->fontTexture, "EXIT GAME", menuState->font4.size.x, -menuState->font4.size.x/2.5);
    
    
}

void renderGame(ShaderProgram* program, GameState* gameState, float elapsed){
    glUseProgram(program->programID);
    
    if (gameState->level_num == 1) {
        light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(gameState->fire.position.x, gameState->fire.position.y), *new light(gameState->fire2.position.x, gameState->fire2.position.y), *new light(1000.0f,-0.8f)};
        program->SetLightPos(lights);
        program->SetLightIntensity(1.0f);
        
        drawBackground(program, gameState->EasyLevel, gameState);
        gameState->player.Render(program, &gameState->player, elapsed);
        for (Entity* platformPtr : gameState->platforms){
            platformPtr->Render(program, &gameState->player, elapsed);
        }
        
        gameState->robots[0]->Render(program, &gameState->player, elapsed);
        gameState->robots[1]->Render(program, &gameState->player, elapsed);
        gameState->robots[2]->Render(program, &gameState->player, elapsed);
        gameState->coin.Render(program, &gameState->player, elapsed);
        
        gameState->fire.Render(program);
        gameState->fire2.Render(program);
    }
    else if (gameState->level_num == 2) {
        light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(gameState->fire2.position.x, gameState->fire2.position.y), *new light(gameState->fire.position.x, gameState->fire.position.y), *new light(1000.0f,-0.8f)};
        program->SetLightPos(lights);
        program->SetLightIntensity(0.8f);
        
        drawBackground(program, gameState->MediumLevel, gameState);
        
        gameState->player.Render(program, &gameState->player, elapsed);
        for (Entity* platformPtr : gameState->platforms){
            platformPtr->Render(program, &gameState->player, elapsed);
        }
        
        gameState->robots[3]->Render(program, &gameState->player, elapsed);
        gameState->robots[4]->Render(program, &gameState->player, elapsed);
        gameState->coin.Render(program, &gameState->player, elapsed);
        gameState->fire.Render(program);
        gameState->fire2.Render(program);
    }
    else if (gameState->level_num == 3) {
        light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(-1000.0f, 0.3f), *new light(1000.0f, 0.3f), *new light(1000.0f,-0.8f)};
        program->SetLightPos(lights);
        program->SetLightIntensity(1.0f);
        drawBackground(program, gameState->HardLevel, gameState);
        gameState->player.Render(program, &gameState->player, elapsed);
        for (Entity* platformPtr : gameState->platforms){
            platformPtr->Render(program, &gameState->player, elapsed);
        }
        
        for (int i = 0; i <=4; i++) {
            if (i != 3) {
                gameState->robots[i]->Render(program, &gameState->player, elapsed);
            }
        }
        gameState->coin.Render(program, &gameState->player, elapsed);
    }
}

void cleanup(){
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    ShaderProgram program;
    ShaderProgram program_untextured;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    setup(&program, &program_untextured);
    

    mainMenuState menuState;
    GameState* gameState;
    GameMode mode = STATE_MENU;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        
        if(mode == STATE_MENU){
            processEventMenu(&event, done, &menuState, mode, gameState);
        }else if(mode == STATE_WIN){
            processEventWin(&event, done, &menuState, mode);
        }else if(mode == STATE_LOSE){
            processEventLose(&event, done, &menuState, mode);
        }else if (mode == STATE_GAME){
            processEventGame(&event, done, gameState);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            if(mode == STATE_MENU){
                updateMainMenu(menuState, FIXED_TIMESTEP);
            }else if(mode == STATE_WIN || mode == STATE_LOSE){
                updateBackToMenu(mode);
            }else {
                updateGameLevel(FIXED_TIMESTEP, gameState, mode);
            }
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        if(mode == STATE_MENU){
            renderMenu(&program, &program_untextured, &menuState, FIXED_TIMESTEP);
        }else if(mode == STATE_WIN){
            renderWin(&program, &program_untextured, &menuState);
        }else if(mode == STATE_LOSE){
            renderLose(&program, &program_untextured, &menuState);
        }else if (mode == STATE_GAME){
            renderGame(&program, gameState, FIXED_TIMESTEP);
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    cleanup();
    return 0;
}
