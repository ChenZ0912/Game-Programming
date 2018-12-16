//
//  Entity.h
//  NYUCodebase
//
//  Created by CHEN ZHOU on 12/15/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Entity_h
#define Entity_h

#include "SheetSprite.h"
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "SDL_mixer.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"



enum GameMode {STATE_MENU, STATE_GAME, STATE_GAME_OVER, STATE_WIN, STATE_LOSE};

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_STATIC};

class Entity{
public:
    Entity();
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType);
    
    void updateX(float elapsed, GameMode& mode, Entity& player, std::vector<Entity*>, int level);
    
    void updateY(float elapsed);
    
    void Render(ShaderProgram* program, Entity* player, float elapsed);
    
    void draw(ShaderProgram* program, GLuint textureID);
    
    void animate(ShaderProgram* program, float elapsed);
    
   
    SheetSprite sprite;
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::mat4 modelMatrix;
    
    float animationElapsed;
    int index;
    
    EntityType entityType;
    GLuint standTextureID;
    GLuint jumpTextureID;
    GLuint uniformSpriteSheetTextureID;
    GLuint boxCoin_disabledTextureID;
    GLuint boxCoinTextureID;
    std::vector<GLuint> move;
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    bool turnedBack;
    int robotNum;
    Mix_Chunk* screamSound;
    Mix_Chunk* lolSound;
    
};



#endif /* Entity_h */
