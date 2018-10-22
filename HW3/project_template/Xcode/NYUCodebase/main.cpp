#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define LENGTH 1280
#define WIDTH 720
#define INITIAL_X (-3.55f - 0.5 / 2 * 1.5f)
#define INITIAL_Y 1.8f
#define MAX_BULLETS 30

enum GameMode {MENU, RUN, OVER};

SDL_Window* displayWindow;
int bulletIndex = 0;
bool win = false;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

class SheetSprite{
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){ }
    
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
        
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        
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

class Entity{
public:
    Entity() : position(0, 0, 0),  size(0, 0, 0), alive(true) {
    }
    Entity(float positionX, float positionY, float sizeX, float sizeY) :position(positionX, positionY, 0), size(sizeX, sizeY, 0), alive(true)
    {}
    
    void Draw(ShaderProgram& program, const SheetSprite& sprite) const {
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, position.z));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(size.x, size.y, size.z));
        
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        
        glUseProgram(program.programID);
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelMatrix(modelMatrix);
        program.SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    
    void update(float elasped){
        position.x += elasped * 0.2f;
        if(position.x >= 3.55 + size.x / 2 * 0.2){
            position.x = -3.55f - size.x / 2 *0.2;
        }
    }
    
    glm::vec3 position;
    glm::vec3 size;
    float velocityY;
    bool alive;
};

class GameState {
public:
    GameState(float positionX, float positionY, float sizeX, float sizeY) : player(positionX, positionY, sizeX, sizeY), mode(MENU) {}
    Entity player;
    std::vector<Entity> enemies;
    std::vector<Entity> bullets;
    GameMode mode;
};

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

void shootBullet(const Entity& ship, std::vector<Entity>& bullets) {
    bullets[bulletIndex].position.x = ship.position.x;
    bullets[bulletIndex].position.y = ship.position.y;
    bullets[bulletIndex].velocityY = 2.0f;
    bullets[bulletIndex].alive = true;
    bulletIndex = (bulletIndex + 1) % MAX_BULLETS;
}

void ProcessRun(SDL_Event& event, bool& done, std::vector<Entity>& bullets, const Entity& player){
    while (SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }else if (event.type == SDL_KEYDOWN){
            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                shootBullet(player, bullets);
            }
        }
    }
}

void ProcessMenu(SDL_Event& event, bool& done, GameState& game) {
    while (SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        } else if (event.type == SDL_KEYDOWN){
            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                game.mode = RUN;
            }
        }
    }
}
void ProcessOver(SDL_Event& event, bool& done) {
    while (SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }
    }
}
    
void Render(ShaderProgram& program, const std::vector<Entity>& enemies, const SheetSprite& EnemySprite, const Entity& player, const SheetSprite& PlayerSprite, const std::vector<Entity>& bullets, const SheetSprite& bulletSprite ){
    
    for(size_t i = 0; i < enemies.size(); i++){
        if (enemies[i].alive) enemies[i].Draw(program, EnemySprite);
    }
    
    for(size_t i = 0; i < bullets.size(); i++){
        if (bullets[i].alive) bullets[i].Draw(program, bulletSprite);
    }
    
    player.Draw(program, PlayerSprite);
    
}

