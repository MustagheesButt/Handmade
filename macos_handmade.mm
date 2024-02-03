#include <AppKit/AppKit.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable float globalRenderingWidth = 1280;
global_variable float globalRenderingHeight = 720;
global_variable bool running = true;
global_variable uint8_t *buffer;

@interface HandmadeMainWindowDelegate : NSObject <NSWindowDelegate>
;
@end

@implementation HandmadeMainWindowDelegate

- (void)windowWillClose:(id)sender {
  running = false;
}

@end

int main(int argc, const char *argv[]) {
  HandmadeMainWindowDelegate *mainWindowDelegate =
      [[HandmadeMainWindowDelegate alloc] init];

  NSRect screenRect = [[NSScreen mainScreen] frame];
  NSRect initialFrame =
      NSMakeRect((screenRect.size.width - globalRenderingWidth) * 0.5,
                 (screenRect.size.height - globalRenderingHeight) * 0.5,
                 globalRenderingWidth, globalRenderingHeight);
  NSWindow *window = [[NSWindow alloc]
      initWithContentRect:initialFrame
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                          NSWindowStyleMaskMiniaturizable |
                          NSWindowStyleMaskResizable
                  backing:NSBackingStoreBuffered
                    defer:NO];

  [window setBackgroundColor:NSColor.blackColor];
  [window setTitle:@"Handmade Hero"];
  [window makeKeyAndOrderFront:nil];
  [window setDelegate:mainWindowDelegate];
  window.contentView.wantsLayer = YES;

  int bitmapWidth = window.contentView.bounds.size.width;
  int bitmapHeight = window.contentView.bounds.size.height;
  int bytesPerPixel = 4;
  int pitch = bitmapWidth * bytesPerPixel; // size of a row

  buffer = (uint8_t *)malloc(pitch * bitmapHeight);

  while (running) {
    @autoreleasepool {
      NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
          initWithBitmapDataPlanes:&buffer
                        pixelsWide:bitmapWidth
                        pixelsHigh:bitmapHeight
                     bitsPerSample:8
                   samplesPerPixel:bytesPerPixel
                          hasAlpha:YES
                          isPlanar:NO
                    colorSpaceName:NSDeviceRGBColorSpace
                       bytesPerRow:pitch
                      bitsPerPixel:bytesPerPixel * 8] autorelease];

      NSSize imageSize = NSMakeSize(bitmapWidth, bitmapHeight);
      NSImage *image = [[[NSImage alloc] initWithSize:imageSize] autorelease];
      [image addRepresentation:rep];
      window.contentView.layer.contents = image;
    }

    NSEvent *event;
    do {
      event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                 untilDate:nil
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES];
      switch ([event type]) {
      default:
        [NSApp sendEvent:event];
      }
    } while (event != nil);
  }
}
