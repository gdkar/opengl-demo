#include <cmath>
#include <utility>
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdbool.h>
extern "C" {
#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <getopt.h>
}
#include <iostream>
#include <sstream>
#include <fstream>



#define countof(x) (sizeof(x) / sizeof(0[x]))
#define ATTRIB_POINT 0

static const char *
get_shader_str(GLenum type)
{
    switch(type) {
        case GL_FRAGMENT_SHADER: return "frag";
        case GL_GEOMETRY_SHADER: return "geom";
        case GL_VERTEX_SHADER:   return "vert";
        default:                 return "unknown";
    };
}
class GLObject {
    GLuint _id{0};
    protected:
        void set_id(GLuint nid)
        {
            _id = nid;
        }
    public:
        GLObject() = default;
        virtual ~GLObject() = default;
        GLObject(GLObject &&o)
        {
            std::swap(_id,o._id);
        }
        GLObject&operator =(GLObject &&o)
        {
            std::swap(_id, o._id);
            return *this;
        }
        GLObject(GLuint id)
            : _id(id) {  }
        GLObject &operator = (GLuint nid)
        {
            _id = nid;
            return *this;
        }
        inline operator GLuint () const
        {
            return _id;
        }
        inline GLuint id() const
        {
            return _id;
        }

        inline operator bool() const
        {
            return !!_id;
        }
};
class Shader : public GLObject {
    public:
        GLenum _typ{0};
        GLint  _status{0};
        Shader() = default;
        Shader(GLenum typ)
            : GLObject{glCreateShader(typ)}
            , _typ{typ}
        {}
        Shader(Shader &&o)
        :GLObject(std::forward<GLObject>(o))
        ,_typ(o._typ)
        ,_status(o._status) { }
        Shader &operator=(Shader &&o)
        {
            if(o.id() != id()) {
                set_id(o.id());
                o.set_id(0);
            }
            _typ = o._typ;
            _status = o._status;
            return *this;
        }
        Shader(GLenum typ, const std::string&src)
            :Shader(typ)
        {
            source(src);
        }
        static Shader from_file(GLenum typ, const std::string &file)
        {
            std::ifstream in{file};
            auto src = std::string(std::istreambuf_iterator<char>(in),
                                   std::istreambuf_iterator<char>());
            return Shader(typ, src);
        }
        void source(const std::string &src)
        {
            if(!src.empty()) {
                auto _str = static_cast<const GLchar*>(src.c_str());
                source(_str);
            }
        }
        void source(const GLchar *src)
        {
            glShaderSource(id(), 1, &src, nullptr);
        }
        void compile()
        {
            glCompileShader(id());
            glGetShaderiv(id(),GL_COMPILE_STATUS, &_status);
            if(!_status) {
                GLchar log[512];
                glGetShaderInfoLog(id(),sizeof(log),nullptr,log);
                throw(std::runtime_error(log));
            }
        }
        std::string log() const
        {
            GLchar log[512];
            glGetShaderInfoLog(id(),sizeof(log),nullptr,log);
            return std::string{log};

        }
        virtual ~Shader()
        {
            glDeleteShader(id());
            set_id(0);
        }
};
std::ostream &operator <<(std::ostream &o, Shader &s)
{
    o << get_shader_str(s._typ) << " shader " << s.id();
    return o;
}
class Program : public GLObject {
    public:
    GLint _status{0};
    Program() = default;
    Program(Program &&o)
        : GLObject(std::forward<GLObject>(o))
        , _status(o._status) { }
    Program&operator =(Program &&o)
    {
        glDeleteProgram(id());
        set_id(o.id());
        o.set_id(0);
        _status = o._status;
        return *this;
    }
    Program(std::initializer_list<GLuint> shaders)
    :GLObject(glCreateProgram())
    {
        for(const auto &s: shaders)
            attach(s);
    }
    void attach(GLuint s)
    {
        if(!id())
            set_id(glCreateProgram());
        glAttachShader(id(),s);
    }
    void attach(const Shader &s)
    {
        if(!id())
            set_id(glCreateProgram());
        glAttachShader(id(), s);
    }
    void detatch(const Shader &s)
    {
        glDetachShader(id(), s);
    }
    void link()
    {
        glLinkProgram(id());
        glGetProgramiv(id(),GL_LINK_STATUS, &_status);
        if(!_status) {
            GLchar log[512];
            glGetProgramInfoLog(id(),sizeof(log),nullptr,log);
            throw std::runtime_error(log);
        }
    }
    GLuint getFragDataLocation(const GLchar *name)
    {
        return glGetFragDataLocation(id(),name);
    }
    GLuint getFragDataIndex(const GLchar *name)
    {
        return glGetFragDataIndex(id(),name);
    }