const bool hitEnemy(const Entity& bullet, const Entity& enemy){
    if(bullet.position.x+bullet.size.x * 0.2 / 2 < enemy.position.x - enemy.size.x * 0.2 / 2 || bullet.position.x-bullet.size.x * 0.2 / 2 > enemy.position.x+enemy.size.x * 0.2 / 2 || bullet.position.y+bullet.size.y * 0.2 / 2 < enemy.position.y-enemy.size.y * 0.2 / 2 || bullet.position.y-bullet.size.y * 0.2 / 2 > enemy.position.y+enemy.size.y * 0.2 / 2) {
        return true;
    }
    return false;
}

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 0.2f, 0.0f));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetModelMatrix(modelMatrix);
    program.SetViewMatrix(viewMatrix);
    float character_size = 1.0/16.0f;
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
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    // draw this data (use the .data() method of std::vector to get pointer to data)
    // draw this yourself, use text.size() * 6 or vertexData.size()/2 to get number of vertices
    
    glUseProgram(program.programID);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void Update(std::vector<Entity>& enemies, Entity& player, std::vector<Entity>& bullets, float elapsed, GameMode& gameMode){
    if(keys[SDL_SCANCODE_LEFT]){
        player.position.x -= elapsed * 1.6f;
        if(player.position.x <= -3.55 + player.size.x / 2 * 0.2){
            player.position.x = -3.55 + player.size.x / 2 * 0.2;
        }
        
    } else if(keys[SDL_SCANCODE_RIGHT]){
        player.position.x += elapsed * 1.6f;
        if(player.position.x >= 3.55-player.size.x / 2 * 0.2){
            player.position.x = 3.55-player.size.x / 2 * 0.2;
        }
    }
    
    for (size_t i=0; i < bullets.size(); i++){
        bullets[i].position.y += bullets[i].velocityY * elapsed;
    }
    
    for(size_t i=0; i < enemies.size(); i++){
        enemies[i].update(elapsed);
    }

    for(size_t i=0; i < bullets.size(); i++){
        if (!bullets[i].alive) continue;
        for(size_t j=0; j < enemies.size(); j++){
            if (!bullets[i].alive || !enemies[j].alive) continue;
            if (!hitEnemy(bullets[i], enemies[j]) && bullets[i].velocityY >= 0) {
                bullets[i].alive = false;
                enemies[j].alive = false;
            }
        }
    }
    
    win = true;
    for(size_t i = 0; i < enemies.size(); i++){
        if (enemies[i].alive) {
            win = false;
            break;
        }
    }
    if (win) {
        gameMode = OVER;
    }
    else {
        gameMode = RUN;
    }
}


void Setup(ShaderProgram& program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LENGTH, WIDTH, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, LENGTH, WIDTH);
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


int main(int argc, char *argv[])
{
    ShaderProgram program;
    Setup(program);
    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    
    //<SubTexture name="enemyBlack1.png" x="423" y="728" width="93" height="84"/>
    SheetSprite EnemySprite = SheetSprite(spriteSheetTexture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
    //"playerShip1_blue.png" x="211" y="941" width="99" height="75"/>
    SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 211/1024.0f, 941/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    //<SubTexture name="laserBlue01.png" x="856" y="421" width="9" height="54"/>
    SheetSprite BulletSprite = SheetSprite(spriteSheetTexture, 856/1024.0f, 421/1024.0f, 9/1024.0f, 54/1024.0f, 0.2);
    
    float lastFrameTicks = 0.0f;
    
    
    SDL_Event event;
    bool done = false;
    GameState game(0.0f, -1.3f, 1.5f, 1.5f);
    float initialX = INITIAL_X;
    float initialY = INITIAL_Y;
    
    for(size_t j = 0; j < 5; j++){
        for(size_t  i = 0; i < 11; i++){
            Entity newEnemy(initialX, initialY, 1.0f, 1.5f);
            initialX+= 0.5 * 1.3f;
            game.enemies.push_back(newEnemy);
        }
        initialX = INITIAL_X;
        initialY -= 0.4f;
    }
    for (size_t i = 0; i < MAX_BULLETS; i++) {
        game.bullets.push_back(Entity(-2000.0f, 0.0f, 1.0f, 1.0f));
    }
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        if (game.mode == MENU) {
            glClear(GL_COLOR_BUFFER_BIT);
            DrawText(program, fontTexture, "Click Space to Start Game", 0.1, 0.05);
            ProcessMenu(event, done, game);
        }
        else if (game.mode == RUN) {
            ProcessRun(event, done, game.bullets, game.player);
            glClear(GL_COLOR_BUFFER_BIT);
            Update(game.enemies, game.player, game.bullets, elapsed, game.mode);
            Render(program, game.enemies, EnemySprite, game.player, playerSprite, game.bullets, BulletSprite);
        }
        else if (game.mode == OVER) {
            glClear(GL_COLOR_BUFFER_BIT);
            DrawText(program, fontTexture, "Game Over", 0.3, 0.05);
            ProcessOver(event, done);
        }
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}

