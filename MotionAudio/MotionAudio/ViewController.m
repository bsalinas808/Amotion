//
//  ViewController.m
//  MotionAudio
//
//  Created by Brian Salinas on 9/24/12.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "ViewController.h"
#import "CoreMotion/CoreMotion.h"
#import "ControlsView.h"
#import "SongPlayer.h"

@interface ViewController()<ControlsViewDelegate>
@property (readwrite, nonatomic, strong)ControlsView *controls;
@property (readwrite, nonatomic, strong)CMMotionManager *motionManager;
@property (readwrite, nonatomic, strong)SongPlayer *sPlayer;
@end

@implementation ViewController

#pragma mark - Toolbar
#define TOOLBAR_HEIGHT 44.0

// this will not work right if loaded in viewDidLoad, use viewWillAppear
- (void)buildToolbar
{
    CGRect rect = self.view.bounds;
    rect.size = CGSizeMake(rect.size.width, TOOLBAR_HEIGHT);
    UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:rect];
    toolbar.barStyle = UIBarStyleBlackTranslucent;
    
    UILabel *titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(0.0,
                                                                    0.0,
                                                                    toolbar.bounds.size.width,
                                                                    TOOLBAR_HEIGHT)];
    titleLabel.backgroundColor = [UIColor clearColor];
    titleLabel.text = @"Motion Audio";
    titleLabel.textColor = [UIColor orangeColor];
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:24.0];
    
    UIBarButtonItem *titleItem = [[UIBarButtonItem alloc] initWithCustomView:titleLabel];
    
    UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                               target:nil
                                                                               action:nil];
    NSArray *items = @[flexSpace, titleItem, flexSpace];
    [toolbar setItems:items animated:NO];
    [self.view addSubview:toolbar];
}

#pragma mark - ControlsViewDelegate methods

- (void)controls:(ControlsView *)sender volume:(AudioUnitParameterValue)volume
{
    self.sPlayer.volume = volume;
}

- (void)controls:(ControlsView *)sender speed:(AudioUnitParameterValue)speed
{
    self.sPlayer.speed = speed;
}

#define BORDER_WIDTH 16.0
#define CORNER_RADIUS 8.0

- (void)buildControlsView
{
    CGRect rect = [[UIScreen mainScreen] bounds];
    rect.size.height = 180.0;
    rect.origin.y = TOOLBAR_HEIGHT ;
    
    self.controls = [[ControlsView alloc] initWithFrame:rect];
    self.controls.delegate = self;
    self.controls.backgroundColor = [[UIColor grayColor] colorWithAlphaComponent:0.5];
    self.controls.layer.cornerRadius = CORNER_RADIUS;
    [self.controls setBounds:CGRectMake(BORDER_WIDTH,
                                        BORDER_WIDTH,
                                        self.controls.bounds.size.width - BORDER_WIDTH,
                                        self.controls.bounds.size.height - BORDER_WIDTH)];
	self.controls.layer.edgeAntialiasingMask = YES;
    [self.view addSubview:self.controls];
}

- (void)buildDirectionsView
{
    CGRect rect = [[UIScreen mainScreen] bounds];
    rect.origin.y = self.controls.bounds.size.height + self.controls.bounds.origin.y + TOOLBAR_HEIGHT - 10.0;
    rect.size.height -= (rect.origin.y + 22.0);
    
    UIView *dirView = [[UIView alloc] initWithFrame:rect];
    
    
    dirView.backgroundColor = [[UIColor grayColor] colorWithAlphaComponent:0.5];
    
    
    dirView.layer.cornerRadius = CORNER_RADIUS;
    [dirView setBounds:CGRectMake(BORDER_WIDTH,
                                  BORDER_WIDTH,
                                  dirView.bounds.size.width - BORDER_WIDTH,
                                  dirView.bounds.size.height - BORDER_WIDTH)];
	dirView.layer.edgeAntialiasingMask = YES;
    

    CGFloat labelBorder = 50.0;
    rect.origin.x += labelBorder;
    rect.origin.y = 6.0;
    rect.size.width -= 2*labelBorder;
    UILabel *directions = [[UILabel alloc] initWithFrame:rect];
    directions.backgroundColor = [UIColor clearColor];
	directions.layer.edgeAntialiasingMask = YES;
    directions.numberOfLines = 9;
    directions.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:18.0];
    directions.textColor = [UIColor lightGrayColor];
    directions.textAlignment = NSTextAlignmentLeft;
    directions.text = @"Lay your iOS device flat on a table with the home button closest to you. Tilt your device back towards you as if letting off the gas pedal. Now, rotate your device to the right, back to the center, then to the left.";
    
    [dirView addSubview:directions];
    [self.view addSubview:dirView];
}

#pragma mark - Motion methods

- (void)buildMotionManager
{
    self.motionManager = [[CMMotionManager alloc] init];
    self.motionManager.deviceMotionUpdateInterval = 0.1;
}

- (void)startMotionManager
{
    float const kThreshold = 0.1;
    if (self.motionManager.isDeviceMotionAvailable){
        
        void (^volAndSpeed)(CMDeviceMotion*, NSError*);
        volAndSpeed = ^(CMDeviceMotion *deviceData, NSError *error){
            float accel = deviceData.gravity.x;
            if (fabs(deviceData.gravity.x)  < kThreshold) {
                self.sPlayer.speed = 1.0;
                [self.controls.speed setValue:1.0 animated:YES];
            } else if (deviceData.gravity.x > 0) {
                self.sPlayer.speed = accel*0.4 + 1.0;
                [self.controls.speed setValue:accel + 0.9 animated:YES];
            } else { // deviceData.gravity.x < 0
                float value = accel*.90 + .10;
                self.sPlayer.speed = value + 1.0;
                [self.controls.speed setValue:value + 0.90 animated:YES];
            }
            
            // Volume
            accel = -deviceData.gravity.z;
            if (deviceData.gravity.z <  0.0 && deviceData.gravity.y < -kThreshold) {
                float value = accel*.90 + .10;
                self.sPlayer.volume = accel;
                [self.controls.volume setValue:value animated:YES];
            }
        };
        
        NSOperationQueue *opQ_ = [NSOperationQueue currentQueue];
        [self.motionManager startDeviceMotionUpdatesToQueue:opQ_
                                                withHandler:volAndSpeed];
    }
}

- (void)stopMotionManager
{
    if (self.motionManager.isDeviceMotionActive)
        [self.motionManager stopDeviceMotionUpdates];
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _sPlayer = [SongPlayer new];
    [self buildControlsView];
    [self buildDirectionsView];
    [self buildMotionManager];
    [self startMotionManager];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [self buildToolbar];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
    
    [self stopMotionManager];
    _motionManager = nil;
    _controls = nil;
    _sPlayer = nil;
}

@end
