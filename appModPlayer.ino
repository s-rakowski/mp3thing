#define BITDEPTH 16                               // 11 bits PWM
#define SAMPLERATE 44100 //(SYSCLK / (1 << BITDEPTH))     // Best audio quality 96MHz / (1 << 11) = 46875Hz

#define SOUNDBUFFERSIZE 16                      // Stereo circular sound buffer 2 * 8192 = 16384 bytes of memory
#define FATBUFFERSIZE 2048                       // File system buffers CHANNELS * 4096 = 73728 bytes of memory
#define DIVIDER 10                                // Fixed-point mantissa used for integer arithmetic
#define STEREOSEPARATION 32                       // 0 (max) to 64 (mono)

// Hz = 7093789 / (amigaPeriod * 2) for PAL
// Hz = 7159091 / (amigaPeriod * 2) for NTSC
#define AMIGA (7093789 / 2 / SAMPLERATE << DIVIDER)
// Mixer.channelFrequency[channel] = AMIGA / amigaPeriod

#define ROWS 64
#define SAMPLES 31
#define CHANNELS 8
#define NONOTE 0xFFFF

typedef struct {
  char name[22];
  uint16_t length;
  int8_t fineTune;
  uint8_t volume;
  uint16_t loopBegin;
  uint16_t loopLength;
} 
Sample;

struct {
  char name[20];
  Sample samples[SAMPLES];
  uint8_t songLength;
  uint8_t numberOfPatterns;
  uint8_t order[128];
  uint8_t numberOfChannels;
} 
Mod;

typedef struct {
  uint8_t sampleNumber[ROWS][CHANNELS];
  uint16_t note[ROWS][CHANNELS];
  uint8_t effectNumber[ROWS][CHANNELS];
  uint8_t effectParameter[ROWS][CHANNELS];
} 
Pattern;

struct {
  Pattern currentPattern;

  UINT amiga;
  uint16_t samplesPerTick;
  uint8_t speed;
  uint8_t tick;
  uint8_t row;
  uint8_t lastRow;

  uint8_t orderIndex;
  uint8_t oldOrderIndex;
  uint8_t patternDelay;
  uint8_t patternLoopCount[CHANNELS];
  uint8_t patternLoopRow[CHANNELS];

  uint8_t lastSampleNumber[CHANNELS];
  int8_t volume[CHANNELS];
  uint16_t lastNote[CHANNELS];
  uint16_t amigaPeriod[CHANNELS];
  int16_t lastAmigaPeriod[CHANNELS];

  uint16_t portamentoNote[CHANNELS];
  uint8_t portamentoSpeed[CHANNELS];

  uint8_t waveControl[CHANNELS];

  uint8_t vibratoSpeed[CHANNELS];
  uint8_t vibratoDepth[CHANNELS];
  int8_t vibratoPos[CHANNELS];

  uint8_t tremoloSpeed[CHANNELS];
  uint8_t tremoloDepth[CHANNELS];
  int8_t tremoloPos[CHANNELS];
} 
Player;

struct {
  UINT sampleBegin[SAMPLES];
  UINT sampleEnd[SAMPLES];
  UINT sampleloopBegin[SAMPLES];
  uint16_t sampleLoopLength[SAMPLES];
  UINT sampleLoopEnd[SAMPLES];

  uint8_t channelSampleNumber[CHANNELS];
  UINT channelSampleOffset[CHANNELS];
  uint16_t channelFrequency[CHANNELS];
  uint8_t channelVolume[CHANNELS];
  uint8_t channelPanning[CHANNELS];
} 
Mixer;

struct {
  uint8_t channels[CHANNELS][FATBUFFERSIZE];
  UINT samplePointer[CHANNELS];
  uint8_t channelSampleNumber[CHANNELS];
} 
FatBuffer;

struct {
  uint16_t left[SOUNDBUFFERSIZE];
  uint16_t right[SOUNDBUFFERSIZE];
  uint16_t writePos;
  volatile uint16_t readPos;
} 
SoundBuffer;

// Effects
#define ARPEGGIO              0x0
#define PORTAMENTOUP          0x1
#define PORTAMENTODOWN        0x2
#define TONEPORTAMENTO        0x3
#define VIBRATO               0x4
#define PORTAMENTOVOLUMESLIDE 0x5
#define VIBRATOVOLUMESLIDE    0x6
#define TREMOLO               0x7
#define SETCHANNELPANNING     0x8
#define SETSAMPLEOFFSET       0x9
#define VOLUMESLIDE           0xA
#define JUMPTOORDER           0xB
#define SETVOLUME             0xC
#define BREAKPATTERNTOROW     0xD
#define SETSPEED              0xF

