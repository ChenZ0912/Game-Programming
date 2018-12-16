//
//  Entity_untextured.h
//  NYUCodebase
//
//  Created by CHEN ZHOU on 12/15/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Entity_untextured_h
#define Entity_untextured_h
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Entity_Untextured{
public:
    
    Entity_Untextured(float positionX, float positionY, float sizeX, float sizeY);
    
    void Draw(ShaderProgram* program);
    
    void update(float elapsed);
    
    glm::vec3 position;
    glm::vec3 size;
    glm::mat4 modelMatrix;
};


#endif /* Entity_untextured_h */
