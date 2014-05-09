// camera.cpp

#include "camera.h"

const float camera_t::pitch_inc     = 0.65f;
const float camera_t::heading_inc   = 0.65f;
const float camera_t::roll_inc      = 0.65f;
const float camera_t::move_amt      = 2.15f;
const int   camera_t::update_ms     = 14;
const float camera_t::p180          = 0.0174531266f;
const int   camera_t::accel_ms      = 800;
const int   camera_t::decel_ms      = 1066;

void camera_t::keyDownForward()
{
    setkey( MV_FORWARD );
}

void camera_t::keyDownBackward()
{
    setkey( MV_BACKWARD );
}

void camera_t::keyDownLeft()
{
    setkey( MV_LEFT );
}

void camera_t::keyDownRight()
{
    setkey( MV_RIGHT );
}

void camera_t::keyUp( int k )
{
    // note: dont unset key, there is partial momentum
    timer[k].reset();
    accel[k].set( (int)(last_velocity[k] / move_amt * decel_ms) + update_ms, 2 );
}

void camera_t::keyUpForward()
{
    keyUp( MV_FORWARD );
}

void camera_t::keyUpBackward()
{
    keyUp( MV_BACKWARD );
}

void camera_t::keyUpLeft()
{
    keyUp( MV_LEFT );
}

void camera_t::keyUpRight()
{
    keyUp( MV_RIGHT );
}

void camera_t::updatePitchAngle( int steps )
{
    if ( steps == 0 )
        return;

    pitch += pitch_inc * steps;
    if ( pitch > 90.0f ) {
        pitch = 90.0f;
    }

    if ( pitch < -90.f ) {
        pitch = -90.f;
    }
}

void camera_t::updateHeadingAngle( int steps ) 
{
    if ( 0 == steps )
        return;

    heading += heading_inc * steps;

    if ( heading > 360.0f ) {
        heading = heading - 360.0f;
    }
    if ( heading < 0.f ) {
        heading = 360.0f + heading;
    }
}

void camera_t::rotateNStepsZ( int steps )
{
    //rotation.rotateZ( steps * roll_inc );
}

/* 
mouse delta-y: - to +, top-to-bottom
mouse delta-x: - to +, left-to-right 
*/
void camera_t::mouseCursorMove( int dx, int dy )
{
    // around-Y (heading) has to be done first
    // then pitch, to preserve a level perspective (untransformed around z)
    updateHeadingAngle( dy ); 
    updatePitchAngle( dx ); 

    rotation.identity();
    rotation.rotate( pitch, 1.f, 0.f, 0.f );
    rotation.rotate( heading, 0.f, 1.f, 0.f );
}

void camera_t::setkey( int k )
{
    // unset contrary keys
    switch ( k ) {
    case MV_FORWARD:
        keys[MV_BACKWARD] = 0; accel[MV_BACKWARD].stop(); 
        break;
    case MV_BACKWARD:
        keys[MV_FORWARD] = 0; accel[MV_FORWARD].stop();
        break;
    case MV_LEFT:
        keys[MV_RIGHT] = 0; accel[MV_RIGHT].stop();
    case MV_RIGHT:
        keys[MV_LEFT] = 0; accel[MV_LEFT].stop();
        break;
    default:
        return;
    }

    // set the key
    keys[k] = 1;

    // change the flag no matter what
    accel[k].flags = 1;

    // preserve incremental velocity if we can
    if ( accel[k].timeup() ) 
        accel[k].set( accel_ms );
}

void camera_t::update( void )
{
    // set move amounts
    for ( int i = 0; i < 4; i++ ) 
    {
        // dampen during acceleration and decelleration
        if ( !accel[i].timeup() )
        {
            // decelleration
            if ( accel[i].flags == 2 ) {
                move[i] = move_amt * ( 1.0f - accel[i].ratio() ); // linear
            } 
            // acceleration
            else {
                // tanh((x+0.2)^2*3)
                float r = accel[i].ratio() + 0.2f;
                move[i] = move_amt * tanh ( r * r * 3.f );
            }
        }
        else {
            move[i] = move_amt; // full power
        }

/*
        move[i] = !accel[i].timeup() && accel[i].flags == 2 ? 
                  move_amt * ( 1.0f - accel[i].ratio() ) : 
                  !accel[i].timeup() ? accel[i].ratio() * move_amt : 
                  move_amt;
*/

        // unset keys when decel timers are done
        if ( accel[i].flags == 2 && accel[i].timeup() )
            keys[i] = 0, accel[i].flags = 1;
    }


    if ( keys[MV_FORWARD] && timer[MV_FORWARD].check() ) {
        origin[0] += rotation[8] * move[MV_FORWARD];
        origin[1] += rotation[9] * move[MV_FORWARD];
        origin[2] += rotation[10] * move[MV_FORWARD];
        timer[MV_FORWARD].set( update_ms );
        last_velocity[MV_FORWARD] = move[MV_FORWARD];
    } 

    if ( keys[MV_BACKWARD] && timer[MV_BACKWARD].check() ) {
        origin[0] -= rotation[8] * move[MV_BACKWARD];
        origin[1] -= rotation[9] * move[MV_BACKWARD];
        origin[2] -= rotation[10] * move[MV_BACKWARD];
        timer[MV_BACKWARD].set( update_ms );
        last_velocity[MV_BACKWARD] = move[MV_BACKWARD];
    }
    if ( keys[MV_LEFT] && timer[MV_LEFT].check() ) {
        origin[0] += rotation[0] * move[MV_LEFT];
        origin[1] += rotation[1] * move[MV_LEFT];
        origin[2] += rotation[2] * move[MV_LEFT];
        timer[MV_LEFT].set( update_ms );
        last_velocity[MV_LEFT] = move[MV_LEFT];
    }
    if ( keys[MV_RIGHT] && timer[MV_RIGHT].check() ) {
        origin[0] -= rotation[0] * move[MV_RIGHT];
        origin[1] -= rotation[1] * move[MV_RIGHT];
        origin[2] -= rotation[2] * move[MV_RIGHT];
        timer[MV_RIGHT].set( update_ms );
        last_velocity[MV_RIGHT] = move[MV_RIGHT];
    }

}

