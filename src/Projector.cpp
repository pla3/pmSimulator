//
//  Projector.cpp
//  pmSimulator
//
//  Created by MukaiyamaKazushi on 2016/11/24.
//
//

#include "Projector.h"

Projector::Projector(){
    setAspectType(Aspect::TYPE_16X9);
    setName("unnamed");
    setLight(false);
    setHead(HEAD::UP);
}

const string Projector::getName(){
    return name;
}

void Projector::setName(const string n){
    name = n;
}

Aspect Projector::getAspectType(){
    return aspect;
}

void Projector::setAspectType(const Aspect a){
    aspect = a;
    
    try {
        if(aspect==Aspect::TYPE_16X9){
            texPatternTest = gl::Texture2d::create( loadImage(loadResource(RES_TESTPATTERN_16X9)), gl::Texture::Format().mipmap() );
        } else if(aspect==Aspect::TYPE_4X3){
            texPatternTest = gl::Texture2d::create( loadImage(loadResource(RES_TESTPATTERN_4X3)), gl::Texture::Format().mipmap() );
        }
        //texBlack = gl::Texture2d::create( loadImage(loadResource(RES_BLACK_16X9)), gl::Texture::Format().mipmap());
        texBlack = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_BLACK)));
        texRed = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_RED)));
        texGreen = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_GREEN)));
        texBlue = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_BLUE)));
        texCyan = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_CYAN)));
        texMagenta = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_MAGENTA)));
        texYellow = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_YELLOW)));
        texOrnge = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_ORANGE)));
        texPurple = gl::Texture2d::create(loadImage(loadResource(RES_COLOR_PURPLE)));
    } catch ( Exception &exc ) {
        CI_LOG_EXCEPTION( "texture image load failed", exc );
        std::terminate();
    }
    
    float width = texPatternTest->getWidth();
    float height = texPatternTest->getHeight();

    setAspectRatio(width/height);
    
    // shadow maps and their buffers
    gl::Texture2d::Format depthFormat;
    depthFormat.setInternalFormat( GL_DEPTH_COMPONENT32F );
    depthFormat.setCompareMode( GL_COMPARE_REF_TO_TEXTURE );
    depthFormat.setMagFilter( GL_LINEAR );
    depthFormat.setMinFilter( GL_LINEAR );
    depthFormat.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
    depthFormat.setCompareFunc( GL_LEQUAL );
    shadowMapTex = gl::Texture2d::create( width, height, depthFormat );
    gl::Fbo::Format fboFormat;
    fboFormat.attachment( GL_DEPTH_ATTACHMENT, shadowMapTex );
    fbo = gl::Fbo::create( width, height, fboFormat );
}

SCREEN Projector::getScreen(){
    return screen;
}

void Projector::setScreen(SCREEN scr){
    screen = scr;
    switch(screen){
        case SCREEN::TEST_PATTERN:
            tex = texPatternTest;
            break;
        case SCREEN::BLACK:
            tex = texBlack;
            break;
        case SCREEN::RED:
            tex = texRed;
            break;
        case SCREEN::GREEN:
            tex = texGreen;
            break;
        case SCREEN::BLUE:
            tex = texBlue;
            break;
        case SCREEN::CYAN:
            tex = texCyan;
            break;
        case SCREEN::MAGENTA:
            tex = texMagenta;
            break;
        case SCREEN::YELLOW:
            tex = texYellow;
            break;
        case SCREEN::ORANGE:
            tex = texOrnge;
            break;
        case SCREEN::PURPLE:
            tex = texPurple;
            break;
        default:
            break;
    }
}

HEAD Projector::getHead(){
    return head;
}

void Projector::setHead(HEAD hd){
    head = hd;
    switch(head){
        case HEAD::UP:
            setWorldUp(vec3(0,1,0));
            break;
        case HEAD::DOWN:
            setWorldUp(vec3(0,-1,0));
            break;
        case HEAD::RIGHT:
            setWorldUp(vec3(1,0,0));
            break;
        case HEAD::LEFT:
            setWorldUp(vec3(-1,0,0));
            break;
        default:
            break;
    }
}

void Projector::setLight(const bool st){
    if(st){
        setScreen(SCREEN::TEST_PATTERN);
    } else {
        setScreen(SCREEN::BLACK);
    }
}

bool Projector::isOn(){
    if(screen == SCREEN::BLACK){
        return false;
    } else {
        return true;
    }
}
