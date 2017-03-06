//
//  Projection Mapping Simulator
//
//  Created by Kazushi Mukaiyama on 2016/11/24.
//  Future University Hakodate - http://www.kazushi.info/
//
//  Refer to the follows;
//  Basic implementation of shadow mapping
//  Keith Butters - 2014 - http://www.keithbutters.com

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/CameraUi.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/ObjLoader.h"
#include "cinder/Utilities.h"

#include "Resources.h"
#include "Projector.h"

using namespace ci;
using namespace ci::app;
using namespace std;

enum ViewMode{
    MAIN,
    CONFIGURE
};

class pmSimulator : public App {
  public:
    pmSimulator();
	static void prepareSettings( Settings *settings );
	void setup() override;
	void resize() override;
	void update() override;
	void draw() override;
    
    void mouseDrag( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;

  private:
    ViewMode mode;
    params::InterfaceGlRef	mParamsMode;
    vector<string>			mEnumNamesMode;
    int						mEnumSelectionMode;
    
    void setMenu();
    
    void drawMainView();
    void drawConfigureeView();
    
    void loadObj( const DataSourceRef &dataSource );
    void loadObjButton();
    
    void loadProjectorConf(int id);
    void loadConfButton();
    
    void loadPlan( const DataSourceRef &dataSource );
    void loadPlanButton();
    void savePlan();
    
    void setProjPosition(vec3 pos);
    vec3 getProjPosition();

    fs::path planPath;
    string objName;
    
    CameraPersp*		mCam;
    CameraUi			mCamUi;
    ivec2               mMousePos; //! Keep track of the mouse.
    
    CameraPersp         mPerspectiveCam;
    Projector*          mProjector;
    int                 proj_num;
    bool*               proj_on;
	
	gl::GlslProgRef		mGlsl;
    
	gl::BatchRef		mObjBatch;
	gl::BatchRef		mObjShadowedBatch;
    
	gl::BatchRef		mFloorBatch;
	gl::BatchRef		mFloorShadowedBatch;
	
    params::InterfaceGlRef	mParams;
    vector<string>			mEnumNames;
    int						mEnumSelection;

    params::InterfaceGlRef	mParamsProjectors;
    vector<string>			mEnumNamesProjectors;
    int						mEnumSelectionProjector;
    quat                    mProjOrientation;
    
    /*--- projector configuration ---*/
    void loadConfiguration();
    void saveConfiguration();
    void loadImage();
    
    float scale;
    CameraOrtho         mCamOrtho;
    
    Projector           tProjector;
    float				tFov;
    float               tFar;
    float               tShiftV;
    float               tShiftH;
    
    gl::TextureRef		mProjectorPlanSketch;
    Rectf               destRect;
    
    params::InterfaceGlRef	mParamsConfig;
    vector<string>			mEnumConfigNamesAspect;
    int                     mEnumConfigSelectionAspect;
    vector<string>			mEnumConfigNamesView;
    int						mEnumConfigSelectionView;
    vector<string>          mEnumNamesScreen;
    int                     mEnumSelectionScreen;
    vector<string>			mEnumNamesHead;
    int                     mEnumSelectionHead;
    
    gl::VertBatchRef	mGrid;
};

void pmSimulator::setup()
{
    mode = ViewMode::MAIN;

    try {
        mGlsl = gl::GlslProg::create( loadResource(RES_SHADER_VERT), loadResource(RES_SHADER_FRAG) );
    }
    catch ( Exception &exc ) {
        CI_LOG_EXCEPTION( "glsl load failed", exc );
        std::terminate();
    }
    
    // objects
    auto floor				= geom::Cube().size( 200.0f, 0.1f, 200.0f );
    mFloorBatch				= gl::Batch::create( floor, gl::getStockShader( gl::ShaderDef() ) );
    mFloorShadowedBatch		= gl::Batch::create( floor, mGlsl );
    loadObj(loadResource(RES_BOX_OBJ));
    
    // projectors
    proj_num = 8;
    mProjector = new Projector[proj_num];
    for(int i=0; i<proj_num; i++){
        mProjector[i].setNearClip(0.1f);
        mProjector[i].setFarClip(100000.0f);
    }
    proj_on = new bool[proj_num];
    
    // set GUI menus
    setMenu();
    
    loadPlan(loadResource(RES_DEFAULT_PLAN));
    
    // main camera
    mPerspectiveCam.lookAt( vec3( 80.0f, 220.0f, 240.0f ), vec3( 0.0f, 50.0f, 0.0f ) );
    mPerspectiveCam.setPerspective( 40.0f, getWindowAspectRatio(), 0.1f, 1000000.0f );
    mCam = &mPerspectiveCam;
    mCamUi = CameraUi( mCam, getWindow() );
    
    /*--- projector configuration ---*/
    // projector
    tProjector.setPerspective( 25.0f, 16.0/9.0, 0.1f, 10.0f );
    tProjector.lookAt( vec3( 0.0f, 0.0f, 5.0f ), vec3( 0.0f, 0.0f, -1.0f) );
    
    // orthology camera
    scale = 30.0;
    mCamOrtho.setOrtho( -getWindowWidth()/(2*scale), getWindowWidth()/(2*scale), getWindowHeight()/(2*scale), -getWindowHeight()/(2*scale), -100, 100 );
    mCamOrtho.setEyePoint(vec3( 0.0f, 20.f, 0.0f ));
    mCamOrtho.setViewDirection(vec3( 0.0f, -1.0f, 0.0f ));
    
    // create grid
    mGrid = gl::VertBatch::create( GL_LINES );
    mGrid->begin( GL_LINES );
    for( int i = -10; i <= 10; ++i ) {
        mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
        mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
        mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
        mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
        
        mGrid->vertex( float( i ), 0.0f, -10.0f );
        mGrid->vertex( float( i ), 0.0f, +10.0f );
        mGrid->vertex( -10.0f, 0.0f, float( i ) );
        mGrid->vertex( +10.0f, 0.0f, float( i ) );
    }
    mGrid->end();
    
    tProjector.setName("unnamed");
    tFov = tProjector.getFov();
    tFar = tProjector.getFarClip();
    tShiftV = tProjector.getLensShiftVertical();
    tShiftH = tProjector.getLensShiftHorizontal();
}

void pmSimulator::resize()
{
    switch (mode) {
        case ViewMode::MAIN:
            mPerspectiveCam.setAspectRatio( getWindowAspectRatio() );
            mCam = &mPerspectiveCam;
            mEnumSelection = 0;
            break;
        case ViewMode::CONFIGURE:
            /*--- projector configuration ---*/
            mCamOrtho.setOrtho( -getWindowWidth()/(2*scale), getWindowWidth()/(2*scale), getWindowHeight()/(2*scale), -getWindowHeight()/(2*scale), -100, 100 );
            break;
        default:
            break;
    }

}

void pmSimulator::update()
{
    /* ------------------------------ renderDepthFbo ------------------------------ */
    // Set polygon offset to battle shadow acne
    gl::enable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 2.0f, 2.0f );
    