// 0xE subset
#define SETFILTER             0x0
#define FINEPORTAMENTOUP      0x1
#define FINEPORTAMENTODOWN    0x2
#define GLISSANDOCONTROL      0x3
#define SETVIBRATOWAVEFORM    0x4
#define SETFINETUNE           0x5
#define PATTERNLOOP           0x6
#define SETTREMOLOWAVEFORM    0x7
#define RETRIGGERNOTE         0x9
#define FINEVOLUMESLIDEUP     0xA
#define FINEVOLUMESLIDEDOWN   0xB
#define NOTECUT               0xC
#define NOTEDELAY             0xD
#define PATTERNDELAY          0xE
#define INVERTLOOP            0xF

static const unsigned char riffHeader[] = { //2 channels, 44100Hz, 16 bit
  'R','I','F','F', //0 - id
  0xff,0xff,0xff,0xff, //4 - file size
  'W','A','V','E', //8
  'f','m','t',' ', //12
  0x10,0,0,0, //16 - chunk size
  0x01,0, //20 - format (PCM)
  0x2,0, //22 - channels
  0x44,0xAC,0x00,0x00, //24 - sample rate (44100Hz) (blocks per second)
  0x10,0xB1,0x02,0x00, //28 - data rate (bytes per second)
  0x04,0x00,//32 - block size (1)
  0x10, 0x00,//34 - bits per sample
  'd','a','t','a', //36
  0xff,0xff,0xff,0xff//40 - chunk size
  //44 - start data
};

// Sorted Amiga periods
static const uint16_t amigaPeriods[296] = {
  907, 900, 894, 887, 881, 875, 868, 862, //  -8 to -1
  856, 850, 844, 838, 832, 826, 820, 814, // C-1 to +7
  808, 802, 796, 791, 785, 779, 774, 768, // C#1 to +7
  762, 757, 752, 746, 741, 736, 730, 725, // D-1 to +7
  720, 715, 709, 704, 699, 694, 689, 684, // D#1 to +7
  678, 675, 670, 665, 660, 655, 651, 646, // E-1 to +7
  640, 636, 632, 628, 623, 619, 614, 610, // F-1 to +7
  604, 601, 597, 592, 588, 584, 580, 575, // F#1 to +7
  570, 567, 563, 559, 555, 551, 547, 543, // G-1 to +7
  538, 535, 532, 528, 524, 520, 516, 513, // G#1 to +7
  508, 505, 502, 498, 494, 491, 487, 484, // A-1 to +7
  480, 477, 474, 470, 467, 463, 460, 457, // A#1 to +7
  453, 450, 447, 444, 441, 437, 434, 431, // B-1 to +7
  428, 425, 422, 419, 416, 413, 410, 407, // C-2 to +7
  404, 401, 398, 395, 392, 390, 387, 384, // C#2 to +7
  381, 379, 376, 373, 370, 368, 365, 363, // D-2 to +7
  360, 357, 355, 352, 350, 347, 345, 342, // D#2 to +7
  339, 337, 335, 332, 330, 328, 325, 323, // E-2 to +7
  320, 318, 316, 314, 312, 309, 307, 305, // F-2 to +7
  302, 300, 298, 296, 294, 292, 290, 288, // F#2 to +7
  285, 284, 282, 280, 278, 276, 274, 272, // G-2 to +7
  269, 268, 266, 264, 262, 260, 258, 256, // G#2 to +7
  254, 253, 251, 249, 247, 245, 244, 242, // A-2 to +7
  240, 238, 237, 235, 233, 232, 230, 228, // A#2 to +7
  226, 225, 223, 222, 220, 219, 217, 216, // B-2 to +7
  214, 212, 211, 209, 208, 206, 205, 203, // C-3 to +7
  202, 200, 199, 198, 196, 195, 193, 192, // C#3 to +7
  190, 189, 188, 187, 185, 184, 183, 181, // D-3 to +7
  180, 179, 177, 176, 175, 174, 172, 171, // D#3 to +7
  170, 169, 167, 166, 165, 164, 163, 161, // E-3 to +7
  160, 159, 158, 157, 156, 155, 154, 152, // F-3 to +7
  151, 150, 149, 148, 147, 146, 145, 144, // F#3 to +7
  143, 142, 141, 140, 139, 138, 137, 136, // G-3 to +7
  135, 134, 133, 132, 131, 130, 129, 128, // G#3 to +7
  127, 126, 125, 125, 123, 123, 122, 121, // A-3 to +7
  120, 119, 118, 118, 117, 116, 115, 114, // A#3 to +7
  113, 113, 112, 111, 110, 109, 109, 108  // B-3 to +7
};

