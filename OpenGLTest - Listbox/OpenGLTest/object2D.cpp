//
// Created by magpayor on 10/11/2017.
//

#include "object2D.h"

Object2D::Object2D()
        : _objectState(OBJECT2D_STATE_IDLE),
         _animationType(OBJECT2D_ANIMATION_NONE),
          _alphaSpeed(5.0f), _speedZoom(10.0f),
          _speedMove(0.0f), _objectID(0)
{
    //...
}
Object2D::~Object2D()
{}

void Object2D::Update(float elapsed)
{
    if( _objectState == OBJECt2D_STATE_ANIMATE){
        if(_animationType & OBJECT2D_ANIMATION_FADE_IN) {
            if(_alpha < _targetAlpha ) {
                _alpha += (_alphaSpeed * elapsed);
            }else{
                _alpha = _targetAlpha;
                _animationType ^= OBJECT2D_ANIMATION_FADE_IN;
                OnFinishAnimation(OBJECT2D_ANIMATION_FADE_IN);
            }

        }if(_animationType & OBJECT2D_ANIMATION_FADE_OUT) {
            if(_alpha > 0.0f ) {
                _alpha -= (_alphaSpeed * elapsed);
            }else{
                _alpha = _targetAlpha;
                _animationType ^= OBJECT2D_ANIMATION_FADE_OUT;
                OnFinishAnimation(OBJECT2D_ANIMATION_FADE_OUT);
            }

        }

        if(_animationType & OBJECT2D_ANIMATION_ZOOM_OUT) {
            if( _height < _targetHeight){
                _height += (_speedZoom * elapsed);
            }else{
                _width = _targetWidth;
                _height = _targetHeight;

                _animationType ^= OBJECT2D_ANIMATION_ZOOM_OUT;
                OnFinishAnimation(OBJECT2D_ANIMATION_ZOOM_OUT);
            }

            _width = _ratio * _height;

            //adjust coordinates
            _x = _centerX - (_width/2);
            _y = _centerY -(_height/2);

        }
        if(_animationType & OBJECT2D_ANIMATION_ZOOM_IN) {
            if( _height > _targetHeight){
                _height -= (_speedZoom * elapsed);
            }else{
                _width = _targetWidth;
                _height = _targetHeight;

                _animationType ^= OBJECT2D_ANIMATION_ZOOM_IN;
                OnFinishAnimation(OBJECT2D_ANIMATION_ZOOM_IN);
            }

            _width = _ratio * _height;

            //adjust coordinates
            _x = _centerX - (_width/2);
            _y = _centerY -(_height/2);
        }

        if(_animationType & OBJECT2D_ANIMATION_MOVEX) {
            if(_directionX == 1 && _x < _targetX){
                _x += (_speedMove * elapsed);
            }else if( _directionX == 0 && _x > _targetX){
                _x -= (_speedMove * elapsed);
            }else{
                _x = _targetX;
                _animationType ^= OBJECT2D_ANIMATION_MOVEX;
                OnFinishAnimation(OBJECT2D_ANIMATION_MOVEX);
            }
        }
        if(_animationType & OBJECT2D_ANIMATION_MOVEY) {
            if(_directionY == 1 && _y < _targetY){
                _y += (_speedMove * elapsed);
            }else if( _directionY == 0 && _y > _targetY){
                _y -= (_speedMove * elapsed);
            }else{
                _y = _targetY;
                _animationType ^= OBJECT2D_ANIMATION_MOVEY;
                OnFinishAnimation(OBJECT2D_ANIMATION_MOVEY);
            }
        }

        if(_animationType == OBJECT2D_ANIMATION_NONE) {
            _objectState = OBJECT2D_STATE_IDLE;
            OnCompleteAnimation();
        }
    }
}

void Object2D::Draw()
{}

int Object2D::GetWidth()
{
    return _width;
}
int Object2D::GetHeight()
{
    return _height;
}

void Object2D::SetPosition(int x, int y)
{
    _x = x;
    _y = y;
}

void Object2D::SetAlpha(float alpha)
{
    _alpha = alpha;
}
void Object2D::SetID(int id)
{
    _objectID = id;
}

int Object2D::GetID()
{
    return _objectID;
}

void Object2D::SetDimension(int width, int height)
{
    _width = width;
    _height = height;
}

void Object2D::StartAnimation()
{
    if( _animationType != OBJECT2D_ANIMATION_NONE ) {
        _objectState = OBJECt2D_STATE_ANIMATE;

        if( (_animationType & OBJECT2D_ANIMATION_MOVEX) ||
            (_animationType & OBJECT2D_ANIMATION_MOVEY) )
        {
            _directionX = ( _x < _targetX)? 1 : 0;
            _directionY = ( _y < _targetY)? 1 : 0;
        }
    }
}

void Object2D::OnFinishAnimation(int type)
{
    //...
}

void Object2D::OnCompleteAnimation()
{
    //..
}