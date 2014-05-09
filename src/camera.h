// camera.h

#include "geometry.h"
#include "ftimer.h"

struct camera_t 
{
    mat16_t rotation;
    vec3_t origin;
    float heading;      // 0 - 360
    float pitch;        // 180 - 0
    
    static const float heading_inc;
    static const float pitch_inc;
    static const float roll_inc;
    static const float move_amt;
    static const int update_ms;
    static const float p180;
    static const int accel_ms;
    static const int decel_ms;
    
    enum {
        MV_FORWARD,
        MV_BACKWARD,
        MV_LEFT,
        MV_RIGHT,
    };

    ftimer_t timer[4];
    ftimer_t accel[4];
    unsigned char keys[4];
    vec4_t move;
    vec4_t last_velocity;

    camera_t() : origin(-15.f,-60.f,-40.f) {
        heading = 32.f;
        pitch = 23.0f;
        keys[0] = keys[1] = keys[2] = keys[3] = 0;
        mouseCursorMove(0,0);
        last_velocity.zero();
        move.zero();
    }

    void keyUp( int );
    void keyDownForward();
    void keyDownBackward();
    void keyDownLeft();
    void keyDownRight();

    void keyUpForward();
    void keyUpBackward();
    void keyUpLeft();
    void keyUpRight();

    void updateHeadingAngle( int );      // around X-axis, pitch
    void updatePitchAngle( int );      // around Y-axis, heading/yaw
    void rotateNStepsZ( int );      // roll or bank
    void mouseCursorMove( int, int );

    void setkey( int );

    // 
    void update( void );
};
