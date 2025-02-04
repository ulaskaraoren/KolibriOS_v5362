#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "render.h"
#include <drm/i915_drm.h>
#include <kos32sys.h>

void create_mask(struct render *render);

static int drm_ioctl(int fd, unsigned long request, void *arg)
{
    ioctl_t  io;

    io.handle   = fd;
    io.io_code  = request;
    io.input    = arg;
    io.inp_size = 64;
    io.output   = NULL;
    io.out_size = 0;

    return call_service(&io);
}

static EGLImageKHR px_create_image(EGLDisplay display, EGLContext context,
			 int width, int height, int stride, int name)
{
	EGLImageKHR image;
	EGLint attribs[] = {
		EGL_WIDTH, 0,
		EGL_HEIGHT, 0,
		EGL_DRM_BUFFER_STRIDE_MESA, 0,
		EGL_DRM_BUFFER_FORMAT_MESA,
		EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
		EGL_DRM_BUFFER_USE_MESA,
		EGL_DRM_BUFFER_USE_SHARE_MESA |
		    EGL_DRM_BUFFER_USE_SCANOUT_MESA,
		EGL_NONE
	};
	attribs[1] = width;
	attribs[3] = height;
	attribs[5] = stride/4;

    printf("%s w:%d :%d pitch:%d handle %d\n", __FUNCTION__,
           width, height, stride, name);

	image = eglCreateImageKHR(display, context, EGL_DRM_BUFFER_MESA,
						 (void *) (uintptr_t)name, attribs);

	return image;
}

static GLint create_shader(GLenum type, const char *source)
{
	GLint ok;
    GLint shader;

    shader = glCreateShader(type);
    if(shader == 0)
        goto err;

    glShaderSource(shader, 1, (const GLchar **) &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

        glGetShaderInfoLog(shader, size, NULL, info);
		printf("Failed to compile %s: %s\n",
                type == GL_FRAGMENT_SHADER ? "FS" : "VS",info);
		printf("Program source:\n%s", source);
		printf("GLSL compile failure\n");
        free(info);
        glDeleteShader(shader);
        shader = 0;
	}
err:
    return shader;
}


struct render* create_render(EGLDisplay dpy, EGLSurface surface, int dx, int dy)
{
    const char *vs_src =
	    "attribute vec4 v_position;\n"
	    "attribute vec4 v_texcoord0;\n"
	    "varying vec2 source_texture;\n"
        "void main()\n"
	    "{\n"
	    "	gl_Position = v_position;\n"
	    "	source_texture = v_texcoord0.xy;\n"
	    "}\n";

    const char *fs_i965 =
	    "varying vec2 source_texture;\n"
        "uniform sampler2D sampler_src;\n"
        "uniform sampler2D sampler_mask;\n"
	    "void main()\n"
	    "{\n"
        "   float ca = texture2D(sampler_mask, source_texture).r;\n"
        "   gl_FragColor = vec4(texture2D(sampler_src, source_texture).rgb, ca);\n"
	    "}\n";

    const char *fs_i915 =
        "varying vec2 source_texture;\n"
        "uniform sampler2D sampler_src;\n"
        "uniform sampler2D sampler_mask;\n"
        "void main()\n"
        "{\n"
        "   float ca = texture2D(sampler_mask, source_texture).a;\n"
        "   gl_FragColor = vec4(texture2D(sampler_src, source_texture).rgb, ca);\n"
        "}\n";

    EGLint config_attribs[14];
    EGLConfig config;
    EGLint num_configs;

    EGLContext context;

    struct drm_i915_fb_info fb;
    GLint  vs_shader, fs_shader;
    GLenum status;
    GLint ret;
    int fd;
    struct render *render;
    char *drv_name;
    const char *fs_src;


    fd = get_service("DISPLAY");

    memset(&fb, 0, sizeof(fb));
    ret = drm_ioctl(fd, SRV_FBINFO, &fb);
    if( ret != 0 )
    {   printf("failed to get framebuffer info\n");
        goto err;
    };

    render = (struct render*)malloc(sizeof(struct render));
    if(render == NULL)
        goto err;

    render->fd  = fd;
    render->dpy = dpy;
    render->dx  = dx;
    render->dy  = dy;

    if(!eglQuerySurface(dpy, surface, EGL_WIDTH, &render->width))
        goto err1;

    if(!eglQuerySurface(dpy, surface, EGL_HEIGHT, &render->height))
        goto err1;

    render->scr_width  = fb.width;
    render->scr_height = fb.height;

    render->front = eglGetBufferImage(dpy, surface, EGL_DRM_BUFFER_FRONT);
    if(render->front == EGL_NO_IMAGE_KHR)
        goto err1;

