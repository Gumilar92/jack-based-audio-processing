#ifndef KAISERBESSEL_H
#define KAISERBESSEL_H

#include <jack/jack.h>
#include <math.h>
#include <stdlib.h>

class kaiser_bessel{

    int fa, fb, att, length ;

public :
    kaiser_bessel();
    kaiser_bessel(int fa, int fb);
    kaiser_bessel(int fa, int fb, int att, int length);
    float calculate_alpha(int att);
    float ino(float alpha);
    void calculate_impulse_response(float impulse_response[]);
    void calculate_coefficients(float coefficients[]);
    void set_fa(int fa);
    void set_fb(int fb);
    void set_att(int att);
    void set_length(int length);
    int get_fa();
    int get_fb();
    int get_att();
    int get_length();
};


#endif // KAISERBESSEL_H
