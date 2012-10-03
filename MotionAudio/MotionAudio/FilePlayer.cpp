//
//  AudioPlayer.cpp
//  FilePlayerIO
//
//  Created by Brian Salinas on 11/2/11.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#include <iostream>
#include "FilePlayer.h"

//#define PRINT_ASBD

CFURLRef getSourceURL();

const AudioUnitElement kOutputBus = 0;
const AudioUnitElement kInputBus = 1;

#pragma mark - utility functions

static void CheckError(OSStatus error, const char *operation)
{
    if (error == noErr) return;
    
    char errorString[20] = {0};
    // See if it appears to be a 4-char-code
    *(UInt32 *)(errorString + 1) = CFSwapInt32HostToBig(error);
    if (isprint(errorString[1]) && isprint(errorString[2]) &&
        isprint(errorString[3]) && isprint(errorString[4])) {
        errorString[0] = errorString[5] = '\'';
        errorString[6] = '\0';
    } else
        // No, format it as an integer
        sprintf(errorString, "%d", (int)error);
    
    fprintf(stderr, "Error: %s (%s)\n", operation, errorString);
    exit(1);
}

void FilePlayer::printASBD(AudioStreamBasicDescription asbd)
{
    char formatIDString[5];
    UInt32 formatID = CFSwapInt32HostToBig (asbd.mFormatID);
    bcopy (&formatID, formatIDString, 4);
    formatIDString[4] = '\0';
    
    printf("  Sample Rate:         %10.0f\n",   asbd.mSampleRate);
    printf("  Format ID:           %10s\n",     formatIDString);
    printf("  Format Flags:        %10lX\n",    asbd.mFormatFlags);
    printf("  Bytes per Packet:    %10ld\n",    asbd.mBytesPerPacket);
    printf("  Frames per Packet:   %10ld\n",    asbd.mFramesPerPacket);
    printf("  Bytes per Frame:     %10ld\n",    asbd.mBytesPerFrame);
    printf("  Channels per Frame:  %10ld\n",    asbd.mChannelsPerFrame);
    printf("  Bits per Channel:    %10ld\n",    asbd.mBitsPerChannel);
}

CFURLRef createSourceURL()
{
    CFStringRef fileType = CFSTR("wav");
	CFStringRef fileName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     "looperman-cufool-im-alive",
                                                     kCFStringEncodingASCII);
	CFBundleRef mainBundle  = CFBundleGetMainBundle();
    assert(mainBundle != NULL);
    
	CFURLRef sourceURL = CFBundleCopyResourceURL(mainBundle, fileName, fileType, NULL);
    if (sourceURL == NULL) {
        printf("BRSampleLoader: Problem loading source url for: %s.\n", CFStringGetCStringPtr(fileName, kCFStringEncodingMacRoman));
    }
    
	CFRelease(fileName);
	return sourceURL;
}

void FilePlayer::createFilePlayer(FilePlayerStruct *player)
{
    CFURLRef inputFileURL = createSourceURL();
    
    CheckError(AudioFileOpenURL(inputFileURL, 
                                kAudioFileReadPermission, 
                                0, 
                                &player->inputFile), 
               "AudioFileOpenURL failed");
    
    if (inputFileURL) {
        CFRelease(inputFileURL);
    }
    
    
    // Get the audio data format from the file
    UInt32 propSize = sizeof(player->fileFormat);
    CheckError(AudioFileGetProperty(player->inputFile, 
                                    kAudioFilePropertyDataFormat, 
                                    &propSize, 
                                    &player->fileFormat), 
               "Couldn't get file's data format");
}

