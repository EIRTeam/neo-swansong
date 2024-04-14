#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include "core/object/class_db.h"
#include "../console_system.h"

class HBUserConfig : public Object {
    GDCLASS(HBUserConfig, Object);
public:
    enum WindowMode {
        FULLSCREEN,
        BORDERLESS_WINDOWED,
        WINDOWED,
    };

    enum RenderScaleMode {
        BILINEAR,
        FSR,
        FSR2
    };

    WindowMode window_mode = WindowMode::BORDERLESS_WINDOWED;
    RenderScaleMode render_scale_mode = RenderScaleMode::BILINEAR;

    int screen = -1;
    float render_scale = 1.0;

    void _update_window_settings();
    void _update_screen_settings();
    void _update_render_scale();
    void _update_render_scale_mode();
protected:
    static CVar window_mode_cvar;
    static CVar render_scale_cvar;
    static CVar render_scale_mode_cvar;
    static void _bind_methods();
public:
    void set_window_mode(WindowMode p_window_mode);
    WindowMode get_window_mode() const;

    void set_render_scale(float p_render_scale);
    float get_render_scale() const;

    RenderScaleMode get_render_scale_mode() const { return render_scale_mode; }
    void set_render_scale_mode(const RenderScaleMode p_render_scale_mode);

    void apply();
    TypedArray<Vector2i> get_available_screens() const;
    String get_window_mode_text(WindowMode p_window_mode) const;

    HBUserConfig();
};

VARIANT_ENUM_CAST(HBUserConfig::WindowMode);
VARIANT_ENUM_CAST(HBUserConfig::RenderScaleMode);

#endif // USER_CONFIG_H
