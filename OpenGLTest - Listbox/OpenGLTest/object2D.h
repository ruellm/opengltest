//
// Created by magpayor on 10/11/2017.
//

#ifndef RAMO_OBJECT2D_H
#define RAMO_OBJECT2D_H

typedef enum _object2DState
{
    OBJECT2D_STATE_IDLE,
    OBJECt2D_STATE_ANIMATE
}Object2DState;


#define    OBJECT2D_ANIMATION_NONE      0
#define    OBJECT2D_ANIMATION_ZOOM_IN   1
#define    OBJECT2D_ANIMATION_FADE_IN   2
#define    OBJECT2D_ANIMATION_FADE_OUT  4
#define    OBJECT2D_ANIMATION_ZOOM_OUT  8
#define    OBJECT2D_ANIMATION_MOVEX     16
#define    OBJECT2D_ANIMATION_MOVEY     32
    //... add more

class Object2D {
public:
    Object2D();
    ~Object2D();

    int _x, _y;
    int _width, _height;
    float _alpha;
    bool _visible;

    virtual void Update(float elapsed);
    virtual void Draw();

    int GetWidth();
    int GetHeight();
    void SetPosition(int x, int y);
    void SetAlpha(float alpha);
    void SetDimension(int width, int height);
    void SetID(int id);
    int GetID();

protected:
    // --------------------------------------------------------------------
    // FOR ANIMATION handling, do we need to create animation controller?
    Object2DState       _objectState;
    int   _animationType;
    int   _objectID;

    //methods
    void StartAnimation();
    virtual void OnFinishAnimation(int type);
    virtual void OnCompleteAnimation();

    // for fade animation
    float _targetAlpha;
    float _alphaSpeed;

    // for zoom animation
    int _targetWidth;
    int _targetHeight;
    int _centerX;
    int _centerY;
    float _speedZoom;
    float _ratio;

    // for move x/y animation
    int _targetX;
    int _targetY;
    float _speedMove;
    // --------------------------------------------------------------------

private:
    int _directionX;
    int _directionY;
};


#endif //RAMO_OBJECT2D_H