void FilePlayer::buildProcessingGraph(FilePlayerStruct *player)
{
    CheckError(NewAUGraph(&auGraph), "NewAUGraph failed");
    
 	AudioComponentDescription ioDesc = {0};
	ioDesc.componentType = kAudioUnitType_Output;
	ioDesc.componentSubType = kAudioUnitSubType_RemoteIO;
	ioDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    // get the mixer unit
	AudioComponentDescription mixerDesc = {0};
	mixerDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
	mixerDesc.componentType = kAudioUnitType_Mixer;
	mixerDesc.componentSubType = kAudioUnitSubType_MultiChannelMixer;
    
    // get the file player unit
	AudioComponentDescription fPlayerDesc = {0};
	fPlayerDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
	fPlayerDesc.componentType = kAudioUnitType_Generator;
	fPlayerDesc.componentSubType = kAudioUnitSubType_AudioFilePlayer;
    
    AudioComponentDescription varispeedDesc = {0};
	varispeedDesc.componentManufacturer = kAudioUnitManufacturer_Apple;

    varispeedDesc.componentType = kAudioUnitType_FormatConverter;
    varispeedDesc.componentSubType = kAudioUnitSubType_Varispeed;
    
    CheckError(AUGraphAddNode (auGraph, &ioDesc, &ioNode), "Couldn't add ioNode"); 
    CheckError(AUGraphAddNode (auGraph, &mixerDesc, &mixerNode), "Couldn't add mixerNode"); 
    CheckError(AUGraphAddNode (auGraph, &fPlayerDesc, &fileNode), "Couldn't add fileNode"); 
    CheckError(AUGraphAddNode (auGraph, &varispeedDesc, &speedNode), "Couldn't add filterNode");
    
    CheckError(AUGraphOpen (auGraph),
               "Couldn't open the graph");
    CheckError(AUGraphNodeInfo (auGraph, ioNode, NULL, &rioUnit),
               "Couldn't get node info for ioNode");
    CheckError(AUGraphNodeInfo (auGraph, mixerNode, NULL, &mixerUnit),
               "Couldn't get node info for mixerNode"); 
    CheckError(AUGraphNodeInfo (auGraph, fileNode, NULL, &fileUnit),
               "Couldn't get node info for fPlayerNode"); 
    CheckError(AUGraphNodeInfo (auGraph, speedNode, NULL, &filterUnit),
               "Couldn't get node info for filterNode"); 
}

void FilePlayer::connectNodes()
{
    CheckError(AUGraphConnectNodeInput(auGraph, 
                                       fileNode, 
                                       kOutputBus, 
                                       speedNode,
                                       0), 
               "AUGraphConnectNodeInput fileNode -> filterNode failed");

    CheckError(AUGraphConnectNodeInput(auGraph, 
                                       speedNode,
                                       kOutputBus, 
                                       mixerNode,
                                       playerBus), 
               "AUGraphConnectNodeInput filterNode -> mixerNode failed");

    
    CheckError(AUGraphConnectNodeInput(auGraph,
                                       mixerNode,         // source node
                                       kOutputBus,        // source node output bus number
                                       ioNode,            // destination node
                                       0),                // desintation node input bus number
               "AUGraphConnectNodeInput mixerNode -> ioNode failed"); 
}