    render->back  = eglGetBufferImage(dpy, surface, EGL_DRM_BUFFER_BACK);
    if( render->back == EGL_NO_IMAGE_KHR)
        goto err2;


    context = eglGetCurrentContext();

    config_attribs[0] = EGL_RED_SIZE;
    config_attribs[1] = 1;
    config_attribs[2] = EGL_GREEN_SIZE;
    config_attribs[3] = 1;
    config_attribs[4] = EGL_BLUE_SIZE;
    config_attribs[5] = 1;
    config_attribs[6] = EGL_DEPTH_SIZE;
    config_attribs[7] = 1;

    config_attribs[8] = EGL_SURFACE_TYPE;
    config_attribs[9] = EGL_WINDOW_BIT;

    config_attribs[10] = EGL_RENDERABLE_TYPE;
    config_attribs[11] = EGL_OPENGL_BIT;
    config_attribs[12] = EGL_NONE;

    if (!eglChooseConfig(dpy,config_attribs, &config, 1, &num_configs) || !num_configs)
    {
        printf("failed to choose a config");
        goto err3;
    }

    render->context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
    if (!context)
    {
        printf("failed to create context");
        goto err3;
    };

    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, render->context))
    {
        printf("failed to make window current");
        goto err4;
    };


    glGenTextures(2, render->tx_buffers);
    if(glGetError() != GL_NO_ERROR)
       goto err5;

    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[EGL_DRM_BUFFER_FRONT]);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->front);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[EGL_DRM_BUFFER_BACK]);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->back);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glBindTexture(GL_TEXTURE_2D, 0);

    render->back_buffer = EGL_DRM_BUFFER_FRONT;

    render->screen = px_create_image(dpy,context,fb.width,fb.height,
                                     fb.pitch,fb.name);
    if(render->screen == EGL_NO_IMAGE_KHR)
        goto err6;

    glGenTextures(1, &render->tx_screen);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glBindTexture(GL_TEXTURE_2D, render->tx_screen);
    if(glGetError() != GL_NO_ERROR)
       goto err7;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->screen);
    if(glGetError() != GL_NO_ERROR)
       goto err7;

    glBindTexture(GL_TEXTURE_2D, 0);

    create_mask(render);

    glGenFramebuffers(1, &render->framebuffer);
    if(glGetError() != GL_NO_ERROR)
       goto err8;

    glBindFramebuffer(GL_FRAMEBUFFER, render->framebuffer);
    if(glGetError() != GL_NO_ERROR)
       goto err9;

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, render->tx_screen,0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        const char *str;
        switch (status)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                 str = "incomplete attachment";
                 break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                str = "incomplete/missing attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                str = "incomplete draw buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                str = "incomplete read buffer";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                str = "unsupported";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                str = "incomplete multiple";
                break;
            default:
                str = "unknown error";
                break;
        }
        printf("destination is framebuffer incomplete: %s [%#x]\n", str, status);
        goto err9;
    }

    glViewport(0, 0, render->scr_width, render->scr_height);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    drv_name = (char*)glGetString(0x1F04);
    printf("Render: %s\n", drv_name);

    if(strstr(drv_name, "i965"))
        fs_src = fs_i965;
    else if(strstr(drv_name, "i915"))
        fs_src = fs_i915;
    else fs_src = NULL;

    render->blit_prog = glCreateProgram();
    if(render->blit_prog == 0)
        goto err9;

    vs_shader = create_shader(GL_VERTEX_SHADER,vs_src);
    if(vs_shader == 0)
        goto err10;

    fs_shader = create_shader(GL_FRAGMENT_SHADER, fs_src);
    if(fs_shader == 0)
        goto err11;

    glAttachShader(render->blit_prog, vs_shader);
    glAttachShader(render->blit_prog, fs_shader);
    glBindAttribLocation(render->blit_prog, 0, "v_position");
    glBindAttribLocation(render->blit_prog, 1, "v_texcoord0");

    glLinkProgram(render->blit_prog);
    glGetProgramiv(render->blit_prog, GL_LINK_STATUS, &ret);
    if (!ret)
    {
        GLchar *info;
        GLint size;

        glGetProgramiv(render->blit_prog, GL_INFO_LOG_LENGTH, &size);
        info = malloc(size);

        glGetProgramInfoLog(render->blit_prog, size, NULL, info);
        printf("Failed to link: %s\n", info);
        printf("GLSL link failure\n");
        free(info);
        goto err12;
    }

    render->sampler = glGetUniformLocation(render->blit_prog,"sampler_src");
    render->sm_mask = glGetUniformLocation(render->blit_prog,"sampler_mask");

    glUseProgram(render->blit_prog);
    glUniform1i(render->sampler, 0);
    glUniform1i(render->sm_mask, 1);

    glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),render->vertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),render->tc_src);
    glEnableVertexAttribArray(1);

    render->tc_src[0]    = 0.0;
    render->tc_src[1]    = 0.0;
    render->tc_src[1*2]  = 1.0;
    render->tc_src[1*2+1]= 0.0;
    render->tc_src[2*2]  = 1.0;
    render->tc_src[2*2+1]= 1.0;
    render->tc_src[3*2]  = 0.0;
    render->tc_src[3*2+1]= 1.0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, render->tx_mask);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);

    eglMakeCurrent(dpy, surface, surface, context);

    return render;

