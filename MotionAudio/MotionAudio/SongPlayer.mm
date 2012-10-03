//
//  SongPlayer.m
//  FilterMotion
//
//  Created by Brian Salinas on 9/23/12.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#import "SongPlayer.h"
#import <AVFoundation/AVFoundation.h>
#import "FilePlayer.h"

@implementation SongPlayer
{
    FilePlayer *fPlayer_;    
}

- (void)configureAudioSession
{
    NSError *audioSessionError = nil;
    
    AVAudioSession *mySession = [AVAudioSession sharedInstance];
    [mySession setPreferredSampleRate:_hardwareSampleRate
                                error: &audioSessionError];
    [mySession setCategory: AVAudioSessionCategoryPlayback
                     error: &audioSessionError];
    [mySession setActive: YES
                   error: &audioSessionError];
    
    _hardwareSampleRate = [mySession sampleRate];
    
    /*
     * The default duration is about 23 ms at a 44.1 kHz sample rate, equivalent
     * to a slice size of 1,024 samples. If I/O latency is critical in your app,
     * you can request a smaller duration, down to about 0.005 ms (equivalent to
     * 256 samples).
     */
    Float64 ioBufferDuration = 0.005;
    [mySession setPreferredIOBufferDuration: ioBufferDuration
                                      error: &audioSessionError];
}

- (void)setVolume:(AudioUnitParameterValue)volume
{
    fPlayer_->setVolume(volume, playerBus);
}

- (void)setPan:(AudioUnitParameterValue)pan
{
    fPlayer_->setPan(pan, playerBus);
}

- (void)setSpeed:(AudioUnitParameterValue)speed
{
    fPlayer_->setRate(speed, playerBus);
}

- (id)initWithSampleRate:(Float64)sampleRate
{
    if (self = [super init]){
        _hardwareSampleRate = sampleRate;
        [self configureAudioSession];
        fPlayer_ = new FilePlayer(_hardwareSampleRate);
    }
    return self;
}

- (id)init
{
    if (self = [self initWithSampleRate:44100]){}
    return self;
}

- (void)dealloc
{
    delete fPlayer_;
}


@end
