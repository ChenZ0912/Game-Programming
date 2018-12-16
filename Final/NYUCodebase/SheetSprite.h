//
//  SheetSprite.h
//  NYUCodebase
//
//  Created by CHEN ZHOU on 12/15/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef SheetSprite_h
#define SheetSprite_h

#include "ShaderProgram.h"


class SheetSprite{
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size);
    
    void Draw(ShaderProgram *program) const;
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};

#endif /* SheetSprite_h */