static const uint8_t sine[64] = {
  0,  24,  49,  74,  97, 120, 141, 161,
  180, 197, 212, 224, 235, 244, 250, 253,
  255, 253, 250, 244, 235, 224, 212, 197,
  180, 161, 141, 120,  97,  74,  49,  24
};


void loadHeader() {
  UINT count;
  uint8_t i;
  char temp[4];

  f_read(&file, Mod.name, 20, &count);

  for(i = 0; i < SAMPLES; i++) {
    f_read(&file, Mod.samples[i].name, 22, &count);
    f_read(&file, temp, 2, &count);
    Mod.samples[i].length = word(temp[0], temp[1]) * 2;
    f_read(&file, &Mod.samples[i].fineTune, 1, &count);
    if(Mod.samples[i].fineTune > 7) Mod.samples[i].fineTune -= 16;
    f_read(&file, &Mod.samples[i].volume, 1, &count);
    f_read(&file, temp, 2, &count);
    Mod.samples[i].loopBegin = word(temp[0], temp[1]) * 2;
    f_read(&file, temp, 2, &count);
    Mod.samples[i].loopLength = word(temp[0], temp[1]) * 2;
    if(Mod.samples[i].loopBegin + Mod.samples[i].loopLength > Mod.samples[i].length)
      Mod.samples[i].loopLength = Mod.samples[i].length - Mod.samples[i].loopBegin;
  }

  f_read(&file, &Mod.songLength, 1, &count);
  f_read(&file, temp, 1, &count); // Discard this byte

  Mod.numberOfPatterns = 0;
  for(i = 0; i < 128; i++) {
    f_read(&file, &Mod.order[i], 1, &count);
    if(Mod.order[i] > Mod.numberOfPatterns)
      Mod.numberOfPatterns = Mod.order[i];
  }
  Mod.numberOfPatterns++;

  // Offset 1080
  f_read(&file, temp, 4, &count);

  if(!strncmp(temp + 1, "CHN", 3))
    Mod.numberOfChannels = temp[0] - '0';
  else if(!strncmp(temp + 2, "CH", 2))
    Mod.numberOfChannels = (temp[0] - '0') * 10 + temp[1] - '0';
  else
    Mod.numberOfChannels = 4;
}

void loadSamples() {
  uint8_t i;
  UINT fileOffset = 1084 + Mod.numberOfPatterns * ROWS * Mod.numberOfChannels * 4 - 1;

  for(i = 0; i < SAMPLES; i++) {

    if(Mod.samples[i].length) {
      Mixer.sampleBegin[i] = fileOffset;
      Mixer.sampleEnd[i] = fileOffset + Mod.samples[i].length;
      if(Mod.samples[i].loopLength > 2) {
        Mixer.sampleloopBegin[i] = fileOffset + Mod.samples[i].loopBegin;
        Mixer.sampleLoopLength[i] = Mod.samples[i].loopLength;
        Mixer.sampleLoopEnd[i] = Mixer.sampleloopBegin[i] + Mixer.sampleLoopLength[i];
      } 
      else {
        Mixer.sampleloopBegin[i] = 0;
        Mixer.sampleLoopLength[i] = 0;
        Mixer.sampleLoopEnd[i] = 0;
      }
      fileOffset += Mod.samples[i].length;
    }

  }
}

void loadPattern(uint8_t pattern) {
  static uint8_t row;
  static uint8_t channel;
  static UINT count;
  static uint8_t i;
  static uint8_t temp[4];
  static uint16_t amigaPeriod;

  f_lseek(&file, 1084 + pattern * ROWS * Mod.numberOfChannels * 4);

  for(row = 0; row < ROWS; row++) {
    for(channel = 0; channel < Mod.numberOfChannels; channel++) {

      f_read(&file, temp, 4, &count);

      Player.currentPattern.sampleNumber[row][channel] = (temp[0] & 0xF0) + (temp[2] >> 4);

      amigaPeriod = ((temp[0] & 0xF) << 8) + temp[1];
      Player.currentPattern.note[row][channel] = NONOTE;
      for(i = 1; i < 37; i++)
        if(amigaPeriod > amigaPeriods[i * 8] - 3 &&
          amigaPeriod < amigaPeriods[i * 8] + 3)
          Player.currentPattern.note[row][channel] = i * 8;

      Player.currentPattern.effectNumber[row][channel] = temp[2] & 0xF;
      Player.currentPattern.effectParameter[row][channel] = temp[3];

    }
  }
}

