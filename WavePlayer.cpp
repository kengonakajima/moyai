

#include "common.h"
#include "Remote.h"
#include "WavePlayer.h"


WavePlayer::WavePlayer() {
    buf.ensureMemory(SAMPLE_MAX*sizeof(float));
}


