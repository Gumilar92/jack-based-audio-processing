#include <jack/jack.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include "kaiserbessel.h"
using namespace std;

#define RATE 48000
#define PI 3.141592653

//Constructor
kaiser_bessel::kaiser_bessel(){
    this->att = 60;
    this->fa = 0;
    this->fb = RATE /2;
    this->length = 51;
}

kaiser_bessel::kaiser_bessel(int fa, int fb){
    this->att = 60;

    if(fa > RATE/2){
        this->fa = RATE/2;
    }else if(fa < 0){
        this->fa = 0;
    }else{
        this->fa = fa;
    }

    if(fb > RATE/2){
        this->fb = RATE/2;
    }else if(fb < 0){
        this->fb = 0;
    }else{
        this->fb = fb;
    }

    this->length = 51;
}

kaiser_bessel::kaiser_bessel(int fa, int fb, int att,int length){
    this->att = att;

    if(fa > RATE/2){
        this->fa = RATE/2;
    }else if(fa < 0){
        this->fa = 0;
    }else{
        this->fa = fa;
    }

    if(fb > RATE/2){
        this->fb = RATE/2;
    }else if(fb < 0){
        this->fb = 0;
    }else{
        this->fb = fb;
    }

    this->length = length;
}
//Setter
void kaiser_bessel::set_fa(int fa){this->fa = fa;}
void kaiser_bessel::set_fb(int fb){this->fb = fb;}
void kaiser_bessel::set_att(int att){this->att = att;}
void kaiser_bessel::set_length(int length){this->length = length;}
//Getter
int kaiser_bessel::get_fa(){return fa;}
int kaiser_bessel::get_fb(){return fb;}
int kaiser_bessel::get_att(){return att;}
int kaiser_bessel::get_length(){return length;}

float kaiser_bessel::calculate_alpha(int att){

    float alpha;
        if(att < 21)
        {
            alpha = 0.0;
        }
        else if (att <= 50)
        {
            alpha = 0.5842 * pow( (att-21) , 0.4) + 0.07886 * (att-21);
        }
        else
        {
            alpha = 0.1102 * (att - 8.7);
        }

        return alpha;
}

float kaiser_bessel::ino(float alpha){

    int d = 0;
        float ds = 1.0;
        float s = 1;

        do
        {
            d += 2;
            ds *= (alpha * alpha) / (d*d);
            s += ds;
        }
        while (ds > s*1e-6);

        return s;

}

void kaiser_bessel::calculate_impulse_response(float impulse_response[]){

    int Np = (length-1)/2;

    impulse_response[0] = 2.0* (fb - fa) / RATE;

    for(int i = 1; i<= Np; i++)
    {
        impulse_response[i] = (sin(2*i*PI*fb/RATE) - sin(2*i*PI*fa/RATE)) / (i*PI);
    }

}

void kaiser_bessel::calculate_coefficients(float coefficients[]){

    float impulse_response[length];
    calculate_impulse_response(impulse_response);

    float alpha = calculate_alpha(att);
    float inoalpha = ino(alpha);

    int Np = (length-1)/2;

    for(int j = 0; j <= Np; j++)
    {
         coefficients[j + Np] = impulse_response[j] * ino (alpha * sqrt(1-(j*j/(Np*Np)))) / inoalpha;
    }
    for(int k = 0; k < Np; k++)
    {
        coefficients[k] = coefficients[length-1-k];
    }

}
