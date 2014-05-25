//
//  HelloWorldScene.cpp
//  Freehand
//
//  Created by Akihiro Matsuura on 3/8/13.
//  Copyright Syuhari, Inc. 2013. All rights reserved.
//
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define PTM_RATIO 32

enum {
    kTagParentNode = 1,
};

PhysicsSprite::PhysicsSprite()
: m_pBody(NULL)
{

}

void PhysicsSprite::setPhysicsBody(b2Body * body)
{
    m_pBody = body;
}

// this method will only get called if the sprite is batched.
// return YES if the physics values (angles, position ) changed
// If you return NO, then nodeToParentTransform won't be called.
bool PhysicsSprite::isDirty(void)
{
    return true;
}

// returns the transform matrix according the Chipmunk Body values
CCAffineTransform PhysicsSprite::nodeToParentTransform(void)
{
    b2Vec2 pos  = m_pBody->GetPosition();

    float x = pos.x * PTM_RATIO;
    float y = pos.y * PTM_RATIO;

    if ( isIgnoreAnchorPointForPosition() ) {
        x += m_obAnchorPointInPoints.x;
        y += m_obAnchorPointInPoints.y;
    }

    // Make matrix
    float radians = m_pBody->GetAngle();
    float c = cosf(radians);
    float s = sinf(radians);

    if( ! m_obAnchorPointInPoints.equals(CCPointZero) ){
        x += c*-m_obAnchorPointInPoints.x + -s*-m_obAnchorPointInPoints.y;
        y += s*-m_obAnchorPointInPoints.x + c*-m_obAnchorPointInPoints.y;
    }

    // Rot, Translate Matrix
    m_sTransform = CCAffineTransformMake( c,  s, -s, c, x, y );

    return m_sTransform;
}

HelloWorld::HelloWorld()
:m_bClearBox(false)
,m_fAcclX(-0.98f)
,m_fAcclY(0.0f)
{
    CCDirector::sharedDirector()->setDepthTest(false);
    
    setTouchEnabled( true );

    boxLayer = CCLayer::create();
    this->addChild(boxLayer);
    
    CCSize s = CCDirector::sharedDirector()->getWinSize();
    // init physics
    this->initPhysics();
    
    scheduleUpdate();
    
    target = CCRenderTexture::create(s.width, s.height, kCCTexture2DPixelFormat_RGBA8888);
    target->retain();
    target->setPosition(ccp(s.width / 2, s.height / 2));
    
    boxLayer->addChild(target);
    
    brush = CCSprite::create("brush.png");
    brush->retain();
    
    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create("CloseNormal.png", "CloseSelected.png", this, menu_selector(HelloWorld::clearBox) );
    pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20) );
    
    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition( CCPointZero );
    this->addChild(pMenu, 1);
    
    setAccelerometerEnabled(true);
    setAccelerometerInterval(0.1f);
}

HelloWorld::~HelloWorld()
{
    delete world;
    world = NULL;
    
    //delete m_debugDraw;
}

void HelloWorld::initPhysics()
{

    CCSize s = CCDirector::sharedDirector()->getWinSize();

    b2Vec2 gravity;
    gravity.Set(0.0f, -10.0f);
    world = new b2World(gravity);
    world->SetAllowSleeping(false);

    // Do we want to let bodies sleep?
    world->SetAllowSleeping(true);

    world->SetContinuousPhysics(true);

//     m_debugDraw = new GLESDebugDraw( PTM_RATIO );
//     world->SetDebugDraw(m_debugDraw);

    uint32 flags = 0;
    flags += b2Draw::e_shapeBit;
    //        flags += b2Draw::e_jointBit;
    //        flags += b2Draw::e_aabbBit;
    //        flags += b2Draw::e_pairBit;
    //        flags += b2Draw::e_centerOfMassBit;
    //m_debugDraw->SetFlags(flags);


    // Define the ground body.
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0, 0); // bottom-left corner

    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    b2Body* groundBody = world->CreateBody(&groundBodyDef);

    // Define the ground box shape.
    b2EdgeShape groundBox;

    // bottom
    groundBox.Set(b2Vec2(0,0), b2Vec2(s.width/PTM_RATIO,0));
    groundBody->CreateFixture(&groundBox,0);

    // top
    groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO));
    groundBody->CreateFixture(&groundBox,0);

    // left
    groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(0,0));
    groundBody->CreateFixture(&groundBox,0);

    // right
    groundBox.Set(b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,0));
    groundBody->CreateFixture(&groundBox,0);
}

