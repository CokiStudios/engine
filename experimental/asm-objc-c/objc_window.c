#include <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void exp_asm_clear_rgba32(uint32_t* dst, size_t count, uint32_t px);
extern void exp_asm_fill_rect_rgba32(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t px);
extern uint64_t exp_asm_fnv1a64(const void* data, size_t len);

/* Pure C + Objective-C Runtime ABI (`objc_msgSend`) window launcher without C++, Python, or Swift */

#define MSG_ID ((id (*)(id, SEL))objc_msgSend)
#define MSG_ID_BOOL ((id (*)(id, SEL, BOOL))objc_msgSend)
#define MSG_VOID ((void (*)(id, SEL))objc_msgSend)
#define MSG_VOID_ID ((void (*)(id, SEL, id))objc_msgSend)
#define MSG_VOID_BOOL ((void (*)(id, SEL, BOOL))objc_msgSend)
#define MSG_VOID_INT ((void (*)(id, SEL, long))objc_msgSend)
#define MSG_ID_STR ((id (*)(id, SEL, const char*))objc_msgSend)
#define MSG_ID_RECT_STYLE ((id (*)(id, SEL, CGRect, unsigned long, unsigned long, BOOL))objc_msgSend)
#define MSG_ID_RECT ((id (*)(id, SEL, CGRect))objc_msgSend)
#define MSG_ID_RGBA ((id (*)(id, SEL, double, double, double, double))objc_msgSend)
#define MSG_VOID_CGCOLOR ((void (*)(id, SEL, CGColorRef))objc_msgSend)

typedef struct {
    char title[128];
    char card_title[128];
    char card_text[256];
    int width;
    int height;
    int fb_w;
    int fb_h;
    uint32_t* fb;
    int frame;
} ExpWindowCtx;

static ExpWindowCtx g_ctx;

static id nsstr(const char* s) {
    Class strCls = objc_getClass("NSString");
    return MSG_ID_STR((id)strCls, sel_registerName("stringWithUTF8String:"), s ? s : "");
}

static void render_asm_frame(ExpWindowCtx* ctx) {
    if (!ctx || !ctx->fb) return;
    exp_asm_clear_rgba32(ctx->fb, (size_t)ctx->fb_w * (size_t)ctx->fb_h, 0xFF0B101Bu);
    /* Grid */
    for (int gx = 0; gx < ctx->fb_w; gx += 40) {
        exp_asm_fill_rect_rgba32(ctx->fb, ctx->fb_w, ctx->fb_h, gx, 0, 1, ctx->fb_h, 0xFF172338u);
    }
    for (int gy = 0; gy < ctx->fb_h; gy += 40) {
        exp_asm_fill_rect_rgba32(ctx->fb, ctx->fb_w, ctx->fb_h, 0, gy, ctx->fb_w, 1, 0xFF172338u);
    }
    /* Animated entity blocks */
    int px = 40 + ((ctx->frame * 4) % (ctx->fb_w - 80));
    int py = ctx->fb_h / 2 - 22;
    exp_asm_fill_rect_rgba32(ctx->fb, ctx->fb_w, ctx->fb_h, px, py, 44, 44, 0xFF00F5D4u);
    exp_asm_fill_rect_rgba32(ctx->fb, ctx->fb_w, ctx->fb_h, 24, 24, 180, 62, 0xFF1E293Bu);
    exp_asm_fill_rect_rgba32(ctx->fb, ctx->fb_w, ctx->fb_h, 24, 24, 180, 4, 0xFF38BDF8u);
    ctx->frame++;
}

static void exp_view_draw_rect(id self, SEL _cmd, CGRect dirty) {
    (void)self;
    (void)_cmd;
    (void)dirty;
    render_asm_frame(&g_ctx);

    Class nsGcCls = objc_getClass("NSGraphicsContext");
    id curGc = MSG_ID((id)nsGcCls, sel_registerName("currentContext"));
    if (!curGc) return;
    CGContextRef cg = ((CGContextRef (*)(id, SEL))objc_msgSend)(curGc, sel_registerName("CGContext"));
    if (!cg) return;

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef prov = CGDataProviderCreateWithData(
        NULL, g_ctx.fb, (size_t)g_ctx.fb_w * (size_t)g_ctx.fb_h * 4u, NULL);
    CGImageRef img = CGImageCreate(
        (size_t)g_ctx.fb_w, (size_t)g_ctx.fb_h, 8, 32,
        (size_t)g_ctx.fb_w * 4u, cs,
        kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst,
        prov, NULL, false, kCGRenderingIntentDefault);
    if (img) {
        CGContextDrawImage(cg, CGRectMake(0, 0, g_ctx.width, g_ctx.height), img);
        CGImageRelease(img);
    }
    CGDataProviderRelease(prov);
    CGColorSpaceRelease(cs);
}

static void exp_window_will_close(id self, SEL _cmd, id notif) {
    (void)self;
    (void)_cmd;
    (void)notif;
    Class appCls = objc_getClass("NSApplication");
    id app = MSG_ID((id)appCls, sel_registerName("sharedApplication"));
    MSG_VOID_ID(app, sel_registerName("terminate:"), nil);
}