err12:
    glDeleteShader(fs_shader);
err11:
    glDeleteShader(vs_shader);
err10:
    glDeleteProgram(render->blit_prog);
err9:
    glDeleteFramebuffers(1, &render->framebuffer);
err8:
    eglDestroyImageKHR(dpy, render->screen);
err7:
    glDeleteTextures(1, &render->tx_screen);
err6:
    glDeleteTextures(2, render->tx_buffers);
err5:
    eglMakeCurrent(dpy, surface, surface, context);
err4:
    eglDestroyContext(dpy, render->context);
err3:
    eglDestroyImageKHR(dpy, render->back);
err2:
    eglDestroyImageKHR(dpy, render->front);
err1:
    free(render);
err:
    return NULL;
};


void create_mask(struct render *render)
{
    struct drm_i915_gem_create create;
    struct drm_gem_close close;
    struct drm_gem_flink flink;

    struct drm_i915_gem_mmap mmap_arg;
    EGLint handle, stride;
    int winw, winh, pitch;
    void *data;
    int ret;
    GLuint mask_handle, mask_name;
    EGLImageKHR mask_image;

    glGenTextures(1, &render->tx_mask);
    if(glGetError() != GL_NO_ERROR)
       return;

    glBindTexture(GL_TEXTURE_2D, render->tx_mask);
    if(glGetError() != GL_NO_ERROR)
       goto err1;

    pitch = (render->width+15) & ~15;

    create.size = pitch *render->height;
    create.handle = 0;
    ret = drm_ioctl(render->fd, DRM_IOCTL_I915_GEM_CREATE, &create);
    if(ret != 0)
        goto err1;

    mask_handle = create.handle;
    flink.handle = create.handle;
    ret = drm_ioctl(render->fd, DRM_IOCTL_GEM_FLINK, &flink);
    if (ret != 0)
        goto err2;

    mask_name = flink.name;

    printf("create mask bo handle %d name %d\n", create.handle, flink.name);

    EGLint attribs[] = {
        EGL_WIDTH, 0,
        EGL_HEIGHT, 0,
        EGL_DRM_BUFFER_STRIDE_MESA, 0,
        EGL_DRM_BUFFER_FORMAT_MESA,
        EGL_DRM_BUFFER_FORMAT_R8_MESA,
        EGL_DRM_BUFFER_USE_MESA,
        EGL_DRM_BUFFER_USE_SHARE_MESA,
        EGL_NONE
    };
    attribs[1] = pitch;
    attribs[3] = render->height;
    attribs[5] = pitch;

    mask_image = eglCreateImageKHR(render->dpy, render->context, EGL_DRM_BUFFER_MESA,
                         (void *) (uintptr_t)mask_name, attribs);
    printf("create mask image %p\n", mask_image);
    if(mask_image == NULL)
        goto err2;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, mask_image);

    if(glGetError() != GL_NO_ERROR)
       goto err3;

    glBindTexture(GL_TEXTURE_2D, 0);

    mmap_arg.handle = mask_handle;
    mmap_arg.offset = 0;
    mmap_arg.size = pitch * render->height;
    if (drm_ioctl(render->fd, DRM_IOCTL_I915_GEM_MMAP, &mmap_arg))
    {
        printf("%s: failed to mmap image %p handle=%d, %d bytes, into CPU domain\n",
               __FUNCTION__, mask_image, mask_handle, pitch * render->height);
        goto err3;
    }

    render->mask_handle = mask_handle;
    render->mask_name   = mask_name;
    render->mask_buffer = (void *)(uintptr_t)mmap_arg.addr_ptr;
    render->mask_image = mask_image;

    return;

err3:
    eglDestroyImageKHR(render->dpy, mask_image);
err2:
    close.handle = mask_handle;
    (void)drm_ioctl(render->fd, DRM_IOCTL_GEM_CLOSE, &close);
    glBindTexture(GL_TEXTURE_2D, 0);
err1:
    glDeleteTextures(1, &render->tx_mask);
};