void portamento(uint8_t channel) {
  if(Player.lastAmigaPeriod[channel] < Player.portamentoNote[channel]) {
    Player.lastAmigaPeriod[channel] += Player.portamentoSpeed[channel];
    if(Player.lastAmigaPeriod[channel] > Player.portamentoNote[channel])
      Player.lastAmigaPeriod[channel] = Player.portamentoNote[channel];
  }
  if(Player.lastAmigaPeriod[channel] > Player.portamentoNote[channel]) {
    Player.lastAmigaPeriod[channel] -= Player.portamentoSpeed[channel];
    if(Player.lastAmigaPeriod[channel] < Player.portamentoNote[channel])
      Player.lastAmigaPeriod[channel] = Player.portamentoNote[channel];
  }
  Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];
}

void vibrato(uint8_t channel) {
  uint16_t delta;
  uint16_t temp;

  temp = Player.vibratoPos[channel] & 31;

  switch(Player.waveControl[channel] & 3) {
  case 0:
    delta = sine[temp];
    break;
  case 1:
    temp <<= 3;
    if(Player.vibratoPos[channel] < 0)
      temp = 255 - temp;
    delta = temp;
    break;
  case 2:
    delta = 255;
    break;
  case 3:
    delta = rand() & 255;
    break;
  }

  delta *= Player.vibratoDepth[channel];
  delta >>= 7;

  if(Player.vibratoPos[channel] >= 0)
    Mixer.channelFrequency[channel] = Player.amiga / (Player.lastAmigaPeriod[channel] + delta);
  else
    Mixer.channelFrequency[channel] = Player.amiga / (Player.lastAmigaPeriod[channel] - delta);

  Player.vibratoPos[channel] += Player.vibratoSpeed[channel];
  if(Player.vibratoPos[channel] > 31) Player.vibratoPos[channel] -= 64;
}

void tremolo(uint8_t channel) {
  uint16_t delta;
  uint16_t temp;

  temp = Player.tremoloPos[channel] & 31;

  switch(Player.waveControl[channel] & 3) {
  case 0:
    delta = sine[temp];
    break;
  case 1:
    temp <<= 3;
    if(Player.tremoloPos[channel] < 0)
      temp = 255 - temp;
    delta = temp;
    break;
  case 2:
    delta = 255;
    break;
  case 3:
    delta = rand() & 255;
    break;
  }

  delta *= Player.tremoloDepth[channel];
  delta >>= 6;

  if(Player.tremoloPos[channel] >= 0) {
    if(Player.volume[channel] + delta > 64) delta = 64 - Player.volume[channel];
    Mixer.channelVolume[channel] = Player.volume[channel] + delta;
  } 
  else {
    if(Player.volume[channel] - delta < 0) delta = Player.volume[channel];
    Mixer.channelVolume[channel] = Player.volume[channel] - delta;
  }

  Player.tremoloPos[channel] += Player.tremoloSpeed[channel];
  if(Player.tremoloPos[channel] > 31) Player.tremoloPos[channel] -= 64;
}