void FilePlayer::configureStreamFormats(FilePlayerStruct *player)
{
    // Get the audio data format from the file
    UInt32 propSize = sizeof(player->fileFormat);
    CheckError(AudioFileGetProperty(player->inputFile, 
                                    kAudioFilePropertyDataFormat, 
                                    &propSize, 
                                    &player->fileFormat), 
               "Couldn't get file's data format"); 
    
#ifdef PRINT_ASBD
    printASBD(player->fileFormat);
#endif
    
    memset(&audioFormat, 0, sizeof(audioFormat)); 
	UInt32 asbdSize = sizeof(audioFormat); 
    
	CheckError(AudioUnitGetProperty(rioUnit,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Input,
                                    kInputScope,
                                    &audioFormat,
                                    &asbdSize), "inspected input ASBD");
#ifdef PRINT_ASBD
    printASBD(audioFormat);
#endif
    size_t bytesPerSample = sizeof (AudioUnitSampleType);
    audioFormat.mFormatID           = kAudioFormatLinearPCM;
    audioFormat.mBytesPerPacket     = bytesPerSample;
    audioFormat.mBytesPerFrame      = bytesPerSample;
    audioFormat.mFramesPerPacket    = 1;
    audioFormat.mChannelsPerFrame   = 2;           // 2 indicates stereo
    audioFormat.mSampleRate         = hardwareSampleRate;
#ifdef PRINT_ASBD    
    printASBD(audioFormat);
#endif
    
    CheckError(AudioUnitSetProperty(mixerUnit,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Input,
                                    playerBus,
                                    &audioFormat,
                                    sizeof (audioFormat)),
               "Couldn't set ASBD for mixer unit on playThruBus");	
        
    // Where changing the format throughout the app so set the rioUnit
    // output scope to the same audioFormat as everything else. 
    CheckError(AudioUnitSetProperty(rioUnit,
                                    kAudioUnitProperty_StreamFormat,
                                    kAudioUnitScope_Output,
                                    kInputBus,
                                    &audioFormat,
                                    sizeof (audioFormat)),
               "Couldn't set ASBD for rioUnit on output scope ");	
}

void FilePlayer::initGraph()
{
    // Increase the maximum frames per slice allows the mixer unit to accommodate the
    // larger slice size used when the screen is locked.
    // This property crashes is set after AUGraphInitialize called
    UInt32 maximumFramesPerSlice = 4096;
    
    CheckError(AudioUnitSetProperty(mixerUnit,
                                    kAudioUnitProperty_MaximumFramesPerSlice,
                                    kAudioUnitScope_Global,
                                    0,
                                    &maximumFramesPerSlice,
                                    sizeof (maximumFramesPerSlice)),
               "Failed setting kAudioUnitProperty_MaximumFramesPerSlice");
    
    // Initialize the graph (causes resources to be allocated)
    CheckError(AUGraphInitialize(auGraph), "AUGraphInitialize failed");
}

void FilePlayer::setProperties(FilePlayerStruct *player)
{
    // Tell the file player unit to load the file we want to play
    CheckError(AudioUnitSetProperty(fileUnit, 
                                    kAudioUnitProperty_ScheduledFileIDs, 
                                    kAudioUnitScope_Global, 
                                    0, 
                                    &player->inputFile, 
                                    sizeof(player->inputFile)), 
               "AudioUnitSetProperty[kAudioUnitProperty_ScheduledFileIDs] failed");
    
    UInt64 nPackets;
    UInt32 propsize = sizeof(nPackets);
    CheckError(AudioFileGetProperty(player->inputFile, 
                                    kAudioFilePropertyAudioDataPacketCount, 
                                    &propsize, 
                                    &nPackets), 
               "AudioFileGetProperty[kAudioFilePropertyAudioDataPacketCount] failed");
    
    // Tell the file player AU to play the entire file
    ScheduledAudioFileRegion region;
    memset(&region.mTimeStamp, 0, sizeof(region.mTimeStamp));
    region.mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
    region.mTimeStamp.mSampleTime = 0;
    region.mCompletionProc = NULL;
    region.mCompletionProcUserData = NULL;
    region.mAudioFile = player->inputFile;
    region.mLoopCount = -1;
    region.mStartFrame = 0;
    region.mFramesToPlay = nPackets * player->fileFormat.mFramesPerPacket;
    
    CheckError(AudioUnitSetProperty(fileUnit, 
                                    kAudioUnitProperty_ScheduledFileRegion, 
                                    kAudioUnitScope_Global, 
                                    0, 
                                    &region, 
                                    sizeof(region)), 
               "AudioUnitSetProperty[kAudioUnitProperty_ScheduledFileRegion] failed");
    
    // Tell the file player AU when to start playing (-1 sample time means render cycle)
    AudioTimeStamp startTime;
    memset(&startTime, 0, sizeof(startTime));
    startTime.mFlags = kAudioTimeStampSampleTimeValid;
    startTime.mSampleTime  = -1;
    CheckError(AudioUnitSetProperty(fileUnit, 
                                    kAudioUnitProperty_ScheduleStartTimeStamp, 
                                    kAudioUnitScope_Global, 
                                    0, 
                                    &startTime, 
                                    sizeof(startTime)), 
               "AudioUnitSetProperty[kAudioUnitProperty_ScheduleStartTimeStamp]");
    
    CheckError(AudioUnitSetProperty(mixerUnit,
                                    kAudioUnitProperty_ElementCount,
                                    kAudioUnitScope_Input,
                                    0,
                                    &busCount,
                                    sizeof (busCount)),
               "AudioUnitSetProperty (set mixer unit bus count");
    
    CheckError(AudioUnitSetProperty(mixerUnit,
                                    kAudioUnitProperty_SampleRate,
                                    kAudioUnitScope_Output,
                                    0,
                                    &hardwareSampleRate,
                                    sizeof (hardwareSampleRate)),
               "Failure setting AudioUnitSetProperty kAudioUnitProperty_SampleRate"); 
}