    GLuint getUniformLocation(const GLchar *name)
    {
        return glGetUniformLocation(id(), name);
    }
    GLuint getAttribLocation(const GLchar *name)
    {
        return glGetAttribLocation(id(),name);
    }
    void bindFragDataLocation(GLuint color, const GLchar *name)
    {
        glBindFragDataLocation(id(),color,name);
    }
    void bindFragDataLocation(GLuint color, GLuint index, const GLchar *name)
    {
        glBindFragDataLocationIndexed(id(),color,index,name);
    }
    void setUniform(GLuint loc, GLfloat v0) { glProgramUniform1f(id(),loc,v0);}
    void setUniform(GLuint loc, GLfloat v0, GLfloat v1) { glProgramUniform2f(id(),loc,v0,v1);}
    void setUniform(GLuint loc, GLfloat v0, GLfloat v1, GLfloat v2) { glProgramUniform3f(id(),loc,v0,v1,v2);}
    void setUniform(GLuint loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { glProgramUniform4f(id(),loc,v0,v1,v2,v3);}
    void setUniform(GLuint loc, GLint v0) { glProgramUniform1i(id(),loc,v0);}
    void setUniform(GLuint loc, GLint v0, GLint v1) { glProgramUniform2i(id(),loc,v0,v1);}
    void setUniform(GLuint loc, GLint v0, GLint v1, GLint v2) { glProgramUniform3i(id(),loc,v0,v1,v2);}
    void setUniform(GLuint loc, GLint v0, GLint v1, GLint v2, GLint v3) { glProgramUniform4i(id(),loc,v0,v1,v2,v3);}
    void setUniform(GLuint loc, GLuint v0) { glProgramUniform1ui(id(),loc,v0);}
    void setUniform(GLuint loc, GLuint v0, GLuint v1) { glProgramUniform2ui(id(),loc,v0,v1);}
    void setUniform(GLuint loc, GLuint v0, GLuint v1, GLuint v2) { glProgramUniform3ui(id(),loc,v0,v1,v2);}
    void setUniform(GLuint loc, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { glProgramUniform4ui(id(),loc,v0,v1,v2,v3);}
    void use()
    {
        glUseProgram(id());
    }
    void unuse()
    {
        glUseProgram(0);
    }
    virtual ~Program()
    {
        glDeleteProgram(id());
        set_id(0);
    }
};
typedef struct GraphicsContext {
    GLFWwindow      *window;
    GLuint           program;
    GLuint           vbo;
    GLuint           vao;
}GraphicsContext;

struct graphics_context {
    GLFWwindow *window;
    Program program;
    GLint   uniform_angle;
    GLint   uniform_size;
    GLint   uniform_color;
    GLuint  vbo_point;
    GLuint  vao_point;
    size_t  vert_count;
    double  angle;
    long    framecount;
    double  lastframe;
};

const struct { float x; float y;} SQUARE[] = {
    { 0.25f,  .25f, },
    { -0.25f, -.25f, },
//    {  1.0f,  1.0f, },
//    {  1.0f, -1.0f, },
};

static void
render(struct graphics_context *context)
{
    glClearColor(0.15, 0.15, 0.15, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    context->program.use();
    context->program.setUniform(context->uniform_angle,(GLfloat)context->angle);
    context->program.setUniform(context->uniform_size, (GLfloat)0.5f);
    context->program.setUniform(context->uniform_color, (GLfloat)1.f,(GLfloat)0.f,(GLfloat)0.f,(GLfloat)1.f);
    glBindVertexArray(context->vao_point);
    glDrawArrays(GL_POINTS, 0, context->vert_count);
    glBindVertexArray(0);
    context->program.unuse();

    /* Physics */
    double now = glfwGetTime();
    double udiff = now - context->lastframe;
    context->angle -= udiff;

    if (context->angle > 2 * M_PI)
        context->angle -= 2 * M_PI;

    context->framecount++;
    if ((long)now != (long)context->lastframe) {
        printf("FPS: %ld\n", context->framecount);
        context->framecount = 0;
    }
    context->lastframe = now;
    glfwSwapBuffers(context->window);
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int
main(int argc, char **argv)
{
    /* Options */
    auto fullscreen = false;
    const char *title = "OpenGL 3.3 Demo";

    int opt;
    while ((opt = getopt(argc, argv, "f")) != -1) {
        switch (opt) {
            case 'f':
                fullscreen = true;
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    /* Create window and OpenGL context */
    struct graphics_context context;
    if (!glfwInit()) {
        fprintf(stderr, "GLFW3: failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *m = glfwGetVideoMode(monitor);
        context.window = glfwCreateWindow(m->width, m->height, title, monitor, NULL);
    } else {
        context.window = glfwCreateWindow(640, 640, title, NULL, NULL);
    }
    /* Initialize gl3w */
    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(1);
    if (gl3wInit()) {
        fprintf(stderr, "gl3w: failed to initialize\n");
        exit(EXIT_FAILURE);
    }



    /* Shader sources */
    const GLchar *vert_shader =
        "#version 330\n"
        "layout(location = 0) in vec2 point;\n"
        "out vec4 v_pos;\n"
        "out vec4 v_color;\n"
        "uniform float u_angle;\n"
        "uniform float u_size;\n"
        "uniform vec4  u_color;\n"
        "void main()"
        "{\n"
        "   v_pos = vec4(point.x,point.y,u_angle,u_size);\n"
        "   v_color = u_color;\n"
        "}\n";
    const GLchar *geom_shader =
        "#version 330\n"
        "layout(points) in;\n"
        "layout(triangle_strip, max_vertices = 64) out;\n"
        "in vec4 v_pos[];\n"
        "in vec4 v_color[];\n"
        "out vec4 g_color;\n"
        "void main()\n"
        "{\n"
        "   vec2 pos = v_pos[0].xy;\n"
        "   float angle = v_pos[0].z;\n"
        "   float size  = v_pos[0].w;\n"
        "   mat2 rot = mat2(cos(angle), -sin(angle),\n"
        "                   sin(angle),  cos(angle));\n"
        "   gl_Position = vec4(pos + vec2(size,size) * rot, 0, 1);\n"
        "   g_color = v_color[0];\n"
        "   EmitVertex();\n"
        "   gl_Position = vec4(pos + vec2(size,-size) * rot,0,1);\n"
        "   g_color = v_color[0];\n"
        "   EmitVertex();\n"
        "   gl_Position = vec4(pos + vec2(-size,size) * rot,0,1);\n"
        "   g_color = v_color[0];\n"
        "   EmitVertex();\n"
        "   gl_Position = vec4(pos + vec2(-size,-size) * rot,0,1);\n"
        "   g_color = v_color[0];\n"
        "   EmitVertex();\n"
        "   EndPrimitive();\n"
        "}";
    const GLchar *frag_shader =
        "#version 330\n"
        "in  vec4 g_color;\n"
        "out vec4 f_color;\n"
        "void main() {\n"
        "    f_color = g_color;\n"
        "}\n";

    /* Compile and link OpenGL program */
    {
        auto vert = Shader(GL_VERTEX_SHADER, vert_shader);
        auto geom = Shader(GL_GEOMETRY_SHADER, geom_shader);
        auto frag = Shader(GL_FRAGMENT_SHADER, frag_shader);
        vert.compile();
        geom.compile();
        frag.compile();
        context.program = Program({vert.id(),geom.id(),frag.id()});
    }
    context.program.bindFragDataLocation(0,"f_color");
    context.program.link();
    context.uniform_angle = context.program.getUniformLocation("u_angle");
    context.uniform_size  = context.program.getUniformLocation("u_size");
    context.uniform_color = context.program.getUniformLocation("u_color");
    /* Prepare vertex buffer object (VBO) */
    glGenBuffers(1, &context.vbo_point);
    glBindBuffer(GL_ARRAY_BUFFER, context.vbo_point);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SQUARE), SQUARE, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    context.vert_count = countof(SQUARE);

    /* Prepare vertrex array object (VAO) */
    glGenVertexArrays(1, &context.vao_point);
    glBindVertexArray(context.vao_point);
    glBindBuffer(GL_ARRAY_BUFFER, context.vbo_point);
    glVertexAttribPointer(ATTRIB_POINT, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(ATTRIB_POINT);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Start main loop */
    glfwSetKeyCallback(context.window, key_callback);
    context.lastframe = glfwGetTime();
    context.framecount = 0;

    while (!glfwWindowShouldClose(context.window)) {
        render(&context);
        glfwPollEvents();
    }
    fprintf(stderr, "Exiting ...\n");

    /* Cleanup and exit */
    glDeleteVertexArrays(1, &context.vao_point);
    glDeleteBuffers(1, &context.vbo_point);

    glfwTerminate();
    return 0;
}