    for(int i=0; i<proj_num; i++){
        // Render scene to fbo from the view of the light
        gl::ScopedFramebuffer fbo( mProjector[i].fbo );
        gl::ScopedViewport viewport( vec2( 0.0f ), mProjector[i].fbo->getSize() );
        gl::clear( Color::black() );
        gl::color( Color::white() );
        gl::setMatrices( mProjector[i] );
        
        // start to draw objects
        gl::pushModelMatrix();
        gl::color( Color( 0.7f, 0.7f, 0.7f ) );
        mObjBatch->draw();
        gl::popModelMatrix();
        
        gl::pushModelMatrix();
        gl::color( Color( 0.7f, 0.7f, 0.7f ) );
        mFloorBatch->draw();
        gl::popModelMatrix();
    }
    
    // Disable polygon offset for final render
    gl::disable( GL_POLYGON_OFFSET_FILL );
    /* ------------------------------ renderDepthFbo ------------------------------ */
}

void pmSimulator::draw()
{
    switch (mode) {
        case ViewMode::MAIN:
            drawMainView();
            break;
        case ViewMode::CONFIGURE:
            drawConfigureeView();
            break;
        default:
            break;
    }
}

void pmSimulator::drawMainView(){
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::clear( Color::black() );
    gl::setMatrices( *mCam );
    
    gl::ScopedTextureBind texScopeS0( mProjector[0].shadowMapTex, (uint8_t) 0);
    gl::ScopedTextureBind texScope0( mProjector[0].tex, (uint8_t) 10);
    mGlsl->uniform( "uShadowMap0", 0);
    mGlsl->uniform( "uProjectorTex0", 10);
    
    gl::ScopedTextureBind texScopeS1( mProjector[1].shadowMapTex, (uint8_t) 1);
    gl::ScopedTextureBind texScope1( mProjector[1].tex, (uint8_t) 11);
    mGlsl->uniform( "uShadowMap1", 1);
    mGlsl->uniform( "uProjectorTex1", 11);
    
    gl::ScopedTextureBind texScopeS2( mProjector[2].shadowMapTex, (uint8_t) 2);
    gl::ScopedTextureBind texScope2( mProjector[2].tex, (uint8_t) 12);
    mGlsl->uniform( "uShadowMap2", 2);
    mGlsl->uniform( "uProjectorTex2", 12);
    
    gl::ScopedTextureBind texScopeS3( mProjector[3].shadowMapTex, (uint8_t) 3);
    gl::ScopedTextureBind texScope3( mProjector[3].tex, (uint8_t) 13);
    mGlsl->uniform( "uShadowMap3", 3);
    mGlsl->uniform( "uProjectorTex3", 13);
    
    gl::ScopedTextureBind texScopeS4( mProjector[4].shadowMapTex, (uint8_t) 4);
    gl::ScopedTextureBind texScope4( mProjector[4].tex, (uint8_t) 14);
    mGlsl->uniform( "uShadowMap4", 4);
    mGlsl->uniform( "uProjectorTex4", 14);
    
    gl::ScopedTextureBind texScopeS5( mProjector[5].shadowMapTex, (uint8_t) 5);
    gl::ScopedTextureBind texScope5( mProjector[5].tex, (uint8_t) 15);
    mGlsl->uniform( "uShadowMap5", 5);
    mGlsl->uniform( "uProjectorTex5", 15);
    
    gl::ScopedTextureBind texScopeS6( mProjector[6].shadowMapTex, (uint8_t) 6);
    gl::ScopedTextureBind texScope6( mProjector[6].tex, (uint8_t) 16);
    mGlsl->uniform( "uShadowMap6", 6);
    mGlsl->uniform( "uProjectorTex6", 16);
    
    gl::ScopedTextureBind texScopeS7( mProjector[7].shadowMapTex, (uint8_t) 7);
    gl::ScopedTextureBind texScope7( mProjector[7].tex, (uint8_t) 17);
    mGlsl->uniform( "uShadowMap7", 7);
    mGlsl->uniform( "uProjectorTex7", 17);
    
    vec3* mvLightPos = new vec3[proj_num];
    mat4* shadowMatrix = new mat4[proj_num];
    for(int i=0; i<proj_num; i++){
        mvLightPos[i]	= vec3( gl::getModelView() * vec4( mProjector[i].getEyePoint(), 1.0f ) ) ;
        shadowMatrix[i] = mProjector[i].getProjectionMatrix() * mProjector[i].getViewMatrix();
    }
    mGlsl->uniform("uLightPos[0]", mvLightPos, proj_num);
    mGlsl->uniform("uShadowMatrix[0]", shadowMatrix, proj_num);
    delete[] mvLightPos;
    delete[] shadowMatrix;
    
    gl::drawCoordinateFrame( 110.0 );
    
    // start to draw objects
    gl::pushModelMatrix();
    gl::color( Color( 0.7f, 0.7f, 0.7f ) );
    mObjShadowedBatch->draw();
    gl::popModelMatrix();
    
    gl::pushModelMatrix();
    gl::color( Color( 0.7f, 0.7f, 0.7f ) );
    mFloorShadowedBatch->draw();
    gl::popModelMatrix();
    // end to draw objects
    
    // light areas
    for(int i=0; i<proj_num; i++){
        if(mProjector[i].isOn()){
            gl::color( Color( 0.9f, 0.0f, 0.0f ) );
            gl::drawLine(mProjector[i].getEyePoint(), mProjector[i].getEyePoint()+mProjector[i].getViewDirection()*10.0f);
            gl::color( Color( 0.0f, 0.0f, 0.9f ) );
            gl::drawFrustum (mProjector[i]);
        }
    }
    
    mParams->draw();
    mParamsProjectors->draw();
}