int exp_run_native_window(const char* title, const char* card_title, const char* card_text,
                          int w, int h, int headless_smoke) {
    memset(&g_ctx, 0, sizeof(g_ctx));
    snprintf(g_ctx.title, sizeof(g_ctx.title), "%s", title ? title : "Looping Asm+ObjC+C");
    snprintf(g_ctx.card_title, sizeof(g_ctx.card_title), "%s", card_title ? card_title : "Zero-Dep Native");
    snprintf(g_ctx.card_text, sizeof(g_ctx.card_text), "%s", card_text ? card_text : "Assembly + objc_msgSend + C11");
    g_ctx.width = w > 320 ? w : 760;
    g_ctx.height = h > 240 ? h : 480;
    g_ctx.fb_w = g_ctx.width;
    g_ctx.fb_h = g_ctx.height;
    g_ctx.fb = (uint32_t*)calloc((size_t)g_ctx.fb_w * (size_t)g_ctx.fb_h, sizeof(uint32_t));
    if (!g_ctx.fb) return 1;

    if (headless_smoke) {
        for (int i = 0; i < 3; ++i) render_asm_frame(&g_ctx);
        uint64_t digest = exp_asm_fnv1a64(g_ctx.fb, (size_t)g_ctx.fb_w * (size_t)g_ctx.fb_h * 4u);
        free(g_ctx.fb);
        g_ctx.fb = NULL;
        return (digest != 0 && g_ctx.frame == 3) ? 0 : 1;
    }

    Class poolCls = objc_getClass("NSAutoreleasePool");
    id pool = MSG_ID(MSG_ID((id)poolCls, sel_registerName("alloc")), sel_registerName("init"));

    Class appCls = objc_getClass("NSApplication");
    id app = MSG_ID((id)appCls, sel_registerName("sharedApplication"));
    MSG_VOID_INT(app, sel_registerName("setActivationPolicy:"), 0);

    Class viewCls = objc_allocateClassPair(objc_getClass("NSView"), "ExpAsmLoopView", 0);
    if (viewCls) {
        class_addMethod(viewCls, sel_registerName("drawRect:"), (IMP)exp_view_draw_rect, "v@:{CGRect={CGPoint=dd}{CGSize=dd}}");
        objc_registerClassPair(viewCls);
    } else {
        viewCls = objc_getClass("ExpAsmLoopView");
    }

    Class delCls = objc_allocateClassPair(objc_getClass("NSObject"), "ExpAsmWinDelegate", 0);
    if (delCls) {
        class_addMethod(delCls, sel_registerName("windowWillClose:"), (IMP)exp_window_will_close, "v@:@");
        objc_registerClassPair(delCls);
    } else {
        delCls = objc_getClass("ExpAsmWinDelegate");
    }

    Class winCls = objc_getClass("NSWindow");
    id win = MSG_ID_RECT_STYLE(
        MSG_ID((id)winCls, sel_registerName("alloc")),
        sel_registerName("initWithContentRect:styleMask:backing:defer:"),
        CGRectMake(0, 0, g_ctx.width, g_ctx.height),
        (1u | 2u | 4u | 8u), /* Titled | Closable | Miniaturizable | Resizable */
        2u,                   /* NSBackingStoreBuffered */
        NO
    );

    MSG_VOID_ID(win, sel_registerName("setTitle:"), nsstr(g_ctx.title));
    MSG_VOID(win, sel_registerName("center"));

    id delObj = MSG_ID(MSG_ID((id)delCls, sel_registerName("alloc")), sel_registerName("init"));
    MSG_VOID_ID(win, sel_registerName("setDelegate:"), delObj);

    id canvas = MSG_ID_RECT(MSG_ID((id)viewCls, sel_registerName("alloc")),
                            sel_registerName("initWithFrame:"),
                            CGRectMake(0, 0, g_ctx.width, g_ctx.height));
    MSG_VOID_ID(win, sel_registerName("setContentView:"), canvas);

    /* Add native NSTextField label via pure objc_msgSend */
    Class tfCls = objc_getClass("NSTextField");
    id lbl = ((id (*)(id, SEL, id))objc_msgSend)((id)tfCls, sel_registerName("labelWithString:"), nsstr(g_ctx.card_title));
    ((void (*)(id, SEL, CGRect))objc_msgSend)(lbl, sel_registerName("setFrame:"), CGRectMake(26, g_ctx.height - 62, 420, 26));
    Class colCls = objc_getClass("NSColor");
    id whiteCol = MSG_ID((id)colCls, sel_registerName("whiteColor"));
    MSG_VOID_ID(lbl, sel_registerName("setTextColor:"), whiteCol);
    MSG_VOID_ID(canvas, sel_registerName("addSubview:"), lbl);

    MSG_VOID_ID(win, sel_registerName("makeKeyAndOrderFront:"), nil);
    MSG_VOID_BOOL(app, sel_registerName("activateIgnoringOtherApps:"), YES);
    MSG_VOID(app, sel_registerName("run"));

    MSG_VOID(pool, sel_registerName("drain"));
    free(g_ctx.fb);
    g_ctx.fb = NULL;
    return 0;
}
