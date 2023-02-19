#include "notes.hpp"

#include <bitset>
#include <iostream>
#include <cstdlib>
short past=0;
short* randtone = ( short*)malloc(65536);

unsigned int sfreq = 48000;

short* randtonegen(short mod, double volume, unsigned int *len,int smode){
    unsigned short lfsr = 2;
    unsigned short xorr;
    unsigned char high = 0;
    randtone = ( short*)malloc(65536);
    volume /=4;

    for(int i=0;i<32768;++i){
            if(i%mod == 0){
                xorr = ((lfsr & 0b10) >> 1) ^ (lfsr&0b1);
                lfsr >>= 1;
                lfsr |= xorr<<14;
                if(smode){
                    unsigned short mask2 = 0b111111110111111;
                 //  lfsr = (lfsr & mask2);
                 //   lfsr |= xorr<<6;
                }
            }
            randtone[i] = (!(lfsr&1))*32767*volume;

      //  printf("%i\n",randtone[i]);
    }
    *len = 65536;
    //chunks.push_back(Mix_QuickLoad_RAW((unsigned char*)tone,65536));
    return randtone;
}

short* sqrtonegen(unsigned int freq, unsigned int duty, double volume, unsigned int *len){
    short* buf = ( short* )malloc(sfreq/freq*2);
    volume /=4;
    for(int i=0; i<sfreq/freq;++i){
        if(i<sfreq/freq*duty/100){
            buf[i]=32767 * volume;
        }
        else buf[i]=-32767 * volume;
    }

    *len = sfreq/freq*2;
    //chunks.push_back(Mix_QuickLoad_RAW((unsigned char*)buf,sfreq/freq*2));
    return buf;
}

short* tritonegen(unsigned int freq, double volume, unsigned int *len){
    short* buf = ( short* )malloc(((sfreq/freq) + (sfreq/freq))*2);
    volume /=4;

    double step = (32768/(sfreq/freq/2)) * volume;
    double counter = 0;

    for(int dir=0; dir<2; ++dir){
        for(int i=(sfreq/freq*dir)+5; i<sfreq/freq+(sfreq/freq*dir)+5;++i){
            if((i-(sfreq/freq*dir)) < (sfreq/freq/2)){
                buf[i-5] = counter = (dir==1 ? counter + step : counter - step);

            }
            else{
                buf[i-5] = counter = (dir==1 ? counter - step : counter + step);
            }
        }
    }
    *len = ((sfreq/freq) + (sfreq/freq))*2 ;
    //chunks.push_back(Mix_QuickLoad_RAW((unsigned char*)buf,sfreq/freq*2));
    return buf;
}