void HelloWorld::rotateInterfaceOrientation(int orientation)
{
    CCLOG("orientation=%d", orientation);
}

void HelloWorld::draw()
{
    //
    // IMPORTANT:
    // This is only for debug purposes
    // It is recommend to disable it
    //
    /*
    CCLayer::draw();

    ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );

    kmGLPushMatrix();

    world->DrawDebugData();

    kmGLPopMatrix();
    */
}

void HelloWorld::didAccelrate(CCAcceleration* pAccl)
{
    m_fAcclX = pAccl->x;
    m_fAcclY = pAccl->y;
}

void HelloWorld::update(float dt)
{
    //It is recommended that a fixed time step is used with Box2D for stability
    //of the simulation, however, we are using a variable time step here.
    //You need to make an informed choice, the following URL is useful
    //http://gafferongames.com/game-physics/fix-your-timestep/
    
    if (m_bClearBox) {
        // 画面クリア処理
        boxLayer->removeChild(target, true);
        target->release();
        
        boxLayer->removeAllChildren();
        m_bClearBox = false;
        
        for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
        {
            if (b->GetUserData() != NULL) {
                world->DestroyBody(b);
            }    
        }
        
        CCSize s = CCDirector::sharedDirector()->getWinSize();

        target = CCRenderTexture::create(s.width, s.height, kCCTexture2DPixelFormat_RGBA8888);
        target->retain();
        target->setPosition(ccp(s.width / 2, s.height / 2));
        boxLayer->addChild(target);
        
        return;
    }
    
    // 加速度センサより重力の向きを決定する
    b2Vec2 gravity;
    gravity.Set(-m_fAcclY*10.0f, m_fAcclX*10.0f);
    world->SetGravity(gravity);
    
    int velocityIterations = 8;
    int positionIterations = 1;
    
    // Instruct the world to perform a single step of simulation. It is
    // generally best to keep the time step and iterations fixed.
    world->Step(dt, velocityIterations, positionIterations);
    
    //Iterate over the bodies in the physics world
    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        if (b->GetUserData() != NULL) {
            //Synchronize the AtlasSprites position and rotation with the corresponding body
            CCSprite* myActor = (CCSprite*)b->GetUserData();
            myActor->setPosition( CCPointMake( b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO) );
            myActor->setRotation( -1 * CC_RADIANS_TO_DEGREES(b->GetAngle()) );
        }    
    }
}

void HelloWorld::addRectangle(b2Body* body, CCPoint start, CCPoint end)
{
    // 画像の最小幅
    float min = brush->getContentSize().width*brush->getScale()/PTM_RATIO;
    
    float dist_x = start.x-end.x;
    float dist_y = start.y-end.y;
    float angle = atan2(dist_y, dist_x);
    float px = (start.x+end.x)/2/PTM_RATIO - body->GetPosition().x;
    float py = (start.y+end.y)/2/PTM_RATIO - body->GetPosition().y;
    float width = MAX(abs(dist_x)/PTM_RATIO, min);
    float height = MAX(abs(dist_y)/PTM_RATIO, min);
        
    b2PolygonShape boxShape;
    boxShape.SetAsBox(width/2, height/2, b2Vec2(px, py), angle);
    
    b2FixtureDef boxFixtureDef;
    boxFixtureDef.shape = &boxShape;
    boxFixtureDef.density = 5;
    //boxFixtureDef.restitution = 1;
    
    body->CreateFixture(&boxFixtureDef);
}

void HelloWorld::ccTouchesBegan(CCSet* touches, CCEvent* event)
{
    int r = rand()%128+128;
    int b = rand()%128+128;
    int g = rand()%128+128;
    brush->setColor(ccc3(r, b, g));
    
    CCTouch *touch = (CCTouch *)touches->anyObject();
    plataformPoints.clear();
    
    CCPoint location = touch->getLocation();
    
    plataformPoints.push_back(location);
    previousLocation = location;
    
    b2BodyDef myBodyDef;
    myBodyDef.type = b2_staticBody;
    myBodyDef.position.Set(location.x/PTM_RATIO,location.y/PTM_RATIO);
    currentPlatformBody = world->CreateBody(&myBodyDef);
}

