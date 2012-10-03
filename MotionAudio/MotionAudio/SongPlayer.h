//
//  SongPlayer.h
//  FilterMotion
//
//  Created by Brian Salinas on 9/23/12.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

//#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface SongPlayer : NSObject

@property (readonly, nonatomic)Float64 hardwareSampleRate;
@property (readwrite, nonatomic)AudioUnitParameterValue volume;
@property (readwrite, nonatomic)AudioUnitParameterValue pan;
@property (readwrite, nonatomic)AudioUnitParameterValue speed;


- (id)initWithSampleRate:(Float64)sampleRate;

@end
