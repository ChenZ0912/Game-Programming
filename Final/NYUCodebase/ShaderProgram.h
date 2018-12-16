#pragma once

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL_opengl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "light.h"
//#include "Matrix.h"
#include "glm/mat4x4.hpp"
class ShaderProgram {
    public:
	void Load(const char *vertexShaderFile, const char *fragmentShaderFile);
	void Cleanup();   

//  void SetModelMatrix(const glm::mat4 &matrix[0][0]);
//        void SetProjectionMatrix(const Matrix &matrix);
//        void SetViewMatrix(const Matrix &matrix);
    void SetModelMatrix(const glm::mat4 &matrix);
    void SetProjectionMatrix(const glm::mat4 &matrix);
    void SetViewMatrix(const glm::mat4 &matrix);
    void SetLightPos(light lights[4]);
    
    void SetLightIntensity(float val);
    void SetColor(float r, float g, float b, float a);
	
    GLuint LoadShaderFromString(const std::string &shaderContents, GLenum type);
    GLuint LoadShaderFromFile(const std::string &shaderFile, GLenum type);
    
    GLuint programID;
    
    GLuint projectionMatrixUniform;
    GLuint modelMatrixUniform;
    GLuint viewMatrixUniform;
    GLuint colorUniform;
    GLuint lightPosUniform;
    GLuint lightIntensityUniform;

        GLuint positionAttribute;
        GLuint texCoordAttribute;
    
        GLuint vertexShader;
        GLuint fragmentShader;
};
