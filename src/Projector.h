//
//  Projector.hpp
//  pmSimulator
//
//  Created by MukaiyamaKazushi on 2016/11/24.
//
//

#ifndef Projector_hpp
#define Projector_hpp

#include "cinder/app/App.h"
#include "cinder/Log.h"
#include <stdio.h>

#include "Resources.h"

#endif /* Projector_hpp */

using namespace ci;
using namespace ci::app;
using namespace std;

enum Aspect {
    TYPE_16X9 = 0,
    TYPE_4X3  = 1
};

enum SCREEN {
    TEST_PATTERN,
    RED,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW,
    ORANGE,
    PURPLE,
    BLACK
};

enum HEAD {
    UP = 0,
    DOWN = 1,
    RIGHT = 2,
    LEFT = 3
};

class Projector : public CameraPersp {
    public:
        Projector();
        const string getName();
        void setName(const string n);
        Aspect getAspectType();
        void setAspectType(const Aspect a);
        void setLight(const bool st);
        bool isOn();
        void setScreen(SCREEN scr);
        SCREEN getScreen();
        void setHead(HEAD hd);
        HEAD getHead();
    
        gl::Texture2dRef tex;
        gl::Texture2dRef shadowMapTex;
        gl::FboRef		 fbo;
    
    private:
        string name;
        Aspect aspect;
        SCREEN screen;
        HEAD head;
    
        gl::Texture2dRef texPatternTest;
        gl::Texture2dRef texBlack;
        gl::Texture2dRef texRed;
        gl::Texture2dRef texGreen;
        gl::Texture2dRef texBlue;
        gl::Texture2dRef texCyan;
        gl::Texture2dRef texMagenta;
        gl::Texture2dRef texYellow;
        gl::Texture2dRef texOrnge;
        gl::Texture2dRef texPurple;
};