void processRow() {
  static bool jumpFlag;
  static bool breakFlag;
  static uint8_t channel;
  static uint8_t sampleNumber;
  static uint16_t note;
  static uint8_t effectNumber;
  static uint8_t effectParameter;
  static uint8_t effectParameterX;
  static uint8_t effectParameterY;
  static uint16_t sampleOffset;

  Player.lastRow = Player.row++;
  jumpFlag = false;
  breakFlag = false;
  for(channel = 0; channel < Mod.numberOfChannels; channel++) {

    sampleNumber = Player.currentPattern.sampleNumber[Player.lastRow][channel];
    note = Player.currentPattern.note[Player.lastRow][channel];
    effectNumber = Player.currentPattern.effectNumber[Player.lastRow][channel];
    effectParameter = Player.currentPattern.effectParameter[Player.lastRow][channel];
    effectParameterX = effectParameter >> 4;
    effectParameterY = effectParameter & 0xF;
    sampleOffset = 0;

    if(sampleNumber) {
      Player.lastSampleNumber[channel] = sampleNumber - 1;
      if(!(effectParameter == 0xE && effectParameterX == NOTEDELAY))
        Player.volume[channel] = Mod.samples[Player.lastSampleNumber[channel]].volume;
    }

    if(note != NONOTE) {
      Player.lastNote[channel] = note;
      Player.amigaPeriod[channel] = amigaPeriods[note + Mod.samples[Player.lastSampleNumber[channel]].fineTune];

      if(effectNumber != TONEPORTAMENTO && effectNumber != PORTAMENTOVOLUMESLIDE)
        Player.lastAmigaPeriod[channel] = Player.amigaPeriod[channel];

      if(!(Player.waveControl[channel] & 0x80)) Player.vibratoPos[channel] = 0;
      if(!(Player.waveControl[channel] & 0x08)) Player.tremoloPos[channel] = 0;
    }

    switch(effectNumber) {
    case TONEPORTAMENTO:
      if(effectParameter) Player.portamentoSpeed[channel] = effectParameter;
      Player.portamentoNote[channel] = Player.amigaPeriod[channel];
      note = NONOTE;
      break;

    case VIBRATO:
      if(effectParameterX) Player.vibratoSpeed[channel] = effectParameterX;
      if(effectParameterY) Player.vibratoDepth[channel] = effectParameterY;
      break;

    case PORTAMENTOVOLUMESLIDE:
      Player.portamentoNote[channel] = Player.amigaPeriod[channel];
      note = NONOTE;
      break;

    case TREMOLO:
      if(effectParameterX) Player.tremoloSpeed[channel] = effectParameterX;
      if(effectParameterY) Player.tremoloDepth[channel] = effectParameterY;
      break;

    case SETCHANNELPANNING:
      Mixer.channelPanning[channel] = effectParameter >> 1;
      break;

    case SETSAMPLEOFFSET:
      sampleOffset = effectParameter << 8;
      if(sampleOffset > Mod.samples[Player.lastSampleNumber[channel]].length)
        sampleOffset = Mod.samples[Player.lastSampleNumber[channel]].length;
      break;

    case JUMPTOORDER:
      Player.orderIndex = effectParameter;
      if(Player.orderIndex >= Mod.songLength)
        Player.orderIndex = 0;
      Player.row = 0;
      jumpFlag = true;
      break;

    case SETVOLUME:
      if(effectParameter > 64) Player.volume[channel] = 64;
      else Player.volume[channel] = effectParameter;
      break;

    case BREAKPATTERNTOROW:
      Player.row = effectParameterX * 10 + effectParameterY;
      if(Player.row >= ROWS)
        Player.row = 0;
      if(!jumpFlag && !breakFlag) {
        Player.orderIndex++;
        if(Player.orderIndex >= Mod.songLength)
          Player.orderIndex = 0;
      }
      breakFlag = true;
      break;

    case 0xE:
      switch(effectParameterX) {
      case FINEPORTAMENTOUP:
        Player.lastAmigaPeriod[channel] -= effectParameterY;
        break;

      case FINEPORTAMENTODOWN:
        Player.lastAmigaPeriod[channel] += effectParameterY;
        break;

      case SETVIBRATOWAVEFORM:
        Player.waveControl[channel] &= 0xF0;
        Player.waveControl[channel] |= effectParameterY;
        break;

      case SETFINETUNE:
        Mod.samples[Player.lastSampleNumber[channel]].fineTune = effectParameterY;
        if(Mod.samples[Player.lastSampleNumber[channel]].fineTune > 7)
          Mod.samples[Player.lastSampleNumber[channel]].fineTune -= 16;
        break;

      case PATTERNLOOP:
        if(effectParameterY) {
          if(Player.patternLoopCount[channel])
            Player.patternLoopCount[channel]--;
          else
            Player.patternLoopCount[channel] = effectParameterY;
          if(Player.patternLoopCount[channel])
            Player.row = Player.patternLoopRow[channel] - 1;
        } 
        else
          Player.patternLoopRow[channel] = Player.row;
        break;

      case SETTREMOLOWAVEFORM:
        Player.waveControl[channel] &= 0xF;
        Player.waveControl[channel] |= effectParameterY << 4;
        break;

      case FINEVOLUMESLIDEUP:
        Player.volume[channel] += effectParameterY;
        if(Player.volume[channel] > 64) Player.volume[channel] = 64;
        break;

      case FINEVOLUMESLIDEDOWN:
        Player.volume[channel] -= effectParameterY;
        if(Player.volume[channel] < 0) Player.volume[channel] = 0;
        break;

      case NOTECUT:
        note = NONOTE;
        break;

      case PATTERNDELAY:
        Player.patternDelay = effectParameterY;
        break;

      case INVERTLOOP:

        break;
      }
      break;

    case SETSPEED:
      if(effectParameter < 0x20)
        Player.speed = effectParameter;
      else
        Player.samplesPerTick = SAMPLERATE / (2 * effectParameter / 5);
      break;
    }

    if(note != NONOTE || Player.lastAmigaPeriod[channel] &&
      effectNumber != VIBRATO && effectNumber != VIBRATOVOLUMESLIDE &&
      !(effectNumber == 0xE && effectParameterX == NOTEDELAY))
      Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];

    if(note != NONOTE)
      Mixer.channelSampleOffset[channel] = sampleOffset << DIVIDER;

    if(sampleNumber)
      Mixer.channelSampleNumber[channel] = Player.lastSampleNumber[channel];

    if(effectNumber != TREMOLO)
      Mixer.channelVolume[channel] = Player.volume[channel];

  }
}

