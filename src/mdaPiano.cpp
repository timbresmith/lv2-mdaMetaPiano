/* ==================================================
 * LV2 port of the famous mda Piano VSTi
 * ==================================================
 *
 * Port
 *   Author:    Ricardo Wurmus (rekado)
 *   Based on:  mda-vst-src-2010-02-14.zip
 *
 * mda Piano v1.0
 *   Copyright(c)1999-2000 Paul Kellett (maxim digital audio)
 *   Based on VST2 SDK (c)1996-1999 Steinberg Soft und Hardware GmbH, All Rights Reserved
 *
 * ==================================================
 */

#include "mdaPianoCommon.h"
#include "mdaPiano.h"
#include <cstdlib> //for exit

#include <stdio.h>
#include <math.h>

#define STRING_BUF 2048
static const char* sample_file = "samples.raw";


mdaPiano::mdaPiano(double rate)
  : LV2::Synth<mdaPianoVoice, mdaPiano>(p_n_ports, p_midi) {

  cmax = 0x7F;  //just in case...

  load_samples(&waves);
  load_kgrp(kgrp);

  for(uint32_t i=0; i<NVOICES; ++i) {
    voices[i] = new mdaPianoVoice(rate, waves, kgrp);
    add_voices(voices[i]);
  }

  notes[0] = EVENTS_DONE;
  sustain = activevoices = 0;

  update();
}

//parameter change
void mdaPiano::update() {
  for (uint32_t v=0; v<NVOICES; ++v) {
    voices[v]->update(Current);
  }
}


unsigned mdaPiano::find_free_voice(unsigned char key, unsigned char velocity) {
  //is this a retriggered note during sustain?
  if (sustain) {
    for (unsigned i = 0; i < NVOICES; ++i) {
      if ((voices[i]->get_key() == key) && (voices[i]->is_sustained()))
        return i;
    }
  }

  //take the next free voice if
  // ... notes are sustained but not this new one
  // ... notes are not sustained
  for (unsigned i = 0; i < NVOICES; ++i) {
    if (voices[i]->get_key() == LV2::INVALID_KEY)
    {
      return i;
    }
  }

  //TODO: steal quietest note if all voices are used up
  return 0;
}


void mdaPiano::setVolume(float value)
{
  for (uint32_t v=0; v<NVOICES; ++v)
    voices[v]->set_volume(value);
}


void mdaPiano::setParameter(unsigned char id, float value)
{
  if(id>=NPARAMS)
    return;
  *p(id+PARAM_OFFSET) = value;
  update();
#ifdef DEBUG
  printf("changed %i to %f\n", id, value);
#endif
}


void mdaPiano::handle_midi(uint32_t size, unsigned char* data) {
#ifdef DEBUG
  printf("%d\n", data[1]);
#endif

  //discard invalid midi messages
  if (size != 3)
    return;

  //receive on all channels
  switch(data[0] & 0xf0)
  {
    case 0x80: //note off
      notes[npos++] = data[1] & 0x7F; //note
      notes[npos++] = 0;                  //vel
      break;

    case 0x90: //note on
      notes[npos++] = data[1] & 0x7F; //note
      notes[npos++] = data[2] & 0x7F; //vel
      break;

    case 0xB0: //controller
      switch(data[1])
      {
        case 0x01:  //mod wheel
        case 0x43:  //soft pedal
          {
            float muff = 0.01f * (float)((127 - data[2]) * (127 - data[2]));
            for (unsigned i = 0; i < NVOICES; ++i) {
              voices[i]->set_muff(muff);
            }
            break;
          }

        case 0x07:  //volume
          setVolume(0.00002f * (float)(data[2] * data[2]));
          break;

        case 0x40:  //sustain pedal
        case 0x42:  //sustenuto pedal
          sustain = data[2] & 0x40;
          if(sustain==0)
          {
            notes[npos++] = SUSTAIN; //end all sustained notes
            notes[npos++] = 0;
          }
          break;

        //all sound off
        case 0x78:
        //all notes off
        case 0x7b:
        default:
          for(short v=0; v<NVOICES; v++) {
            voices[v]->reset();
          }
          break;
      }
      break;

    default: break;
  }
}