void FilePlayer::setVolume(AudioUnitParameterValue value, AudioUnitElement channel)
{
    CheckError(AudioUnitSetParameter(mixerUnit,
                                     kMultiChannelMixerParam_Volume,
                                     kAudioUnitScope_Input,
                                     channel,           
                                     value,
                                     0),          // bufferedOffsetInFrames
               "Couldn't set mixer volume on first bus");
}

/*
 * Range is from -1 to 1, 0 is center and the default
 *
 * May cause a buzz in the next triggered sound if panned
 * hard left or right. 
 */
void FilePlayer::setPan(AudioUnitParameterValue position, AudioUnitElement channel)
{
	// May need to force the imput bus to the rane
	// -0.01 - 0.99
    CheckError(AudioUnitSetParameter(mixerUnit, 
                                     kMultiChannelMixerParam_Pan,
                                     kAudioUnitScope_Input, 
                                     channel, 
                                     position, 
                                     0),          // bufferedOffsetInFrames
               "Couldn't set mixer kMultiChannelMixerParam_Pan");
}

// Use 0 to disable, or mute an input. Use 1 to enable or unmute an input
void FilePlayer::setMute(bool onOrOff, AudioUnitElement channel)
{
    CheckError(AudioUnitSetParameter(mixerUnit, 
                                     kMultiChannelMixerParam_Enable, 
                                     kAudioUnitScope_Input, 
                                     channel, 
                                     onOrOff, 
                                     0),          // bufferedOffsetInFrames
               "Couldn't set mixer kMultiChannelMixerParam_Enable mute");	
}


void FilePlayer::setRate(AudioUnitParameterValue value, AudioUnitParameterID param)
{
    AudioUnitSetParameter(filterUnit,
						  param,
						  kAudioUnitScope_Global,
						  0, 
						  value, 
						  0);
}

#pragma mark Playback control

void FilePlayer::startGraph()
{
    CheckError(AUGraphStart(auGraph), "AUGraphStart failed");
}

void FilePlayer::stopGraph() {	
    CheckError(AUGraphStop(auGraph), "AUGraphStop failed");
}

FilePlayer::FilePlayer(Float64 sampleRate)
{
    hardwareSampleRate = sampleRate;
    FilePlayerStruct player = {0};
    
    createFilePlayer(&player);
    buildProcessingGraph(&player);
    connectNodes();
    configureStreamFormats(&player);
    
    initGraph();
    setProperties(&player);
    
    startGraph();
}

inline FilePlayer::FilePlayer()
{
    hardwareSampleRate = 44100;
}

FilePlayer::~FilePlayer()
{
    stopGraph();
    AudioOutputUnitStop(rioUnit);
    AudioUnitUninitialize(rioUnit);
    AUGraphUninitialize(auGraph);
    AUGraphClose(auGraph);
    AudioFileClose(fPlayer.inputFile);
}