void processTick() {
  static uint8_t channel;
  static uint8_t sampleNumber;
  static uint16_t note;
  static uint8_t effectNumber;
  static uint8_t effectParameter;
  static uint8_t effectParameterX;
  static uint8_t effectParameterY;
  static uint16_t tempNote;

  for(channel = 0; channel < Mod.numberOfChannels; channel++) {

    if(Player.lastAmigaPeriod[channel]) {

      sampleNumber = Player.currentPattern.sampleNumber[Player.lastRow][channel];
      note = Player.currentPattern.note[Player.lastRow][channel];
      effectNumber = Player.currentPattern.effectNumber[Player.lastRow][channel];
      effectParameter = Player.currentPattern.effectParameter[Player.lastRow][channel];
      effectParameterX = effectParameter >> 4;
      effectParameterY = effectParameter & 0xF;

      switch(effectNumber) {
      case ARPEGGIO:
        if(effectParameter)
          switch(Player.tick % 3) {
          case 0:
            Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];
            break;
          case 1:
            tempNote = Player.lastNote[channel] + effectParameterX * 8 + Mod.samples[Player.lastSampleNumber[channel]].fineTune;
            if(tempNote < 296) Mixer.channelFrequency[channel] = Player.amiga / amigaPeriods[tempNote];
            break;
          case 2:
            tempNote = Player.lastNote[channel] + effectParameterY * 8 + Mod.samples[Player.lastSampleNumber[channel]].fineTune;
            if(tempNote < 296) Mixer.channelFrequency[channel] = Player.amiga / amigaPeriods[tempNote];
            break;
          }
        break;

      case PORTAMENTOUP:
        Player.lastAmigaPeriod[channel] -= effectParameter;
        if(Player.lastAmigaPeriod[channel] < 113) Player.lastAmigaPeriod[channel] = 113;
        Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];
        break;

      case PORTAMENTODOWN:
        Player.lastAmigaPeriod[channel] += effectParameter;
        if(Player.lastAmigaPeriod[channel] > 856) Player.lastAmigaPeriod[channel] = 856;
        Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];
        break;

      case TONEPORTAMENTO:
        portamento(channel);
        break;

      case VIBRATO:
        vibrato(channel);
        break;

      case PORTAMENTOVOLUMESLIDE:
        portamento(channel);
        Player.volume[channel] += effectParameterX - effectParameterY;
        if(Player.volume[channel] < 0) Player.volume[channel] = 0;
        else if(Player.volume[channel] > 64) Player.volume[channel] = 64;
        Mixer.channelVolume[channel] = Player.volume[channel];
        break;

      case VIBRATOVOLUMESLIDE:
        vibrato(channel);
        Player.volume[channel] += effectParameterX - effectParameterY;
        if(Player.volume[channel] < 0) Player.volume[channel] = 0;
        else if(Player.volume[channel] > 64) Player.volume[channel] = 64;
        Mixer.channelVolume[channel] = Player.volume[channel];
        break;

      case TREMOLO:
        tremolo(channel);
        break;

      case VOLUMESLIDE:
        Player.volume[channel] += effectParameterX - effectParameterY;
        if(Player.volume[channel] < 0) Player.volume[channel] = 0;
        else if(Player.volume[channel] > 64) Player.volume[channel] = 64;
        Mixer.channelVolume[channel] = Player.volume[channel];
        break;

      case 0xE:
        switch(effectParameterX) {
        case RETRIGGERNOTE:
          if(!effectParameterY) break;
          if(!(Player.tick % effectParameterY)) {
            Mixer.channelSampleOffset[channel] = 0;
          }
          break;

        case NOTECUT:
          if(Player.tick == effectParameterY)
            Mixer.channelVolume[channel] = Player.volume[channel] = 0;
          break;

        case NOTEDELAY:
          if(Player.tick == effectParameterY) {
            if(sampleNumber) Player.volume[channel] = Mod.samples[Player.lastSampleNumber[channel]].volume;
            if(note != NONOTE) Mixer.channelSampleOffset[channel] = 0;
            Mixer.channelFrequency[channel] = Player.amiga / Player.lastAmigaPeriod[channel];
            Mixer.channelVolume[channel] = Player.volume[channel];
          }
          break;
        }
        break;
      }

    }

  }
}

