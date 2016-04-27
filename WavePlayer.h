#ifndef _WAVEPLAYER_H_
#define _WAVEPLAYER_H_


class WavePlayer {
public:
    static const int CHANNEL_NUM = 2;
    static const int FREQUENCY = 44100;
    static const size_t SAMPLE_MAX = FREQUENCY * 5;
    Buffer buf; // monoral!
    WavePlayer();
};

#endif

