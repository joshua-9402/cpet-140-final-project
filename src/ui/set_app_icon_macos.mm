/*
 * CpET 140 Final Project — macOS Icon Helper
 * StructuraCost - macOS application icon setter
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Sets the macOS application/dock icon at runtime using Cocoa APIs.
 * - Called from ui.cpp PostInit to replace default ImGui icon with app_icon.png.
 *
 * Boundaries
 * - macOS only. Compiled as Objective-C++ (.mm) on Apple platforms.
 *
 * Notes
 * - Uses NSApp setApplicationIconImage to update the dock icon.
 */

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>

extern "C" void setAppIcon(const char* path) {
    @autoreleasepool {
        if (!path) return;
        NSString* nsPath = [NSString stringWithUTF8String:path];
        if (!nsPath) return;
        NSImage* img = [[NSImage alloc] initWithContentsOfFile:nsPath];
        if (img) {
            // Set the application (dock) icon
            dispatch_async(dispatch_get_main_queue(), ^{
                [NSApp setApplicationIconImage:img];
            });
        }
    }
}
#endif