void player() {
  if(Player.tick == Player.speed) {
    Player.tick = 0;

    if(Player.row == ROWS) {
      Player.orderIndex++;
      if(Player.orderIndex == Mod.songLength)
        Player.orderIndex = 0;
      Player.row = 0;
    }

    if(Player.patternDelay) {
      Player.patternDelay--;
    } 
    else {
      if(Player.orderIndex != Player.oldOrderIndex)
        loadPattern(Mod.order[Player.orderIndex]);
      Player.oldOrderIndex = Player.orderIndex;
      processRow();
    }

  } 
  else {
    processTick();
  }
  Player.tick++;
}

void mixer() {
  static int32_t sumL;
  static int32_t sumR; //Overflowed when it was 16bit
  static uint8_t channel;
  static UINT samplePointer;
  static uint16_t fatBufferSize;
  static UINT count;
  static int8_t current;
  static int8_t next;
  static int32_t out;

  sumL = 0;
  sumR = 0;
  for(channel = 0; channel < Mod.numberOfChannels; channel++) {

    if(!Mixer.channelFrequency[channel] ||
      !Mod.samples[Mixer.channelSampleNumber[channel]].length) continue;

    Mixer.channelSampleOffset[channel] += Mixer.channelFrequency[channel];

    if(!Mixer.channelVolume[channel]) continue;

    samplePointer = Mixer.sampleBegin[Mixer.channelSampleNumber[channel]] +
      (Mixer.channelSampleOffset[channel] >> DIVIDER);

    if(Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]]) {

      if(samplePointer >= Mixer.sampleLoopEnd[Mixer.channelSampleNumber[channel]]) {
        Mixer.channelSampleOffset[channel] -= Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]] << DIVIDER;
        samplePointer -= Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]];
      }

    } 
    else {

      if(samplePointer >= Mixer.sampleEnd[Mixer.channelSampleNumber[channel]]) {
        Mixer.channelFrequency[channel] = 0;
        samplePointer = Mixer.sampleEnd[Mixer.channelSampleNumber[channel]];
      }

    }

    if(samplePointer < FatBuffer.samplePointer[channel] ||
      samplePointer >= FatBuffer.samplePointer[channel] + FATBUFFERSIZE - 1 ||
      Mixer.channelSampleNumber[channel] != FatBuffer.channelSampleNumber[channel]) {

      fatBufferSize = Mixer.sampleEnd[Mixer.channelSampleNumber[channel]] - samplePointer + 1;
      if(fatBufferSize > FATBUFFERSIZE) fatBufferSize = FATBUFFERSIZE;

      //   LEDLAT = ~(1 << (channel & 0x3));
      f_lseek(&file, samplePointer);
      f_read(&file, FatBuffer.channels[channel], fatBufferSize, &count);
      //   LEDLAT = 0xF;

      FatBuffer.samplePointer[channel] = samplePointer;
      FatBuffer.channelSampleNumber[channel] = Mixer.channelSampleNumber[channel];
    }

    current = FatBuffer.channels[channel][(samplePointer - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1];
    next = FatBuffer.channels[channel][(samplePointer + 1 - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1];

    out = current;

    // Integer linear interpolation
    out += (next - current) * (Mixer.channelSampleOffset[channel] & (1 << DIVIDER) - 1) >> DIVIDER;

    // Upscale to BITDEPTH
    out <<= BITDEPTH - 8;

    // Channel volume
    out = out * Mixer.channelVolume[channel] >> 6;

    // Channel panning
    sumL += out * min(128 - Mixer.channelPanning[channel], 64) >> 6;
    sumR += out * min(Mixer.channelPanning[channel], 64) >> 6;
  }

  // Downscale to BITDEPTH
  sumL /= Mod.numberOfChannels;
  sumR /= Mod.numberOfChannels;

  // Fill the sound buffer with unsigned values
  SoundBuffer.left[SoundBuffer.writePos] = sumL + (1 << BITDEPTH - 1);
  SoundBuffer.right[SoundBuffer.writePos] = sumR + (1 << BITDEPTH - 1);
  SoundBuffer.writePos++;
  SoundBuffer.writePos &= SOUNDBUFFERSIZE - 1;
}

