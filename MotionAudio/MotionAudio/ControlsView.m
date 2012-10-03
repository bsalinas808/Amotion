//
//  ControlsView.m
//  FilterMotion
//
//  Created by Brian Salinas on 9/23/12.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#import "ControlsView.h"

@implementation ControlsView
{
    UIFont *font_;
}

- (void)setVolume:(UISlider *)sender
{
    [self.delegate controls:self volume:sender.value];
}

- (void)setSpeed:(UISlider *)spee:(UISlider *)sender
{
    [self.delegate controls:self speed:sender.value];
}

#define SLIDER_HEIGHT 50.0

- (void)buildVolumeSlider
{
    CGRect rect = CGRectMake(50.0, 44.0, 84.0, 28.0);
    UILabel *label = [[UILabel alloc] initWithFrame:rect];
    [label setText:@"Volume"];
    [label setFont:font_];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextColor:[UIColor whiteColor]];
    [self addSubview:label];
    
    _volume = [[UISlider alloc] initWithFrame:CGRectMake(40.0, 54.0, 260.0, SLIDER_HEIGHT)];
    _volume.minimumValue = 0.0;
    _volume.maximumValue = 1.0;
    _volume.enabled = NO;
    _volume.tag = 1;
    [_volume addTarget:self action:@selector(setVolume:) forControlEvents:UIControlEventValueChanged];
    _volume.value = 0.0;
    [self addSubview:_volume];
}

- (void)buildSpeedSlider
{
    self.defaultSpeed = 1.0;
    
    CGRect sliderFrame = CGRectMake(40.0, 104.0, 260.0, SLIDER_HEIGHT);
    CGRect labelFrame = sliderFrame;
    labelFrame.origin.x += 10.0;
    labelFrame.origin.y -= 22.0;
    
    UILabel *label = [[UILabel alloc] initWithFrame:labelFrame];
    label.backgroundColor = [UIColor clearColor];
    label.text = @"Speed";
    label.textAlignment = NSTextAlignmentLeft;
    label.textColor = [UIColor whiteColor];
    label.font = font_;
    [self addSubview:label];
    
    _speed = [UISlider new];
    _speed.frame = sliderFrame;
    _speed.enabled = NO;
    _speed.continuous = YES;
    _speed.tag = kVarispeedParam_PlaybackRate;
    _speed.minimumValue = 0.0;
    _speed.maximumValue = 2.0;
    
    [_speed setMinimumTrackTintColor:[UIColor orangeColor]];
    [_speed setMaximumTrackTintColor:[UIColor orangeColor]];
    
    _speed.value = self.defaultSpeed;
    
    [_speed addTarget:self action:@selector(setSpeed:) forControlEvents:UIControlEventValueChanged];
    [self addSubview:_speed];
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        font_  = [UIFont fontWithName:@"ArialRoundedMTBold" size:18.0];
        [self buildVolumeSlider];
        [self buildSpeedSlider];
    }
    return self;
}

@end



