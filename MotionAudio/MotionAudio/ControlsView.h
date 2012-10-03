//
//  ControlsView.h
//  FilterMotion
//
//  Created by Brian Salinas on 9/23/12.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

@class ControlsView;

@protocol ControlsViewDelegate <NSObject>
@required
- (void)controls:(ControlsView *)sender volume:(AudioUnitParameterValue)volume;
- (void)controls:(ControlsView *)sender speed:(AudioUnitParameterValue)value;
@end

@interface ControlsView : UIView
@property (nonatomic)Float32 defaultSpeed;
@property (readwrite, nonatomic, strong)UISlider *speed;
@property (readwrite, nonatomic, strong)UISlider *volume;
@property (nonatomic, weak) id <ControlsViewDelegate> delegate;
@end

