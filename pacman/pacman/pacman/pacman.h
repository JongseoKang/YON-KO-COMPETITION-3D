#ifndef _PACMAN_H_

#define _PACMAN_H

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>


#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <Model.h>

// GL includes

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAP_SIZE 16
#define EMPTY '0'
#define WALL '1'
#define YONSEI '2'
#define KOREA '3'
#define COIN '4'

enum CubeSide {
    Front, Behind, Right, Left, Top, Bottom
};

enum Stadium {
    BASKETBALL = 0, FOOTBALL, HOCKEY, RUGBY, BASEBALL, ENDING
};

enum Input {
    R = 0, U, L, D, NOTHING
};


class Korea{
public:

    unsigned int xPos, yPos;
    Input direction;

    Korea(){};

    Korea(unsigned int x, unsigned int y, unsigned int sx, unsigned int sy,
        unsigned int ex, unsigned int ey, Input dir){
        xPos = x;
        yPos = y;
        sXPos = sx;
        sYPos = sy;
        eXPos = ex;
        eYPos = ey;
        direction = dir;
    }

    void cycle(){
        if((xPos == sXPos && yPos == sYPos) || (xPos == eXPos && yPos == eYPos)){
            if(direction == R) direction = L;
            else if(direction == L) direction = R;
            else if(direction == U) direction = D;
            else direction = U;
        }
        xPos += routine[direction][0];
        yPos += routine[direction][1];
    }
    
    int wallCheck(){
        return (xPos == sXPos && yPos == sYPos) || (xPos == eXPos && yPos == eYPos);
    }
private:
    unsigned int sXPos, sYPos, eXPos, eYPos;
    int routine[4][2] = {{0, 1}, {-1, 0}, {0, -1}, {1, 0}};
};


class Yonsei{
public:

    int xPos, yPos;

    Input dir;

    Yonsei(){};

    Yonsei(int x, int y){
        xPos = x;
        yPos = y;
        dir = D;
    }

private:
};

class Map{
public:

    Model *coin;
    Yonsei yonseiMan;
    std::vector<Korea> koreaMans;
    CubeSide mapSide; 


    Map(){};

    Map(char *initStates, int stad, CubeSide side, std::vector<Korea> korMansData){
        stadium = stad;
        mapSide = side;
        numCoins = 0;

        memcpy(state, initStates, MAP_SIZE * MAP_SIZE);

        for(int i = 0; i < MAP_SIZE; i++)
            for(int j = 0; j < MAP_SIZE; j++)
                if(state[i * MAP_SIZE + j] == YONSEI) yonseiMan = Yonsei(i, j);
        
        for(int i = 0; i < MAP_SIZE; i++)
            for(int j = 0; j < MAP_SIZE; j++)
                if(state[i * MAP_SIZE + j] == COIN) numCoins++;

        koreaMans = korMansData;   


    }