void pmSimulator::drawConfigureeView(){
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::clear( Color::black() );
    
    gl::setMatricesWindow( getWindowSize() );
    // draw sketch image
    if( mProjectorPlanSketch ) {
        gl::color( Color( 1.0f, 1.0f, 1.0f ) );
        gl::draw( mProjectorPlanSketch, destRect );
    }
    
    gl::setMatrices( mCamOrtho );
    // draw axis and grid
    gl::drawCoordinateFrame( 2 );
    mGrid->draw();
    // projector area
    gl::color( Color( 0.9f, 0.9f, 0.0f ) );
    gl::drawFrustum (tProjector);
    
    // draw the interface
    mParamsConfig->draw();
}

void pmSimulator::loadObj( const DataSourceRef &dataSource )
{
    ObjLoader loader( dataSource );
    auto mMesh = TriMesh::create( loader );
    
    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) )
        mMesh->recalculateNormals();
    
    mObjBatch			= gl::Batch::create( *mMesh, gl::getStockShader( gl::ShaderDef() ) );
    mObjShadowedBatch	= gl::Batch::create( *mMesh, mGlsl );
}

void pmSimulator::loadObjButton(){
    try {
        fs::path path = getOpenFilePath("", {"obj"});
        if( ! path.empty() ) {
            objName = path.filename().string();
            loadObj(loadFile(path));
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to load a model.", exc );
    }
}

void pmSimulator::loadProjectorConf(int id){
    try {
        fs::path path = getOpenFilePath("", {"cnf"});
        if( ! path.empty() ) {
            ifstream fin(path.c_str(), ios::in);
            // error check
            if(!fin){
                cout << "loadProjectorConf(): cannot open the file..." << endl;
                return;
            } else {
                string name;
                int aspectType;
                float fov;
                float shiftV;
                float shiftH;
                
                string line;
                while(fin && getline(fin, line)){
                    if(line.find("name:")==0){
                        line.replace(0,5,"");
                        name = line;
                    } else if(line.find("aspect:")==0){
                        line.replace(0,7,"");
                        aspectType = stoi(line);
                    } else if(line.find("fov:")==0){
                        line.replace(0,4,"");
                        fov = stof(line);
                    } else if(line.find("lensshiftV:")==0){
                        line.replace(0,11,"");
                        shiftV = stof(line);
                    } else if(line.find("lensshiftH:")==0){
                        line.replace(0,11,"");
                        shiftH = stof(line);
                    }
                }
                
                mProjector[id].setName(name);
                mParams->setOptions( "name"+to_string(id), "label=`P"+to_string(id+1)+": "+mProjector[id].getName()+"`" );
                mParamsProjectors->setOptions( "nameTP", "label=`"+mProjector[mEnumSelectionProjector].getName()+"`" );
                if(aspectType==0){
                    mProjector[id].setAspectType(Aspect::TYPE_16X9);
                } else if(aspectType==1){
                    mProjector[id].setAspectType(Aspect::TYPE_4X3);
                }
                mProjector[id].setFov(fov);
                mProjector[id].setLensShiftVertical(shiftV);
                mProjector[id].setLensShiftHorizontal(shiftH);
            }
            fin.close();
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to load a projector configuration.", exc );
    }
}

void pmSimulator::loadConfButton(){
    loadProjectorConf(mEnumSelectionProjector);
}

void pmSimulator::loadPlan( const DataSourceRef &dataSource ){
    planPath = dataSource->getFilePath();
    ifstream fin(planPath.c_str(), ios::in);
    // error check
    if(!fin){
        cout << "loadPlan(): cannot open the file..." << endl;
        return;
    } else {
        string line;
        while(fin && getline(fin, line)){
            if(line.find("object:")==0){
                line.replace(0,7,"");
                objName = line;
                if(!objName.empty()) {
                    string dir = planPath.parent_path().string();
                    loadObj(loadFile(dir+"/"+objName));
                } else {
                    cout << "no object file" << endl;
                }
            } else if(line.find("viewPerspFov:")==0){
                line.replace(0,13,"");
                mPerspectiveCam.setFov(stof(line));
            } else if(line.find("viewPerspPos:")==0){
                line.replace(0,13,"");
                vector<string> v = split(line, ",");
                mPerspectiveCam.setEyePoint(vec3(stof(v[0]),stof(v[1]),stof(v[2])));
                mPerspectiveCam.lookAt(vec3(0.0f, 2.0f, 0.0f));
            //} else if(line.find("viewPerspDir:")==0){
            //    line.replace(0,13,"");
            //    vector<string> v = split(line, ",");
            //    mPerspectiveCam.setViewDirection(vec3(stof(v[0]),stof(v[1]),stof(v[2])));
            } else if(line.find("viewPerspWorldUp:")==0){
                line.replace(0,17,"");
                vector<string> v = split(line, ",");
                mPerspectiveCam.setWorldUp(vec3(stof(v[0]),stof(v[1]),stof(v[2])));
            } else if(line.find("swtichON")==0){
                line.replace(0,8,"");
                vector<string> substring = split(line, ":");
                mProjector[stoi(substring[0])].setLight(stoi(substring[1]));
                proj_on[stoi(substring[0])] = mProjector[stoi(substring[0])].isOn();
            } else if(line.find("name")==0){
                line.replace(0,4,"");
                vector<string> substring = split(line, ":");
                mProjector[stoi(substring[0])].setName(substring[1]);
                mParams->setOptions( "name"+substring[0], "label=`P"+to_string(stoi(substring[0])+1)+": "+mProjector[stoi(substring[0])].getName()+"`" );
            } else if(line.find("aspect")==0){
                line.replace(0,6,"");
                vector<string> substring = split(line, ":");
                if(stoi(substring[1])==0){
                    mProjector[stoi(substring[0])].setAspectType(Aspect::TYPE_16X9);
                } else if(stoi(substring[1])==1){
                    mProjector[stoi(substring[0])].setAspectType(Aspect::TYPE_4X3);
                }
            } else if(line.find("fov")==0){
                line.replace(0,3,"");
                vector<string> substring = split(line, ":");
                mProjector[stoi(substring[0])].setFov(stof(substring[1]));
            } else if(line.find("lensshiftV")==0){
                line.replace(0,10,"");
                vector<string> substring = split(line, ":");
                mProjector[stoi(substring[0])].setLensShiftVertical(stof(substring[1]));
            } else if(line.find("lensshiftH")==0){
                line.replace(0,10,"");
                vector<string> substring = split(line, ":");
                mProjector[stoi(substring[0])].setLensShiftHorizontal(stof(substring[1]));
            } else if(line.find("position")==0){
                line.replace(0,8,"");
                vector<string> substring = split(line, ":");
                vector<string> v = split(substring[1], ",");
                mProjector[stoi(substring[0])].setEyePoint(vec3(stof(v[0]),stof(v[1]),stof(v[2])));
            } else if(line.find("direction")==0){
                line.replace(0,9,"");
                vector<string> substring = split(line, ":");
                vector<string> v = split(substring[1], ",");
                mProjector[stoi(substring[0])].setViewDirection(vec3(stof(v[0]),stof(v[1]),stof(v[2])));
            } else if(line.find("head")==0){
                line.replace(0,4,"");
                vector<string> substring = split(line, ":");
                if(stoi(substring[1])==0){
                    mProjector[stoi(substring[0])].setHead(HEAD::UP);
                } else if(stoi(substring[1])==1){
                    mProjector[stoi(substring[0])].setHead(HEAD::DOWN);
                } else if(stoi(substring[1])==2){
                    mProjector[stoi(substring[0])].setHead(HEAD::RIGHT);
                } else if(stoi(substring[1])==3){
                    mProjector[stoi(substring[0])].setHead(HEAD::LEFT);
                }
            }
        }
        mParamsProjectors->setOptions( "nameTP", "label=`"+mProjector[mEnumSelectionProjector].getName()+"`" );
    }
    fin.close();
}

void pmSimulator::loadPlanButton(){
    try {
        fs::path path = getOpenFilePath("", {"pln"});
        if( ! path.empty() ) {
            loadPlan(loadFile(path));
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to load a projection mapping plan.", exc );
    }
}

void pmSimulator::savePlan(){
    try {
        fs::path path = getSaveFilePath("", {"pln"});
        if( ! path.empty() ) {
            ofstream fout(path.c_str(), ios::out);
            // error check
            if(!fout){
                cout << "savePlan(): cannot open the file..." << endl;
                return;
            } else {
                
                fout << "object:" << objName << "\n\n";
                
                vec3 pos = mPerspectiveCam.getEyePoint();
                vec3 dir = mPerspectiveCam.getViewDirection();
                vec3 head = mPerspectiveCam.getWorldUp();
                fout << "viewPerspFov:";
                fout << mPerspectiveCam.getFov();
                fout << "\n";
                fout << "viewPerspPos:";
                fout << pos.x << "," << pos.y << "," << pos.z;
                fout << "\n";
                fout << "viewPerspDir:";
                fout << dir.x << "," << dir.y << "," << dir.z;
                fout << "\n";
                fout << "viewPerspWorldUp:";
                fout << head.x << "," << head.y << "," << head.z;
                fout << "\n";
                fout << "\n";

                for(int i=0; i<proj_num; i++){
                    pos = mProjector[i].getEyePoint();
                    dir = mProjector[i].getViewDirection();
                    head = mProjector[i].getWorldUp();
                    
                    fout << "swtichON"+to_string(i)+":";
                    fout << mProjector[i].isOn();
                    fout << "\n";
                    fout << "name"+to_string(i)+":";
                    fout << mProjector[i].getName();
                    fout << "\n";
                    fout << "aspect"+to_string(i)+":";
                    fout << mProjector[i].getAspectType();
                    fout << "\n";
                    fout << "fov"+to_string(i)+":";
                    fout << mProjector[i].getFov();
                    fout << "\n";
                    fout << "lensshiftV"+to_string(i)+":";
                    fout << mProjector[i].getLensShiftVertical();
                    fout << "\n";
                    fout << "lensshiftH"+to_string(i)+":";
                    fout << mProjector[i].getLensShiftHorizontal();
                    fout << "\n";
                    fout << "position"+to_string(i)+":";
                    fout << pos.x << "," << pos.y << "," << pos.z;
                    fout << "\n";
                    fout << "direction"+to_string(i)+":";
                    fout << dir.x << "," << dir.y << "," << dir.z;
                    fout << "\n";
                    fout << "head"+to_string(i)+":";
                    fout << mProjector[i].getHead();
                    fout << "\n";
                    fout << "\n";
                }
                cout<<"saved projection mapping plan\n";
            }
            fout.close();
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to save a projection mapping plan.", exc );
    }
}

void pmSimulator::setProjPosition(vec3 pos)
{
    mProjector[mEnumSelectionProjector].setEyePoint(pos);
}

vec3 pmSimulator::getProjPosition()
{
    return mProjector[mEnumSelectionProjector].getEyePoint();
}

/*--- projector configuration ---*/
void pmSimulator::loadConfiguration(){
    try {
        fs::path path = getOpenFilePath("", {"cnf"});
        if( ! path.empty() ) {
            ifstream fin(path.c_str(), ios::in);
            // error check
            if(!fin){
                cout << "loadConfiguration(): cannot open the file..." << endl;
                return;
            } else {
                string line;
                while(fin && getline(fin, line)){
                    //while(fin>>line){
                    if(line.find("name:")==0){
                        line.replace(0,5,"");
                        tProjector.setName(line);
                        mParamsConfig->setOptions( "name", "label=`"+tProjector.getName()+"`" );
                    } else if(line.find("aspect:")==0){
                        line.replace(0,7,"");
                        mEnumConfigSelectionAspect = stoi(line);
                        if(mEnumConfigSelectionAspect==Aspect::TYPE_16X9){
                            tProjector.setAspectType(Aspect::TYPE_16X9);
                        } else if(mEnumConfigSelectionAspect==Aspect::TYPE_4X3){
                            tProjector.setAspectType(Aspect::TYPE_4X3);
                        }
                    } else if(line.find("fov:")==0){
                        line.replace(0,4,"");
                        tProjector.setFov(stof(line));
                        tFov = tProjector.getFov();
                    } else if(line.find("farclip:")==0){
                        line.replace(0,8,"");
                        tProjector.setFarClip(stof(line));
                        tFar = tProjector.getFarClip();
                    } else if(line.find("lensshiftV:")==0){
                        line.replace(0,11,"");
                        tShiftV = stof(line);
                        tProjector.setLensShiftVertical(tShiftV);
                    } else if(line.find("lensshiftH:")==0){
                        line.replace(0,11,"");
                        tShiftH = stof(line);
                        tProjector.setLensShiftHorizontal(tShiftH);
                    }
                }
            }
            fin.close();
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to load a projector configuration.", exc );
    }
}

/*--- projector configuration ---*/
void pmSimulator::saveConfiguration(){
    try {
        fs::path path = getSaveFilePath("", {"cnf"});
        if( ! path.empty() ) {
            ofstream fout(path.c_str(), ios::out);
            // error check
            if(!fout){
                cout << "saveConfiguration(): cannot open the file..." << endl;
                return;
            } else {
                tProjector.setName(path.replace_extension().filename().string());
                mParamsConfig->setOptions( "name", "label=`"+tProjector.getName()+"`" );
                fout << "name:";
                fout << tProjector.getName();
                fout << "\n";
                fout << "aspect:";
                fout << tProjector.getAspectType();
                fout << "\n";
                fout << "fov:";
                fout << tProjector.getFov();
                fout << "\n";
                fout << "farclip:";
                fout << tProjector.getFarClip();
                fout << "\n";
                fout << "lensshiftV:";
                fout << tProjector.getLensShiftVertical();
                fout << "\n";
                fout << "lensshiftH:";
                fout << tProjector.getLensShiftHorizontal();
                fout << "\n";
                cout<<"saved projector configuration\n";
            }
            fout.close();
        }
    }
    catch(Exception &exc) {
        CI_LOG_EXCEPTION( "Unable to save a projector configuration.", exc );
    }
}

/*--- projector configuration ---*/
void pmSimulator::loadImage(){
    try {
        fs::path path = getOpenFilePath( "", ImageIo::getLoadExtensions() );
        if( ! path.empty() ) {
            mProjectorPlanSketch = gl::Texture::create( ci::loadImage( path ) );
            destRect = Rectf( mProjectorPlanSketch->getBounds() ).getCenteredFit( getWindowBounds(), true );
        }
    }
    catch( Exception &exc ) {
        CI_LOG_EXCEPTION( "failed to load image.", exc );
    }
}

void pmSimulator::setMenu(){
    // mode menu
    mParamsMode = params::InterfaceGl::create("Mode", toPixels(ivec2(180, 10)));
    mParamsMode->setPosition(vec2(getWindowSize().x*2-400, 16));
    mEnumSelectionMode = 0;
    mEnumNamesMode = { "Main", "Configure"};
    mParamsMode->addParam( "Mode", mEnumNamesMode, &mEnumSelectionMode )
    .keyDecr( "m" )
    .updateFn( [this] {
        if(mEnumSelectionMode==0){
            mode = ViewMode::MAIN;
            mParams->show();
            mParamsProjectors->show();
            mParamsConfig->hide();
            mCamUi.connect(getWindow());
        } else if(mEnumSelectionMode==1) {
            mode = ViewMode::CONFIGURE;
            mParams->hide();
            mParamsProjectors->hide();
            mParamsConfig->show();
            mCamUi.disconnect();
       }
    } );
    
    // main menu
    mParams = params::InterfaceGl::create("Menu", toPixels(ivec2(220, 400)));
    mParams->setPosition(vec2(16, 16));
    mParams->addButton("load object...", bind(&pmSimulator::loadObjButton, this));
    mParams->addButton("load plan...", bind(&pmSimulator::loadPlanButton, this));
    mParams->addButton("save plan...", bind(&pmSimulator::savePlan, this));
    mParams->addSeparator();
    mParams->addParam("Projector1", &proj_on[0]).keyIncr( "1" ).updateFn([this]{
        mProjector[0].setLight(proj_on[0]);
        if(mEnumSelectionProjector==0) mEnumSelectionScreen = mProjector[0].getScreen();
    });
    mParams->addParam("Projector2", &proj_on[1]).keyIncr( "2" ).updateFn([this]{mProjector[1].setLight(proj_on[1]);});
    mParams->addParam("Projector3", &proj_on[2]).keyIncr( "3" ).updateFn([this]{mProjector[2].setLight(proj_on[2]);});
    mParams->addParam("Projector4", &proj_on[3]).keyIncr( "4" ).updateFn([this]{mProjector[3].setLight(proj_on[3]);});
    mParams->addParam("Projector5", &proj_on[4]).keyIncr( "5" ).updateFn([this]{mProjector[4].setLight(proj_on[4]);});
    mParams->addParam("Projector6", &proj_on[5]).keyIncr( "6" ).updateFn([this]{mProjector[5].setLight(proj_on[5]);});
    mParams->addParam("Projector7", &proj_on[6]).keyIncr( "7" ).updateFn([this]{mProjector[6].setLight(proj_on[6]);});
    mParams->addParam("Projector8", &proj_on[7]).keyIncr( "8" ).updateFn([this]{mProjector[7].setLight(proj_on[7]);});
    mParams->addSeparator();
    mParams->addText( "name0", "label=`P1: "+mProjector[0].getName()+"`" );
    mParams->addText( "name1", "label=`P2: "+mProjector[1].getName()+"`" );
    mParams->addText( "name2", "label=`P3: "+mProjector[2].getName()+"`" );
    mParams->addText( "name3", "label=`P4: "+mProjector[3].getName()+"`" );
    mParams->addText( "name4", "label=`P5: "+mProjector[4].getName()+"`" );
    mParams->addText( "name5", "label=`P6: "+mProjector[5].getName()+"`" );
    mParams->addText( "name6", "label=`P7: "+mProjector[6].getName()+"`" );
    mParams->addText( "name7", "label=`P8: "+mProjector[7].getName()+"`" );
    mParams->addSeparator();
    mEnumSelection = 0;
    mEnumNames = { "Perspective", "P1", "P2", "P3", "P4" , "P5", "P6" , "P7", "P8"};
    mParams->addParam( "view from...", mEnumNames, &mEnumSelection )
    .keyDecr( "-" )
    .keyIncr( "=" )
    .updateFn( [this] {
        if(mEnumSelection==0){
            mCam = &mPerspectiveCam;
        } else if(mEnumSelection>0) {
            mCam = &mProjector[mEnumSelection-1];
        }
        mCamUi = CameraUi( mCam, getWindow() );
    } );
    
    // control projectors
    mParamsProjectors = params::InterfaceGl::create("Projector Control", toPixels(ivec2(220, 200)));
    mParamsProjectors->setPosition(vec2(16, 840));
    mEnumSelectionProjector = 0;
    mEnumNamesProjectors = { "P1", "P2", "P3", "P4" , "P5", "P6" , "P7", "P8"};
    mProjOrientation = mProjector[mEnumSelectionProjector].getOrientation();
    mParamsProjectors->addParam( "Projector...", mEnumNamesProjectors, &mEnumSelectionProjector )
    .keyDecr( "[" )
    .keyIncr( "]" )
    .updateFn( [this] {
        mEnumSelectionHead = mProjector[mEnumSelectionProjector].getHead();
        mEnumSelectionScreen = mProjector[mEnumSelectionProjector].getScreen();
        mProjOrientation = mProjector[mEnumSelectionProjector].getOrientation();
        mParamsProjectors->setOptions( "nameTP", "label=`"+mProjector[mEnumSelectionProjector].getName()+"`" );
    } );
    mParamsProjectors->addText( "nameTP", "label=`"+mProjector[mEnumSelectionProjector].getName()+"`" );
    mParamsProjectors->addSeparator();
    mParamsProjectors->addButton("load proj.conf...", bind(&pmSimulator::loadConfButton, this));
    mParamsProjectors->addSeparator();
    function<void( vec3 )> setter	= bind( &pmSimulator::setProjPosition, this, placeholders::_1 );
    function<vec3 ()> getter		= bind( &pmSimulator::getProjPosition, this );
    mParamsProjectors->addParam( "Position", setter, getter );
    mParamsProjectors->addParam( "Rotation", &mProjOrientation )
    .updateFn( [this] {
        mProjector[mEnumSelectionProjector].setOrientation(mProjOrientation);
        mProjector[mEnumSelectionProjector].setWorldUp(vec3(0.0,1.0,0.0));
    } );
    mEnumSelectionScreen = 0;
    mEnumNamesScreen = { "Test Pattern", "Red", "Green", "Blue", "Cyan" , "Magenta", "Yellow" , "Orange", "Purple", "off"};
    mParamsProjectors->addParam( "Screen...", mEnumNamesScreen, &mEnumSelectionScreen )
    .keyDecr( "." )
    .keyIncr( "/" )
    .updateFn( [this] {
        mProjector[mEnumSelectionProjector].setScreen((SCREEN)mEnumSelectionScreen);
        proj_on[mEnumSelectionProjector] = mProjector[mEnumSelectionProjector].isOn();
    } );
    mEnumSelectionHead = 0;
    mEnumNamesHead = { "Up", "Down", "Right", "Left"};
    mParamsProjectors->addParam( "Head...", mEnumNamesHead, &mEnumSelectionHead )
    .keyDecr( "l" )
    .keyIncr( ";" )
    .updateFn( [this] {
        mProjector[mEnumSelectionProjector].setHead((HEAD)mEnumSelectionHead);
    } );
    
    /*--- projector configuration ---*/
    mParamsConfig = params::InterfaceGl::create( getWindow(), "Projector configuration", toPixels( ivec2( 200, 300 ) ) );
    mParamsConfig->setPosition( ivec2( 16, 16 ) );
    mParamsConfig->addText( "name", "label=`"+tProjector.getName()+"`" );
    mParamsConfig->addSeparator();
    mParamsConfig->addText( "text0", "label=`Paramenters`" );
    mEnumConfigSelectionAspect = Aspect::TYPE_16X9;
    mEnumConfigNamesAspect = { "16:9", "4:3"};
    mParamsConfig->addParam( "Aspect", mEnumConfigNamesAspect, &mEnumConfigSelectionAspect ).keyDecr( "[" ).keyIncr( "]" )
    .updateFn( [this] {
        if(mEnumConfigSelectionAspect==Aspect::TYPE_16X9){
            tProjector.setAspectRatio(16.0/9.0);
        } else if(mEnumConfigSelectionAspect==Aspect::TYPE_4X3){
            tProjector.setAspectRatio(4.0/3.0);
        }
    } );
    mParamsConfig->addParam( "Fov", &tFov ).min( 0.1f ).max( 170.0f ).keyIncr( "z" ).keyDecr( "Z" ).precision( 1 ).step( 0.01f ).updateFn( [this] { tProjector.setFov(tFov); });
    mParamsConfig->addParam( "Far clip", &tFar ).min( 1.0f ).max( 200.0f ).keyIncr( "x" ).keyDecr( "X" ).precision( 1 ).step( 0.01f ).updateFn( [this] { tProjector.setFarClip(tFar); });
    mParamsConfig->addParam( "Lens Shift V", &tShiftV ).min( -10.0f ).max( 10.0f ).keyIncr( "v" ).keyDecr( "V" ).precision( 1 ).step( 0.01f ).updateFn( [this] { tProjector.setLensShiftVertical(tShiftV); });
    mParamsConfig->addParam( "Lens Shift H", &tShiftH ).min( -10.0f ).max( 10.0f ).keyIncr( "b" ).keyDecr( "B" ).precision( 1 ).step( 0.01f ).updateFn( [this] { tProjector.setLensShiftHorizontal(tShiftH); });
    mParamsConfig->addSeparator();
    mParamsConfig->addText( "text1", "label=`View`" );
    mEnumConfigSelectionView = 0;
    mEnumConfigNamesView = { "top", "side", "front"};
    mParamsConfig->addParam( "View from...", mEnumConfigNamesView, &mEnumConfigSelectionView )
    .updateFn( [this] {
        switch(mEnumConfigSelectionView){
            case 0:
                mCamOrtho.setOrtho( -getWindowWidth()/(2*scale), getWindowWidth()/(2*scale), getWindowHeight()/(2*scale), -getWindowHeight()/(2*scale), -100, 100 );
                mCamOrtho.setEyePoint(vec3( 0.0f, 20.f, 0.0f ));
                mCamOrtho.setViewDirection(vec3( 0.0f, -1.0f, 0.0f ));
                break;
            case 1:
                mCamOrtho.setOrtho( getWindowWidth()/(2*scale), -getWindowWidth()/(2*scale), -getWindowHeight()/(2*scale), getWindowHeight()/(2*scale), -100, 100 );
                mCamOrtho.setEyePoint(vec3( -20.0f, 0.0f, 0.0f ));
                mCamOrtho.setViewDirection(vec3( 1.0f, 0.0f, 0.0f ));
                break;
            case 2:
                mCamOrtho.setOrtho( -getWindowWidth()/(2*scale), getWindowWidth()/(2*scale), -getWindowHeight()/(2*scale), getWindowHeight()/(2*scale), -100, 100 );
                mCamOrtho.setEyePoint(vec3( 0.0f, 0.0f, 20.0f ));
                mCamOrtho.setViewDirection(vec3( 0.0f, 0.0f, -1.0f ));
                break;
            default:
                break;
        }
    } );
    mParamsConfig->addSeparator();
    mParamsConfig->addButton("load...", bind(&pmSimulator::loadConfiguration, this));
    mParamsConfig->addButton("save...", bind(&pmSimulator::saveConfiguration, this));
    mParamsConfig->addButton("load image...", bind(&pmSimulator::loadImage, this));
    
    if( mode == ViewMode::MAIN) {
        mParams->show();
        mParamsProjectors->show();
        mParamsConfig->hide();
    } else if(mode == ViewMode::CONFIGURE) {
        mParams->hide();
        mParamsProjectors->hide();
        mParamsConfig->show();
   }
}

void pmSimulator::mouseDrag( MouseEvent event ) {
    if(mode==ViewMode::MAIN){
        mProjOrientation = mProjector[mEnumSelectionProjector].getOrientation();
        mParamsProjectors->setOptions( "nameTP", "label=`"+mProjector[mEnumSelectionProjector].getName()+"`" );
    } else if (mode==ViewMode::CONFIGURE) {
        vec2 tp = event.getPos() - mMousePos; tp /= 50.0f;
        vec3 eyePoint = mCamOrtho.getEyePoint();
        switch(mEnumConfigSelectionView){
            case 0:
                mCamOrtho.setEyePoint(vec3(eyePoint.x-tp.x, eyePoint.y, eyePoint.z-tp.y));
                break;
            case 1:
                mCamOrtho.setEyePoint(vec3(eyePoint.x, eyePoint.y+tp.y, eyePoint.z-tp.x));
                break;
            case 2:
                mCamOrtho.setEyePoint(vec3(eyePoint.x-tp.x, eyePoint.y+tp.y, eyePoint.z));
                break;
            default:
                break;
        }
        mMousePos = event.getPos();
    }
}

void pmSimulator::mouseMove( MouseEvent event )
{
    // Keep track of the mouse.
    mMousePos = event.getPos();
}

void pmSimulator::keyDown( KeyEvent event )
{
    if(event.getCode() == KeyEvent::KEY_ESCAPE) {
        // Exit full screen, or quit the application, when the user presses the ESC key.
        if( isFullScreen() )
            setFullScreen( false );
        else
            quit();
    } else if(event.getChar() == 'f') {
        // Toggle full screen when the user presses the 'f' key.
        setFullScreen( ! isFullScreen() );
    }
}

pmSimulator::pmSimulator()
{
    // Connect the windows to their respective signals
    //getWindowIndex( 0 )->getSignalDraw().connect( bind( &pmSimulator::draw, this ) );
    //getWindowIndex( 1 )->getSignalDraw().connect( bind( &pmSimulator::drawWindow1, this ) );
}

void pmSimulator::prepareSettings( Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize(1280, 720);
    settings->setTitle("Projection Mapping Simulator");
    //settings->prepareWindow( Window::Format().size(1280, 720).pos( ivec2( 0, 40 ) ).title( "Window 0" ) );
    //settings->prepareWindow( Window::Format().size( ivec2( 256 ) ).pos( ivec2( 0, 320 ) ).title( "Window 1" ) );
}

CINDER_APP( pmSimulator, RendererGl, pmSimulator::prepareSettings )
