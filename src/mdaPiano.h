#ifndef MDA_PIANO_H
#define MDA_PIANO_H
//See associated .cpp file for copyright and other info

#include "mdaPianoVoice.h"
#include "mdaPiano.peg"

#pragma GCC system_header
#include <lv2synth.hpp>


class mdaPiano : public LV2::Synth<mdaPianoVoice, mdaPiano> {
public:
  mdaPiano(double);
  ~mdaPiano() {
    for (unsigned char i = 0; i < NSAMPLES; i++) {
      free(samples[i].buffer);
    }
    free(samples);
  }

  bool sustain;
  KGRP kgrp[NSAMPLES];
  mdaPianoVoice *voices[NVOICES];
  Sample *samples = (Sample*) malloc (NSAMPLES * sizeof(Sample));

  void load_kgrp(KGRP*);
  void load_sample(Sample*, const char*);

  unsigned find_free_voice(unsigned char, unsigned char);
  void handle_midi(uint32_t, unsigned char*);
  void setVolume(float);
  void update(void);
};

#endif
