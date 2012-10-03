//
//  BRBackdrop.m
//  Drumtapz
//
//  Created by Brian Salinas on 1/30/11.
//  Copyright (c) 2012 Bit Rhythmic Inc. All rights reserved.
//

#import "Backdrop.h"

@implementation Backdrop{
    UIImage	*backgroundTileImage;
	CGRect	tileRect;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:CGRectMake(0.0, 0.0, frame.size.width, frame.size.height)];
    if (self) {
		NSString *imagePath = [[NSBundle mainBundle] pathForResource:@"BackdropTile" ofType:@"png"];
		backgroundTileImage = [[UIImage alloc] initWithContentsOfFile:imagePath];
#ifdef DEBUG
		NSAssert1(backgroundTileImage != nil, @"%@.initWithFrame: BackdropTile did not load", [self class]);
#endif
		tileRect.size = backgroundTileImage.size;
	}
    return self;
}

- (void)drawRect:(CGRect)rect{
	CGContextRef context = UIGraphicsGetCurrentContext();
	
    // Draw the image in the upper left corner (0,0) with size 64x64
    CGContextDrawImage(context, tileRect, backgroundTileImage.CGImage);
    
    // Tile the same image across the bottom of the view
    // CGContextDrawTiledImage() will fill the entire clipping area with the image, so to avoid
    // filling the entire view, we'll clip the view to the rect below. This rect extends
    // past the region of the view, but since the view's rectangle has already been applied as a clip
	// to our drawing area, it will be intersected with this rect to form the final clipping area
	CGContextClipToRect(context, CGRectMake(0.0, 0.0, rect.size.width, rect.size.height));
	
    tileRect.origin = CGPointMake(tileRect.size.width, tileRect.size.height);
    CGContextDrawTiledImage(context, tileRect, backgroundTileImage.CGImage);
}

@end
