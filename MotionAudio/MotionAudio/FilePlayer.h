//
//  AudioPlayer.h
//  FilePlayerIO
//
//  Created by B Slnas on 11/2/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>

#ifndef FilePlayerIO_AudioPlayer_h
#define FilePlayerIO_AudioPlayer_h
 
static const UInt32 playerBus       = 0;
static const UInt32 busCount        = 1;    // bus count for mixer unit input
static const UInt32 kInputScope     = 1; 
static const UInt32 kOutputScope    = 0;

typedef struct FilePlayerStruct{
    AudioStreamBasicDescription fileFormat;
    AudioFileID                 inputFile;
}FilePlayerStruct;

class FilePlayer {
public:
    FilePlayer(Float64 sampleRate);
    FilePlayer();
	~FilePlayer();
    
    void setVolume(AudioUnitParameterValue value, AudioUnitElement channel);
    void setPan(AudioUnitParameterValue position, AudioUnitElement channel);
    void setMute(bool onOrOff, AudioUnitElement channel);
    void setRate(AudioUnitParameterValue value, AudioUnitParameterID param);
    
private:
    AUNode  ioNode;
    AUNode  mixerNode;
    AUNode  fileNode;
    AUNode  speedNode;
    AUGraph                     auGraph;
    AudioUnit                   mixerUnit;
    AudioUnit                   rioUnit;
    AudioUnit                   fileUnit;
    AudioUnit                   filterUnit;
    AudioStreamBasicDescription audioFormat;
    Float64                     hardwareSampleRate;
    FilePlayerStruct            fPlayer;
    
    void configureStreamFormats(FilePlayerStruct *player);
    void setProperties(FilePlayerStruct *player);
    void connectNodes();
    
    void createFilePlayer(FilePlayerStruct *player);
    void scheduleFileRegion(FilePlayerStruct *player);
    void buildProcessingGraph(FilePlayerStruct *player);
    
    void initGraph();
    void startGraph();    
    void stopGraph();
    
    void setParamater(AudioUnitParameterValue value, AudioUnitParameterID param);
    void printASBD(AudioStreamBasicDescription asbd);
};


#endif
