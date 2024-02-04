#include <AppKit/AppKit.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable float globalRenderingWidth = 1280;
global_variable float globalRenderingHeight = 720;
global_variable bool running = true;
global_variable uint8_t *buffer;

global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;
global_variable int pitch;

void macOS_refreshBuffer(NSWindow *window) {
  if (buffer) {
    free(buffer);
  }

  bitmapWidth = window.contentView.bounds.size.width;
  bitmapHeight = window.contentView.bounds.size.height;
  pitch = bitmapWidth * bytesPerPixel; // size of a row
  buffer = (uint8_t *)malloc(pitch * bitmapHeight);
}

void macOS_redrawBuffer(NSWindow *window) {
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
}

global_variable int xOffset = 0;
void renderWeirdGradiant() {
  uint8_t *row = buffer;

  for (int y = 0; y < bitmapHeight; y++) {
    uint8_t *pixelChannel = row;

    for (int x = 0; x < bitmapWidth; x++) {
      // red
      *pixelChannel = 0;
      ++pixelChannel;

      // green
      *pixelChannel = x + xOffset;
      ++pixelChannel;

      // blue
      *pixelChannel = y;
      ++pixelChannel;

      // alpha
      *pixelChannel = 255;
      ++pixelChannel;
    }

    row = pixelChannel;
  }
}

@interface HandmadeMainWindowDelegate : NSObject <NSWindowDelegate>
;
@end

@implementation HandmadeMainWindowDelegate

- (void)windowWillClose:(id)sender {
  running = false;
}

- (void)windowDidResize:(NSNotification *)notification {
  NSWindow *window = notification.object;
  macOS_refreshBuffer(window);
  renderWeirdGradiant(); // comment out for a beautiful effect when resizing
  macOS_redrawBuffer(window);
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

  macOS_refreshBuffer(window);

  while (running) {
    renderWeirdGradiant();
    macOS_redrawBuffer(window);
    xOffset += 1;

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
