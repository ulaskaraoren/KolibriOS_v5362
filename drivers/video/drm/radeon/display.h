
typedef struct tag_display display_t;

typedef struct
{
    kobj_t     header;

    uint32_t  *data;
    uint32_t   hot_x;
    uint32_t   hot_y;

    struct list_head   list;
    struct radeon_bo  *robj;
}cursor_t;

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

struct tag_display
{
    int  x;
    int  y;
    int  width;
    int  height;
    int  bpp;
    int  vrefresh;
    int  pitch;
    int  lfb;

    int  supported_modes;
    struct drm_device    *ddev;
    struct drm_connector *connector;
    struct drm_crtc      *crtc;

    struct list_head   cursors;

    cursor_t   *cursor;
    int       (*init_cursor)(cursor_t*);
    cursor_t* (__stdcall *select_cursor)(cursor_t*);
    void      (*show_cursor)(int show);
    void      (__stdcall *move_cursor)(cursor_t *cursor, int x, int y);
    void      (__stdcall *restore_cursor)(int x, int y);
    void      (*disable_mouse)(void);
    u32  mask_seqno;
    u32  check_mouse;
    u32  check_m_pixel;

};

extern display_t *os_display;

int   init_cursor(cursor_t *cursor);
void  __stdcall restore_cursor(int x, int y);