void HelloWorld::ccTouchesMoved(CCSet* touches, CCEvent* event)
{
    CCTouch *touch = (CCTouch *)touches->anyObject();
    CCPoint start = touch->getLocation();
    CCPoint end = touch->getPreviousLocation();
    
    target->begin();
    
    float distance = ccpDistance(start, end);
    
    for (int i = 0; i < distance; i++)
    {
        float difx = end.x - start.x;
        float dify = end.y - start.y;
        float delta = (float)i / distance;
        brush->setPosition(ccp(start.x + (difx * delta), start.y + (dify * delta)));
        brush->visit();
    }
    target->end();
    
    float distance2 = ccpDistance(start, previousLocation);
    if(distance2 > 15.0f)
    {
        //this->addRectangle(currentPlatformBody, previousLocation, start);
        plataformPoints.push_back(start);
        previousLocation = start;
    }
}

void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    CCSize s = CCDirector::sharedDirector()->getWinSize();

    if (plataformPoints.size()>1) {
        //Add a new body/atlas sprite at the touched location
        b2BodyDef myBodyDef;
        myBodyDef.type = b2_dynamicBody; //this will be a dynamic body
        myBodyDef.position.Set(currentPlatformBody->GetPosition().x, currentPlatformBody->GetPosition().y); //set the starting position
        b2Body* newBody = world->CreateBody(&myBodyDef);
        
        for(int i=0; i < plataformPoints.size()-1; i++)
        {
            CCPoint start = plataformPoints[i];
            CCPoint end = plataformPoints[i+1];
            this->addRectangle(newBody, start, end);
        }
        
        world->DestroyBody(currentPlatformBody);
        
        CCRect bodyRectangle = this->getBodyRectangle(newBody);
        
        CCImage *pImage = target->newCCImage();
        CCTexture2D *tex = CCTextureCache::sharedTextureCache()->addUIImage(pImage,NULL);
        CC_SAFE_DELETE(pImage);
        CCSprite *sprite = CCSprite::createWithTexture(tex, bodyRectangle);
        
        float anchorX = newBody->GetPosition().x * PTM_RATIO - bodyRectangle.origin.x;
        float anchorY = bodyRectangle.size.height - (s.height - bodyRectangle.origin.y - newBody->GetPosition().y * PTM_RATIO);
        
        sprite->setAnchorPoint(ccp(anchorX / bodyRectangle.size.width,  anchorY / bodyRectangle.size.height));
        newBody->SetUserData(sprite);
        boxLayer->addChild(sprite);
    }
    
    boxLayer->removeChild(target, true);
    target->release();
    
    target = CCRenderTexture::create(s.width, s.height, kCCTexture2DPixelFormat_RGBA8888);
    target->retain();
    target->setPosition(ccp(s.width / 2, s.height / 2));
    boxLayer->addChild(target);
}

CCRect HelloWorld::getBodyRectangle(b2Body* body)
{
    CCSize s = CCDirector::sharedDirector()->getWinSize();
    
    float minX = s.width;
    float maxX = 0;
    float minY = s.height;
    float maxY = 0;
    
    const b2Transform& xf = body->GetTransform();
    for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
    {
        b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
        int32 vertexCount = poly->m_vertexCount;
        b2Assert(vertexCount <= b2_maxPolygonVertices);
        
        for (int32 i = 0; i < vertexCount; ++i)
        {
            b2Vec2 vertex = b2Mul(xf, poly->m_vertices[i]);
            
            minX = MIN(minX, vertex.x);
            maxX = MAX(maxX, vertex.x);
            minY = MIN(minY, vertex.y);
            maxY = MAX(maxY, vertex.y);
        }
    }
    
    maxX *= PTM_RATIO;
    minX *= PTM_RATIO;
    maxY *= PTM_RATIO;
    minY *= PTM_RATIO;
    
    // ブラシのぼかしが入るように補正
    float margin = brush->getContentSize().width * brush->getScale() / 2.0f;
    
    float width  = maxX - minX + margin * 2;
    float height = maxY - minY + margin * 2;
    
    float x = minX - margin;
    float y = s.height - maxY - margin;
    
    // 画面外に出ないように補正
    x = MAX(0.0f, x);
    y = MAX(0.0f, y);
    if (minX+width > s.width) {
        width  = s.width - x;
    }
    if (minY+height > s.height) {
        height = s.height - y;
    }
    
    return CCRectMake(x, y, width, height);
}

void HelloWorld::clearBox() {
    // フラグを立てて次の画面更新のタイミングで画面クリアと box2d データをクリアする
    this->m_bClearBox = true;
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // add layer as a child to scene
    CCLayer* layer = new HelloWorld();
    scene->addChild(layer);
    layer->release();
    
    return scene;
}