// TODO: load this from a file
void mdaPiano::load_kgrp(KGRP *kgrp)
{
  //Waveform data and keymapping is hard-wired in *this* version
  kgrp[ 0].root = 36;  kgrp[ 0].high = 37;  kgrp[ 0].pos = 0;       kgrp[ 0].end = 36275;   kgrp[ 0].loop = 14774;
  kgrp[ 1].root = 40;  kgrp[ 1].high = 41;  kgrp[ 1].pos = 36278;   kgrp[ 1].end = 83135;   kgrp[ 1].loop = 16268;
  kgrp[ 2].root = 43;  kgrp[ 2].high = 45;  kgrp[ 2].pos = 83137;   kgrp[ 2].end = 146756;  kgrp[ 2].loop = 33541;
  kgrp[ 3].root = 48;  kgrp[ 3].high = 49;  kgrp[ 3].pos = 146758;  kgrp[ 3].end = 204997;  kgrp[ 3].loop = 21156;
  kgrp[ 4].root = 52;  kgrp[ 4].high = 53;  kgrp[ 4].pos = 204999;  kgrp[ 4].end = 244908;  kgrp[ 4].loop = 17191;
  kgrp[ 5].root = 55;  kgrp[ 5].high = 57;  kgrp[ 5].pos = 244910;  kgrp[ 5].end = 290978;  kgrp[ 5].loop = 23286;
  kgrp[ 6].root = 60;  kgrp[ 6].high = 61;  kgrp[ 6].pos = 290980;  kgrp[ 6].end = 342948;  kgrp[ 6].loop = 18002;
  kgrp[ 7].root = 64;  kgrp[ 7].high = 65;  kgrp[ 7].pos = 342950;  kgrp[ 7].end = 391750;  kgrp[ 7].loop = 19746;
  kgrp[ 8].root = 67;  kgrp[ 8].high = 69;  kgrp[ 8].pos = 391752;  kgrp[ 8].end = 436915;  kgrp[ 8].loop = 22253;
  kgrp[ 9].root = 72;  kgrp[ 9].high = 73;  kgrp[ 9].pos = 436917;  kgrp[ 9].end = 468807;  kgrp[ 9].loop = 8852;
  kgrp[10].root = 76;  kgrp[10].high = 77;  kgrp[10].pos = 468809;  kgrp[10].end = 492772;  kgrp[10].loop = 9693;
  kgrp[11].root = 79;  kgrp[11].high = 81;  kgrp[11].pos = 492774;  kgrp[11].end = 532293;  kgrp[11].loop = 10596;
  kgrp[12].root = 84;  kgrp[12].high = 85;  kgrp[12].pos = 532295;  kgrp[12].end = 560192;  kgrp[12].loop = 6011;
  kgrp[13].root = 88;  kgrp[13].high = 89;  kgrp[13].pos = 560194;  kgrp[13].end = 574121;  kgrp[13].loop = 3414;
  kgrp[14].root = 93;  kgrp[14].high = 999; kgrp[14].pos = 574123;  kgrp[14].end = 586343;  kgrp[14].loop = 2399;
}


void mdaPiano::load_samples(short **buffer)
{
  FILE *f;
  long num, size;
  char filepath[STRING_BUF];

  strncpy(filepath, bundle_path(), STRING_BUF);
  strncat(filepath,
          sample_file,
          STRING_BUF - strlen(filepath));
  f = fopen(filepath, "rb");
  if (f == NULL) {
    fputs("File error", stderr);
    exit(1);
  }

  // obtain file size
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f);

  // allocate memory to contain the whole file
  *buffer = (short*) malloc (sizeof(short)*size);
  if (*buffer == NULL) {
    fputs("Memory error", stderr);
    exit(2);
  }

  // copy the file into the buffer
  num = fread(*buffer, 1, size, f);
  if (num != size) {
    fputs ("Reading error", stderr);
    exit (3);
  }
  fclose (f);
  return;
}
