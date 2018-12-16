//
//  Entity.cpp
//  NYUCodebase
//
//  Created by CHEN ZHOU on 12/15/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity.h"


float lerp(float v0, float v1, float t);


Entity::Entity(){};

Entity::Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX * sprite.size * sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType), collidedTop(false), collidedBottom(false), collidedLeft(false),
collidedRight(false), turnedBack(false), animationElapsed(0.0f), index(0){
    
    screamSound = Mix_LoadWAV("scream.wav");
    lolSound = Mix_LoadWAV("lol.wav");
    
};

void Entity::updateX(float elapsed, GameMode& mode, Entity& player, std::vector<Entity*> platforms, int level){
    collidedRight = false;
    collidedLeft = false;
    
    if(entityType == ENTITY_PLAYER){
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        if(keys[SDL_SCANCODE_RIGHT]){
            acceleration.x = 1.5f;
        }else if(keys[SDL_SCANCODE_LEFT]){
            acceleration.x = -1.5f;
        }else{
            acceleration.x = 0.0f;
        }
        
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 0.8f);
        velocity.x += acceleration.x * elapsed;
        position.x += velocity.x * elapsed;
        
    }
    if(entityType == ENTITY_ENEMY){
        if(collidedTop || collidedBottom || collidedLeft || collidedRight){
            acceleration.x = 20.0f;
            Mix_PlayChannel(-1, screamSound, 0);
            mode = STATE_LOSE;
            player.position.x = -3.35f;
            player.position.y = -1.0f;
            player.velocity.x = 0.0f;
            player.velocity.y = 0.0f;
            
            velocity.x = 0.0f;
            acceleration.x = -3.0f;
            
        }else{
            bool detected = false;
            if(pow(pow((player.position.x-position.x),2)+pow((player.position.y-position.y),2), 0.5) < 0.8f){
                detected = true;
            }
            Entity* platform = platforms[(robotNum - 1) % platforms.size()];
            if (position.x >= platform->position.x + platform->size.x * 0.5) {
                turnedBack = false;
                acceleration.x = -1.0f;
                if(level == 3){
                    acceleration.x = -2.0f;
                }
            }
            else if (position.x <= platform->position.x - platform->size.x * 0.5){
                turnedBack = true;
                acceleration.x = 1.0f;
                if(level == 3){
                    acceleration.x = 2.0f;
                }
            }
            if(detected){
                acceleration.x *= 1.125f;
            }
            detected = false;

        }
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 10.0f);
        velocity.x += acceleration.x * elapsed;
        position.x += velocity.x * elapsed;
    }
}

void Entity::updateY(float elapsed){
    collidedBottom = false;
    collidedTop = false;

    
    if(entityType == ENTITY_PLAYER){
        if(position.x-size.x*0.5 == -2.0f){
            collidedBottom = true;
        }
        acceleration.y = -2.0f;
        
        velocity.y += acceleration.y * elapsed;
        position.y += velocity.y * elapsed;
        
        if(position.y <= -2.0f+size.y*0.5){
            position.y = -2.0f+size.y*0.5;
            collidedBottom = true;
            velocity.y = 0.0f;
        }
        
    }else if(entityType == ENTITY_ENEMY){
        if(collidedTop){
            acceleration.y = -2.0f;
            
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            
            if(position.y <= -2.0f+size.y*0.5){
                position.y = -2.0f+size.y*0.5;
                collidedBottom = true;
            }
        }
    }
    
}


void Entity::Render(ShaderProgram* program, Entity* player, float elapsed){
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, position.z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(size.x, size.y, size.z));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    glUseProgram(program->programID);
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    
    
    
    program->SetViewMatrix(viewMatrix);
    
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(entityType == ENTITY_PLAYER){
        if(collidedBottom == true){
            if(keys[SDL_SCANCODE_RIGHT]){
                animate(program, elapsed);
            }else if(keys[SDL_SCANCODE_LEFT]){
//                modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(-1.0f, 1.0f, 1.0f));
                program->SetModelMatrix(modelMatrix);
                animate(program, elapsed);
            }else{
                draw(program, standTextureID);
            }
        }else{
            draw(program, jumpTextureID);
        }
    }else if(entityType == ENTITY_STATIC){
        if(collidedTop == true){
            draw(program, boxCoin_disabledTextureID);
            collidedTop = false;
        }else{
            draw(program, boxCoinTextureID);
        }
    }else if (entityType == ENTITY_COIN){
        sprite.Draw(program);
    }else if (entityType == ENTITY_ENEMY){
//            modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(-1.0f, 1.0f, 1.0f));
            if(turnedBack){
//                modelMatrix.Scale(1.0f, 1.0f, 1.0f);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
            }
        
            program->SetModelMatrix(modelMatrix);
            animate(program, elapsed);
    }
}

void Entity::draw(ShaderProgram* program, GLuint textureID){
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

void Entity::animate(ShaderProgram* program, float elapsed){
    if(entityType == ENTITY_PLAYER){
        const int numFrames = 2;
        float framesPerSecond = 10.0f;
        
        animationElapsed += elapsed;
        
        if(animationElapsed >= 1.0/framesPerSecond){
            index++;
            animationElapsed = 0.0;
            
            if(index > numFrames-1){
                index = 0;
            }
        }
        draw(program, move[index]);
    }else if(entityType == ENTITY_ENEMY){
        const int numFrames = 2;
        float framesPerSecond = 5.0f;
        
        animationElapsed += elapsed;
        
        if(animationElapsed >= 1.0/framesPerSecond){
            index++;
            animationElapsed = 0.0;
            
            if(index > numFrames - 1){
                index = 0;
            }
        }
        draw(program, move[index]);
    }
}