void loadMod() {
  uint8_t channel;

  loadHeader();
  loadSamples();

  Player.amiga = AMIGA;
  Player.samplesPerTick = SAMPLERATE / (2 * 125 / 5); // Hz = 2 * BPM / 5
  Player.speed = 6;
  Player.tick = Player.speed;
  Player.row = 0;

  Player.orderIndex = 0;
  Player.oldOrderIndex = 0xFF;
  Player.patternDelay = 0;

  for(channel = 0; channel < Mod.numberOfChannels; channel++) {
    Player.patternLoopCount[channel] = 0;
    Player.patternLoopRow[channel] = 0;

    Player.lastAmigaPeriod[channel] = 0;

    Player.waveControl[channel] = 0;

    Player.vibratoSpeed[channel] = 0;
    Player.vibratoDepth[channel] = 0;
    Player.vibratoPos[channel] = 0;

    Player.tremoloSpeed[channel] = 0;
    Player.tremoloDepth[channel] = 0;
    Player.tremoloPos[channel] = 0;

    FatBuffer.samplePointer[channel] = 0;
    FatBuffer.channelSampleNumber[channel] = 0xFF;

    Mixer.channelSampleOffset[channel] = 0;
    Mixer.channelFrequency[channel] = 0;
    Mixer.channelVolume[channel] = 0;
    switch(channel % 4) {
    case 0:
    case 3:
      Mixer.channelPanning[channel] = STEREOSEPARATION;
      break;
    default:
      Mixer.channelPanning[channel] = 128 - STEREOSEPARATION;
    }
  }

  SoundBuffer.writePos = 0;
  SoundBuffer.readPos = 0;
}

void appTest2()
{
  vsWriteReg(SCI_VOL,0x4040);
  VS_DCS_LOW;
  unsigned char temp,aa;
  fileDialog(file,dir);
  loadMod();
  consoleReset();
  consolePuts(Mod.name);
  consolePuts("\n--------\n");
  for(aa=0;aa<SAMPLES;aa++)consolePuts(Mod.samples[aa].name),consolePutc('\n');
  for(aa=0;aa<44;aa++)
  {
    while(!vsDreq);
    vsWrite(riffHeader[aa],temp);
  }
  for(;;)
  {
    if(!returnButtonState) break;
    while((SoundBuffer.writePos + 1 & SOUNDBUFFERSIZE - 1) != SoundBuffer.readPos)
    {
      if(!i)
      {
        player();
        i = Player.samplesPerTick;
      }
      mixer();
      i--;
      if(vsDreq&&SoundBuffer.writePos!=SoundBuffer.readPos)
      {
        vsWrite(SoundBuffer.left[SoundBuffer.readPos]&0xFF,temp);
        vsWrite((SoundBuffer.left[SoundBuffer.readPos]>>8)^0x80,temp);
        vsWrite(SoundBuffer.right[SoundBuffer.readPos]&0xFF,temp);
        vsWrite((SoundBuffer.right[SoundBuffer.readPos]>>8)^0x80,temp);
        SoundBuffer.readPos++;
        SoundBuffer.readPos &= SOUNDBUFFERSIZE-1;
      }
    }
    if(vsDreq&&SoundBuffer.writePos!=SoundBuffer.readPos)
    {
      vsWrite(SoundBuffer.left[SoundBuffer.readPos]&0xFF,temp);
      vsWrite((SoundBuffer.left[SoundBuffer.readPos]>>8)^0x80,temp);
      vsWrite(SoundBuffer.right[SoundBuffer.readPos]&0xFF,temp);
      vsWrite((SoundBuffer.right[SoundBuffer.readPos]>>8)^0x80,temp);
      SoundBuffer.readPos++;
      SoundBuffer.readPos &= SOUNDBUFFERSIZE-1;
    }
  }
  VS_DCS_HIGH;
  vsEndPlaying();
  LCD_ResetWindow();
}
