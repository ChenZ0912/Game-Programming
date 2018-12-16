//
//  Entity_untextured.cpp
//  NYUCodebase
//
//  Created by CHEN ZHOU on 12/15/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity_untextured.h"

float easeIn(float from, float to, float time) {
    float tVal = time*time*time*time*time;
    return (1.0f-tVal)*from + tVal*to;
}


float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin)/(srcMax-srcMin) * (dstMax-dstMin));
    if(retVal < dstMin) {
        retVal = dstMin;
    }
    if(retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}

Entity_Untextured::Entity_Untextured(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};

void Entity_Untextured::Draw(ShaderProgram* program){
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, position.z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(size.x, size.y, size.z));
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    program->SetColor(0.0f, 0.0f, 0.0f, 0.3f);
    glUseProgram(program->programID);
    
    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);

    modelMatrix = glm::mat4(1.0f);
}
void Entity_Untextured::update(float elapsed){}


    