    void render(Shader *shader){
        for(int i = 0; i < MAP_SIZE; i++){
            for(int j = 0; j < MAP_SIZE; j++){
                if(state[i * MAP_SIZE + j] == COIN){
                    float x = 7.5f - i;
                    float y = -7.5f + j;
                
                    glm::mat4 model(1.0f);
                    
                    if(mapSide == Top){
                        // do nothing (ending page)
                    }
                    else if(mapSide == Bottom){
                        model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.0f, 1.f, 0.f));
                        model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 0.f, 1.f));
                    }
                    else if(mapSide == Right){
                        model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
                        model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.f, 0.f, 1.f));
                    }
                    else if(mapSide == Left){
                        model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
                    }
                    else if(mapSide == Front){
                        model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
                    }
                    else if(mapSide == Behind){
                        model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
                        model = glm::rotate(model, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
                    }
                    
                    model = glm::translate(model, glm::vec3(y, 0.f, -x));
                    model = glm::translate(model, glm::vec3(0.0f, 8.5f, 0.0f));
                    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

                    shader->use();
                    shader->setMat4("model", model);
                    coin->Draw(*shader);
                }
            }
        }



    }


    void cycle(){
        for(auto koreaMan = koreaMans.begin(); koreaMan != koreaMans.end(); koreaMan++){
            state[koreaMan->xPos * MAP_SIZE + koreaMan->yPos] = EMPTY;
            koreaMan->cycle();
            state[koreaMan->xPos * MAP_SIZE + koreaMan->yPos] = KOREA;
        }

        if (action == R){
            if(yonseiMan.yPos + 1 < MAP_SIZE && state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos + 1] != WALL){
                state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = EMPTY;
                yonseiMan.yPos++;
                yonseiMan.dir = R;
            }
            else action = NOTHING;
        }
        else if(action == U){
            if(yonseiMan.xPos - 1 >= 0 && state[(yonseiMan.xPos - 1) * MAP_SIZE + yonseiMan.yPos] != WALL){
                state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = EMPTY;
                yonseiMan.xPos--;
                yonseiMan.dir = U;
            }
            else action = NOTHING;
        }
        else if(action == D){
            if(yonseiMan.xPos + 1 < MAP_SIZE && state[(yonseiMan.xPos + 1) * MAP_SIZE + yonseiMan.yPos] != WALL){
                state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = EMPTY;
                yonseiMan.xPos++;
                yonseiMan.dir = D;
            }
            else action = NOTHING;
        }
        else if(action == L){
            if(yonseiMan.yPos - 1 >= 0 && state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos - 1] != WALL){
                state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = EMPTY;
                yonseiMan.yPos--;
                yonseiMan.dir = L;
            }
            else action = NOTHING;
        }

        actionbuf = action;
        action = NOTHING;

        if(state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] == COIN){
            numCoins--;
            state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = YONSEI;
            if(numCoins == 0) gameOver = 2;
        }
        else if(state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] == EMPTY){
            state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] = YONSEI;
        }
        else if(state[yonseiMan.xPos * MAP_SIZE + yonseiMan.yPos] == KOREA){
            // koreaman 에게 잡힘
            gameOver = 1;
        }
    }

    void setAction(Input key){
        action = key;
    }

    Input getAction(){
        return actionbuf;
    }

    unsigned int isGameOver(){
        return gameOver;
    }


private:
    char state[MAP_SIZE * MAP_SIZE];

    unsigned int stadium, numCoins, gameOver;
    Input action, actionbuf;
};




class PacMan{

public:

    Model *cube;
    Map maps[6];
    unsigned int currMap;

    PacMan(){};

    PacMan(char sixStates[][MAP_SIZE * MAP_SIZE], std::vector<Korea>& basketball, std::vector<Korea>& football, std::vector<Korea>& hockey,
        std::vector<Korea>& rugby, std::vector<Korea>& baseball, std::vector<Korea>& ending, std::string dataPath){
        dataDir = dataPath;
        currMap = BASKETBALL;
        
        action = NOTHING;


        maps[BASEBALL] = Map(sixStates[BASEBALL], BASEBALL, Left, baseball);
        maps[FOOTBALL] = Map(sixStates[FOOTBALL], FOOTBALL, Right, football);
        maps[RUGBY] = Map(sixStates[RUGBY], RUGBY, Behind, rugby);
        maps[BASKETBALL] = Map(sixStates[BASKETBALL], BASKETBALL, Front, basketball);
        maps[HOCKEY] = Map(sixStates[HOCKEY], HOCKEY, Bottom, hockey);
        maps[ENDING] = Map(sixStates[ENDING], ENDING, Top, ending);
 
        cube = new Model(dataDir + "/CUBE_MAP/CUBE_MAP.obj");
        maps[BASEBALL].coin = new Model(dataDir + "/balls/baseball.obj");
        maps[FOOTBALL].coin = new Model(dataDir + "/balls/soccer.obj");
        maps[RUGBY].coin = new Model(dataDir + "/Rugby/rugby.obj");
        maps[BASKETBALL].coin = new Model(dataDir + "/balls/basketball.obj");
        maps[HOCKEY].coin = new Model(dataDir + "/Hockey/hockey puk.obj");
        maps[ENDING].coin = new Model(dataDir + "/Hockey/hockey puk.obj");
    };

    int cycle(){
        maps[currMap].cycle();

        if(maps[currMap].isGameOver() == 2){
            currMap++;
            return 2;
        }
        else if(maps[currMap].isGameOver() == 1){
            // game over
            delete cube;
            
            cube = new Model(dataDir + "/gameover/untitled.obj");
            
            return 1;
        }
        
        return 0;
    };


    void render(Shader *shader){
        glm::mat4 model(1.0f);
        shader->use();
        model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.0f, 1.f, 0.f));
        shader->setMat4("model", model);
        cube->Draw(*shader);
    }


private:
    Input action;
    std::string dataDir;
};

#endif
